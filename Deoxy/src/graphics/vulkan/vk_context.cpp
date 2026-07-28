#include "vk_context.hpp"
#include "components/vk_helper.hpp"

#include <deoxy/platform/window.hpp>
#include <deoxy/platform/logger.hpp>
#include <deoxy/graphics/vertex.hpp>

#include <SDL3/SDL_vulkan.h>

#include <array>

namespace deoxy::graphics {
    VulkanContext::VulkanContext(platform::Window& window)
        : m_instance(),
          m_surface(m_instance.GetHandle(), window.GetHandle()),
          m_device(m_instance.GetHandle(), m_surface.GetHandle()),
          m_allocator(m_instance.GetHandle(), m_device.GetPhysical(), m_device.GetLogical()),
          m_commandPool(m_device.GetLogical(), m_device.GetGraphicsQueueFamily()),
          m_swapchain(window.GetHandle(), m_surface, m_device, m_allocator),
          m_frames{
              vulkan::VulkanFrame{ m_device.GetLogical(), m_commandPool },
              vulkan::VulkanFrame{ m_device.GetLogical(), m_commandPool }
          },
          m_pipeline(
              m_device.GetLogical(), m_swapchain.GetColorFormat(),
              "shaders/basic.vert.spv", "shaders/basic.frag.spv") {
        createGeometryBuffers();
    }

    VulkanContext::~VulkanContext() {
        const VkDevice device = m_device.GetLogical();

        if (device != VK_NULL_HANDLE) vkDeviceWaitIdle(device);

        const VmaAllocator allocator = m_allocator.GetHandle();
        if (m_indexBuffer != VK_NULL_HANDLE) vmaDestroyBuffer(allocator, m_indexBuffer, m_indexAllocation);
        if (m_vertexBuffer != VK_NULL_HANDLE) vmaDestroyBuffer(allocator, m_vertexBuffer, m_vertexAllocation);
    }

    void VulkanContext::DrawFrame() {
        if (!m_swapchain.CanRender()) return;
        if (m_swapchain.NeedsRecreation()) { m_swapchain.Recreate(); return; }

        vulkan::VulkanFrame& frame = m_frames[m_currentFrame];
        frame.Wait();

        uint32_t imageIndex = 0;

        // Pede uma imagem disponível da swapchain
        VkResult acquireResult = vkAcquireNextImageKHR(
            m_device.GetLogical(), m_swapchain.GetHandle(),
            UINT64_MAX,
            frame.GetImageAvailableSemaphore(),
            VK_NULL_HANDLE,
            &imageIndex
        );

        if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR) {
            m_swapchain.Recreate();
            return;
        }

        if (acquireResult != VK_SUCCESS && acquireResult != VK_SUBOPTIMAL_KHR) vulkan::CheckResult(acquireResult);

        const bool swapchainSuboptimal = acquireResult == VK_SUBOPTIMAL_KHR;

        // Não resta a cerca antes do acquire
        frame.ResetForSubmit();

        // Gravando o command buffer
        recordCommandBuffer(frame.GetCommandBuffer(), imageIndex);

        // Enviando para a GPU
        const VkSemaphore renderFinishedSemaphore = m_swapchain.GetRenderFinishedSemaphore(imageIndex);
        VkSemaphoreSubmitInfo waitSemaphoreInfo {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .semaphore = frame.GetImageAvailableSemaphore(),
            .value = 0,
            .stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            .deviceIndex = 0
        };

        VkCommandBufferSubmitInfo commandBufferInfo {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
            .commandBuffer = frame.GetCommandBuffer(),
            .deviceMask = 1
        };

        VkSemaphoreSubmitInfo signalSemaphoreInfo {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .semaphore = renderFinishedSemaphore,
            .value = 0,
            .stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
            .deviceIndex = 0
        };

        VkSubmitInfo2 submitInfo {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
            .waitSemaphoreInfoCount = 1,
            .pWaitSemaphoreInfos = &waitSemaphoreInfo,
            .commandBufferInfoCount = 1,
            .pCommandBufferInfos = &commandBufferInfo,
            .signalSemaphoreInfoCount = 1,
            .pSignalSemaphoreInfos = &signalSemaphoreInfo
        };

        vulkan::CheckResult(vkQueueSubmit2(m_device.GetQueue(), 1, &submitInfo, frame.GetFence()));

        // Presentar a imagem
        const VkSwapchainKHR swapchainHandle = m_swapchain.GetHandle();
        VkPresentInfoKHR presentInfo {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &renderFinishedSemaphore,
            .swapchainCount = 1,
            .pSwapchains = &swapchainHandle,
            .pImageIndices = &imageIndex
        };

        VkResult presentResult = vkQueuePresentKHR(m_device.GetQueue(), &presentInfo);

        const bool mustRecreate = presentResult == VK_ERROR_OUT_OF_DATE_KHR ||
                                  presentResult == VK_SUBOPTIMAL_KHR ||
                                  swapchainSuboptimal;

        if (presentResult != VK_SUCCESS &&
            presentResult != VK_SUBOPTIMAL_KHR &&
            presentResult != VK_ERROR_OUT_OF_DATE_KHR
        ) vulkan::CheckResult(presentResult);

        m_currentFrame = (m_currentFrame + 1) % FRAMES_IN_FLIGHT;

        if (mustRecreate) m_swapchain.Recreate();
    }

    void VulkanContext::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
        // Iniciando o command buffer
        VkCommandBufferBeginInfo beginInfo {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
        };

        vulkan::CheckResult(vkBeginCommandBuffer(commandBuffer, &beginInfo));

        // Prepara a imagem para receber renderização
        const VkImage swapchainImage = m_swapchain.GetImage(imageIndex);
        const VkImageView swapchainImageView = m_swapchain.GetImageView(imageIndex);
        const VkExtent2D extent = m_swapchain.GetExtent();
        transitionImage(
            commandBuffer, swapchainImage,
            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_PIPELINE_STAGE_2_NONE, 0,
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT
        );

        // Configura o attachment de cor
        VkRenderingAttachmentInfo colorAttachment {
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = swapchainImageView,
            .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue {
                .color {
                    .float32 {
                        0.05f, 0.1f, 0.2f, 1.0f
                    }
                }
            }
        };

        VkRenderingInfo renderingInfo {
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .renderArea {
                .offset = {0, 0},
                .extent = extent
            },
            .layerCount = 1,
            .colorAttachmentCount = 1,
            .pColorAttachments = &colorAttachment
        };

        // Limpa a imagem
        vkCmdBeginRendering(commandBuffer, &renderingInfo);
            // Bindando a pipeline
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline.GetHandle());
                // Criando o viewport
                VkViewport viewport {
                    .x = 0.0f, .y = 0.0f,
                    .width = static_cast<float>(extent.width), .height = static_cast<float>(extent.height),
                    .minDepth = 0.0f, .maxDepth = 1.0f
                };

                // Criando o scissor
                VkRect2D scissor {
                    .offset = {0, 0},
                    .extent = extent
                };

                vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
                vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

                VkDeviceSize vertexOffset = 0;
                vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_vertexBuffer, &vertexOffset);
                vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);

                vkCmdDrawIndexed(commandBuffer, m_indexCount, 1, 0, 0, 0);
        vkCmdEndRendering(commandBuffer);

        // Prepara a imagem para aparecer na tela
        transitionImage(
            commandBuffer, swapchainImage,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
            VK_PIPELINE_STAGE_2_NONE, 0
        );

        vulkan::CheckResult(vkEndCommandBuffer(commandBuffer));
    }

    void VulkanContext::transitionImage(
        VkCommandBuffer commandBuffer,
        VkImage image,
        VkImageLayout oldLayout,
        VkImageLayout newLayout,
        VkPipelineStageFlags2 srcStage,
        VkAccessFlags2 srcAccess,
        VkPipelineStageFlags2 dstStage,
        VkAccessFlags2 dstAccess
    ) {
        VkImageMemoryBarrier2 barrier{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = srcStage,
            .srcAccessMask = srcAccess,
            .dstStageMask = dstStage,
            .dstAccessMask = dstAccess,
            .oldLayout = oldLayout,
            .newLayout = newLayout,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = image,
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };

        VkDependencyInfo dependencyInfo{
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &barrier
        };

        vkCmdPipelineBarrier2(commandBuffer, &dependencyInfo);
    }

    void VulkanContext::createGeometryBuffers() {
        // Cria as informações
        const std::array<Vertex, 3> vertices {
            Vertex {
                .Position = { 0.0f, -0.5f, 0.0f },
                .Color = { 1.0f, 0.0f, 0.0f }
            },
            Vertex {
                .Position = { 0.5f, 0.5f, 0.0f },
                .Color = { 0.0f, 1.0f, 0.0f }
            },
            Vertex {
                .Position = { -0.5f, 0.5f, 0.0f },
                .Color = { 0.0f, 0.0f, 1.0f }
            }
        };

        const std::array<uint32_t, 3> indices {
            0, 1, 2
        };

        // Cria o staging buffer para o vertex
        const VkDeviceSize vertexBufferSize = sizeof(vertices);
        VkBuffer stagingVertexBuffer = VK_NULL_HANDLE;
        VmaAllocation stagingVertexAllocation = nullptr;
        createBuffer(
            vertexBufferSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
            stagingVertexBuffer,
            stagingVertexAllocation
        );

        // Copia os dados da CPU
        vulkan::CheckResult(vmaCopyMemoryToAllocation(
            m_allocator.GetHandle(),
            vertices.data(),
            stagingVertexAllocation,
            0,
            vertexBufferSize
        ));

        // Cria o VBO
        createBuffer(
            vertexBufferSize,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE, 0,
            m_vertexBuffer,
            m_vertexAllocation
        );

        // Copia staging para a GPU
        copyBuffer(stagingVertexBuffer, m_vertexBuffer, vertexBufferSize);

        // Destrói o staging
        vmaDestroyBuffer(m_allocator.GetHandle(), stagingVertexBuffer, stagingVertexAllocation);

        // Cria o staging buffer para o index
        const VkDeviceSize indexBufferSize = sizeof(indices);
        VkBuffer stagingIndexBuffer = VK_NULL_HANDLE;
        VmaAllocation stagingIndexAllocation = nullptr;
        createBuffer(
            indexBufferSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
            stagingIndexBuffer,
            stagingIndexAllocation
        );

        // Copia os dados da CPU
        vulkan::CheckResult(vmaCopyMemoryToAllocation(
            m_allocator.GetHandle(),
            indices.data(),
            stagingIndexAllocation,
            0,
            indexBufferSize
        ));

        // Cria o EBO
        createBuffer(
            indexBufferSize,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE, 0,
            m_indexBuffer,
            m_indexAllocation
        );

        // Copia staging para a GPU
        copyBuffer(stagingIndexBuffer, m_indexBuffer, indexBufferSize);

        // Destrói o staging
        vmaDestroyBuffer(m_allocator.GetHandle(), stagingIndexBuffer, stagingIndexAllocation);

        m_indexCount = static_cast<uint32_t>(indices.size());
    }

    // Helper para criar buffers
    void VulkanContext::createBuffer(
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VmaMemoryUsage memoryUsage,
        VmaAllocationCreateFlags allocationFlags,
        VkBuffer& buffer,
        VmaAllocation& allocation
    ) {
        VkBufferCreateInfo bufferCI {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = size,
            .usage = usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE
        };

        VmaAllocationCreateInfo allocationCI {
            .flags = allocationFlags,
            .usage = memoryUsage
        };

        vulkan::CheckResult(vmaCreateBuffer(m_allocator.GetHandle(), &bufferCI, &allocationCI, &buffer, &allocation, nullptr));
    }

    // Helper para copiar buffers
    void VulkanContext::copyBuffer(VkBuffer src, VkBuffer dest, VkDeviceSize size) {
        VkCommandBuffer commandBuffer = m_commandPool.AllocatePrimary();

        try {
            VkCommandBufferBeginInfo beginInfo {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
            };

            vulkan::CheckResult(vkBeginCommandBuffer(commandBuffer, &beginInfo));
                VkBufferCopy copyRegion {
                    .srcOffset = 0,
                    .dstOffset = 0,
                    .size = size
                };

                vkCmdCopyBuffer(commandBuffer, src, dest, 1, &copyRegion);
            vulkan::CheckResult(vkEndCommandBuffer(commandBuffer));

            VkCommandBufferSubmitInfo commandBufferInfo {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
                .commandBuffer = commandBuffer,
                .deviceMask = 1
            };

            VkSubmitInfo2 submitInfo {
                .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
                .commandBufferInfoCount = 1,
                .pCommandBufferInfos = &commandBufferInfo
            };

            vulkan::CheckResult(vkQueueSubmit2(m_device.GetQueue(), 1, &submitInfo, VK_NULL_HANDLE));
            vulkan::CheckResult(vkQueueWaitIdle(m_device.GetQueue()));
        } catch (...) {
            m_commandPool.Free(commandBuffer);
            throw;
        }

        m_commandPool.Free(commandBuffer);
    }
}
