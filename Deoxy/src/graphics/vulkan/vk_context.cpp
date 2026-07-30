#include "vk_context.hpp"
#include "components/vk_helper.hpp"
#include "components/vk_commands.hpp"

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
    }

    VulkanContext::~VulkanContext() {
        const VkDevice device = m_device.GetLogical();
        if (device != VK_NULL_HANDLE) vkDeviceWaitIdle(device);

        m_meshes.clear();
    }

    bool VulkanContext::BeginFrame() {
        vulkan::CheckBool(!m_frameActive, "BeginFrame called while another frame is active");

        if (!m_swapchain.CanRender()) return false;
        if (m_swapchain.NeedsRecreation()) { m_swapchain.Recreate(); return false; }

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

        if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR) { m_swapchain.Recreate(); return false; }
        if (acquireResult != VK_SUCCESS && acquireResult != VK_SUBOPTIMAL_KHR) vulkan::CheckResult(acquireResult);

        m_activeImageIndex = imageIndex;
        m_swapchainSuboptimal = acquireResult == VK_SUBOPTIMAL_KHR;

        // Não reseta a cerca antes do acquire
        frame.ResetForSubmit();

        // Iniciando o command buffer
        const VkCommandBuffer commandBuffer = frame.GetCommandBuffer();
        VkCommandBufferBeginInfo beginInfo {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
        };

        vulkan::CheckResult(vkBeginCommandBuffer(commandBuffer, &beginInfo));

        // Prepara a imagem para receber renderização
        const VkImage swapchainImage = m_swapchain.GetImage(imageIndex);
        vulkan::TransitionImage(
            commandBuffer, swapchainImage,
            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_PIPELINE_STAGE_2_NONE, 0,
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT
        );

        // Configura o attachment de cor

        VkClearValue clearValue {};
        clearValue.color.float32[0] = m_clearColor.R;
        clearValue.color.float32[1] = m_clearColor.G;
        clearValue.color.float32[2] = m_clearColor.B;
        clearValue.color.float32[3] = m_clearColor.A;

        const VkImageView swapchainImageView = m_swapchain.GetImageView(imageIndex);
        VkRenderingAttachmentInfo colorAttachment {
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = swapchainImageView,
            .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue = clearValue
        };

        const VkExtent2D extent = m_swapchain.GetExtent();
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

        m_frameActive = true;
        return true;
    }

    void VulkanContext::EndFrame() {
        vulkan::CheckBool(m_frameActive, "EndFrame called without BeginFrame");

        vulkan::VulkanFrame& frame = m_frames[m_currentFrame];
        const VkCommandBuffer commandBuffer = frame.GetCommandBuffer();
        vkCmdEndRendering(commandBuffer);

        // Prepara a imagem para aparecer na tela
        const VkImage swapchainImage = m_swapchain.GetImage(m_activeImageIndex);
        vulkan::TransitionImage(
            commandBuffer, swapchainImage,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
            VK_PIPELINE_STAGE_2_NONE, 0
        );

        vulkan::CheckResult(vkEndCommandBuffer(commandBuffer));

        // Enviando para a GPU
        const VkSemaphore renderFinishedSemaphore = m_swapchain.GetRenderFinishedSemaphore(m_activeImageIndex);
        VkSemaphoreSubmitInfo waitSemaphoreInfo {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .semaphore = frame.GetImageAvailableSemaphore(),
            .value = 0,
            .stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            .deviceIndex = 0
        };

        VkCommandBufferSubmitInfo commandBufferInfo {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
            .commandBuffer = commandBuffer,
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
        const VkSwapchainKHR swapchain = m_swapchain.GetHandle();
        VkPresentInfoKHR presentInfo {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &renderFinishedSemaphore,
            .swapchainCount = 1,
            .pSwapchains = &swapchain,
            .pImageIndices = &m_activeImageIndex
        };

        const VkResult presentResult = vkQueuePresentKHR(m_device.GetQueue(), &presentInfo);
        const bool mustRecreate = presentResult == VK_ERROR_OUT_OF_DATE_KHR ||
                                  presentResult == VK_SUBOPTIMAL_KHR ||
                                  m_swapchainSuboptimal;

        m_frameActive = false;
        m_swapchainSuboptimal = false;

        m_currentFrame = (m_currentFrame + 1) % FRAMES_IN_FLIGHT;

        if (presentResult != VK_SUCCESS &&
            presentResult != VK_SUBOPTIMAL_KHR &&
            presentResult != VK_ERROR_OUT_OF_DATE_KHR
        ) vulkan::CheckResult(presentResult);

        if (mustRecreate && m_swapchain.CanRender()) m_swapchain.Recreate();
    }

    void VulkanContext::SetClearColor(Color color) {
        m_clearColor = color;
    }

    MeshHandle VulkanContext::CreateMesh(std::span<const Vertex> vertices, std::span<const std::uint32_t> indices) {
        // Primeiro procura um espaço liberado
        for (uint32_t i = 0; i < m_meshes.size(); ++i) {
            MeshSlot& slot = m_meshes[i];

            if (!slot.Mesh.has_value()) {
                slot.Mesh.emplace(
                    m_allocator, m_commandPool, m_device.GetQueue(),
                    vertices, indices
                );

                return MeshHandle {
                    .Index = i,
                    .Generation = slot.Generation
                };
            }
        }

        // Se não encontrar, cria um novo
        vulkan::CheckBool(m_meshes.size() < static_cast<size_t>(
            MeshHandle::InvalidIndex
        ), "Mesh storage has reached its maximum capacity");

        const auto index = static_cast<uint32_t>(m_meshes.size());
        m_meshes.emplace_back();

        MeshSlot& slot = m_meshes.back();
        slot.Mesh.emplace(
            m_allocator, m_commandPool, m_device.GetQueue(),
            vertices, indices
        );

        return MeshHandle {
            .Index = index,
            .Generation = slot.Generation
        };
    }

    void VulkanContext::DestroyMesh(MeshHandle handle) {
        vulkan::CheckBool(!m_frameActive, "Cannot destroy a mesh during an active frame");

        MeshSlot& slot = getMeshSlot(handle);
        vulkan::CheckResult(vkDeviceWaitIdle(m_device.GetLogical()));

        slot.Mesh.reset();
        ++slot.Generation;
    }

    void VulkanContext::DrawMesh(MeshHandle handle) {
        vulkan::CheckBool(m_frameActive, "DrawMesh must be called between BeginFrame and EndFrame");

        MeshSlot& slot = getMeshSlot(handle);
        slot.Mesh->Draw(getActiveCommandBuffer());
    }

    VulkanContext::MeshSlot& VulkanContext::getMeshSlot(MeshHandle handle) {
        vulkan::CheckBool(handle.IsValid(), "Received an invalid mesh handle");
        vulkan::CheckBool(handle.Index < m_meshes.size(), "Mesh handle index is out of bounds");

        MeshSlot& slot = m_meshes[handle.Index];
        vulkan::CheckBool(slot.Generation == handle.Generation, "Mesh handle generation does not match");
        vulkan::CheckBool(slot.Mesh.has_value(), "Mesh handle refers to a destroyed mesh");

        return slot;
    }

    VkCommandBuffer VulkanContext::getActiveCommandBuffer() const { return m_frames[m_currentFrame].GetCommandBuffer(); }
}
