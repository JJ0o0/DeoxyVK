#include "vk_context.hpp"
#include "components/vk_helper.hpp"
#include "components/vk_commands.hpp"
#include "components/vk_depth_buffer.hpp"
#include "components/vk_texture.hpp"
#include "shading/vk_push_constants.hpp"
#include "shading/vk_uniforms.hpp"

#include <deoxy/graphics/graphical_handles.hpp>
#include <deoxy/graphics/image_loader.hpp>
#include <deoxy/graphics/image_data.hpp>
#include <deoxy/platform/window.hpp>
#include <deoxy/platform/logger.hpp>
#include <deoxy/graphics/vertex.hpp>

#include <SDL3/SDL_vulkan.h>

#include <vector>
#include <array>

namespace deoxy::graphics {
    VulkanContext::VulkanContext(platform::Window& window)
        : m_instance(),
          m_surface(m_instance.GetHandle(), window.GetHandle()),
          m_device(m_instance.GetHandle(), m_surface.GetHandle()),
          m_allocator(m_instance.GetHandle(), m_device.GetPhysical(), m_device.GetLogical()),
          m_commandPool(m_device.GetLogical(), m_device.GetGraphicsQueueFamily()),
          m_swapchain(window.GetHandle(), m_surface, m_device, m_allocator),
          m_cameraSetLayout(m_device.GetLogical(), vulkan::CameraBindings),
          m_descriptorPool(m_device.GetLogical(), vulkan::CreateCameraPoolSizes(FRAMES_IN_FLIGHT), FRAMES_IN_FLIGHT),
          m_textureSetLayout(m_device.GetLogical(), vulkan::TextureBindings),
          m_textureDescriptorPool(m_device.GetLogical(), vulkan::CreateTexturePoolSizes(MAX_TEXTURES), MAX_TEXTURES),
          m_frames{
              vulkan::VulkanFrame{ m_device.GetLogical(), m_commandPool, m_allocator },
              vulkan::VulkanFrame{ m_device.GetLogical(), m_commandPool, m_allocator }
          },
          m_pipeline(
              m_device.GetLogical(),
              m_swapchain.GetColorFormat(), m_swapchain.GetDepthFormat(),
              m_cameraSetLayout.GetHandle(), m_textureSetLayout.GetHandle(),
              "assets/shaders/basic.vert.spv", "assets/shaders/basic.frag.spv"),
          m_resourceManager(
              m_device, m_allocator, m_commandPool,
              m_textureSetLayout, m_textureDescriptorPool
          ) {
        std::array<VkDescriptorSetLayout, FRAMES_IN_FLIGHT> layouts{};
        layouts.fill(m_cameraSetLayout.GetHandle());

        const std::vector<VkDescriptorSet> descriptorSets = m_descriptorPool.Allocate(layouts);
        for (size_t i = 0; i < FRAMES_IN_FLIGHT; ++i) { m_frames[i].SetCameraDescriptorSet(descriptorSets[i]); }

        updateCameraDescriptorSets();
    }

    VulkanContext::~VulkanContext() {
        const VkDevice device = m_device.GetLogical();
        if (device != VK_NULL_HANDLE) vkDeviceWaitIdle(device);
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

        frame.GetCameraBuffer().Upload(&m_cameraData, sizeof(m_cameraData));

        // Não reseta a cerca antes do acquire
        frame.ResetForSubmit();

        // Iniciando o command buffer
        const VkCommandBuffer commandBuffer = frame.GetCommandBuffer();
        VkCommandBufferBeginInfo beginInfo {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
        };

        vulkan::CheckResult(vkBeginCommandBuffer(commandBuffer, &beginInfo));

        // Prepara a imagem de cor
        const VkImage swapchainImage = m_swapchain.GetImage(imageIndex);
        vulkan::TransitionImage(
            commandBuffer, swapchainImage,
            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_PIPELINE_STAGE_2_NONE, 0,
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
            VK_IMAGE_ASPECT_COLOR_BIT
        );

        // Prepara a imagem de Depth
        const vulkan::VulkanDepthBuffer& depthBuffer = m_swapchain.GetDepthBuffer(imageIndex);
        vulkan::TransitionImage(
            commandBuffer, depthBuffer.GetImage(),
            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
            VK_PIPELINE_STAGE_2_NONE, 0,
            VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
            VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            VK_IMAGE_ASPECT_DEPTH_BIT
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

        // Configura o attachment de depth
        VkClearValue depthClearValue {};
        depthClearValue.depthStencil.depth = 1.0f;
        depthClearValue.depthStencil.stencil = 0;

        VkRenderingAttachmentInfo depthAttachment {
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = depthBuffer.GetImageView(),
            .imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .clearValue = depthClearValue
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
            .pColorAttachments = &colorAttachment,
            .pDepthAttachment = &depthAttachment
        };

        // Limpa a imagem
        vkCmdBeginRendering(commandBuffer, &renderingInfo);
            // Bindando a pipeline
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline.GetHandle());
                // Bindando descriptor sets
                const VkDescriptorSet cameraDescriptorSet = frame.GetCameraDescriptorSet();
                vulkan::CheckBool(cameraDescriptorSet != VK_NULL_HANDLE, "Current frame has no camera descriptor set");

                vkCmdBindDescriptorSets(
                    commandBuffer,
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    m_pipeline.GetLayout(),
                    0,
                    1,
                    &cameraDescriptorSet,
                    0,
                    nullptr
                );

                // Criando o viewport
                VkViewport viewport {
                    .x = 0.0f,
                    .y = static_cast<float>(extent.height),
                    .width = static_cast<float>(extent.width),
                    .height = -static_cast<float>(extent.height),
                    .minDepth = 0.0f,
                    .maxDepth = 1.0f
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

    void VulkanContext::DrawMesh(MeshHandle meshHandle, MaterialHandle materialHandle, const math::Mat4& modelMatrix) {
        vulkan::CheckBool(m_frameActive, "DrawMesh must be called between BeginFrame and EndFrame");

        vulkan::VulkanMesh& mesh = m_resourceManager.GetMesh(meshHandle);

        const MaterialCreateInfo& materialCI = m_resourceManager.GetMaterial(materialHandle);
        const VkDescriptorSet textureDescriptor = m_resourceManager.GetTextureDescriptorSet(materialCI.Albedo);
        const VkCommandBuffer commandBuffer = getActiveCommandBuffer();

        vulkan::PushConstants pushConstants {
            .ModelMatrix = modelMatrix,
            .MaterialTint = materialCI.Tint,
            .MaterialUVScale = materialCI.UVScale,
            .MaterialUVOffset = materialCI.UvOffset
        };

        vkCmdPushConstants(
            commandBuffer,
            m_pipeline.GetLayout(),
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0, sizeof(vulkan::PushConstants),
            &pushConstants
        );

        vkCmdBindDescriptorSets(
            commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_pipeline.GetLayout(),
            1, 1,
            &textureDescriptor,
            0, nullptr
        );

        mesh.Draw(commandBuffer);
    }

    MeshHandle VulkanContext::CreateMesh(std::span<const Vertex> vertices, std::span<const std::uint32_t> indices) {
        return m_resourceManager.CreateMesh(vertices, indices);
    }

    void VulkanContext::DestroyMesh(MeshHandle handle) {
        vulkan::CheckBool(!m_frameActive, "Cannot destroy a mesh during an active frame");
        vulkan::CheckResult(vkDeviceWaitIdle(m_device.GetLogical()));
        m_resourceManager.DestroyMesh(handle);
    }

    TextureHandle VulkanContext::CreateTexture(const ImageData& data, const TextureCreateInfo& createInfo) {
        return m_resourceManager.CreateTexture(data, createInfo);
    }

    void VulkanContext::DestroyTexture(TextureHandle handle) {
        vulkan::CheckBool(!m_frameActive, "Cannot destroy a texture during an active frame");
        vulkan::CheckResult(vkDeviceWaitIdle(m_device.GetLogical()));
        m_resourceManager.DestroyTexture(handle);
    }

    MaterialHandle VulkanContext::CreateMaterial(const MaterialCreateInfo& data) {
        return m_resourceManager.CreateMaterial(data);
    }

    void VulkanContext::DestroyMaterial(MaterialHandle handle) {
        vulkan::CheckBool(!m_frameActive, "Cannot destroy a material during an active frame");
        m_resourceManager.DestroyMaterial(handle);
    }

    void VulkanContext::SetCamera(const math::Mat4& view, const math::Mat4& projection) {
        m_cameraData.View = view;
        m_cameraData.Projection = projection;
    }

    void VulkanContext::updateCameraDescriptorSets() {
        for (size_t i = 0; i < FRAMES_IN_FLIGHT; ++i) {
            vulkan::VulkanFrame& frame = m_frames[i];

            VkDescriptorBufferInfo bufferInfo {
                .buffer = frame.GetCameraBuffer().GetHandle(),
                .offset = 0,
                .range = sizeof(vulkan::CameraUniformData)
            };

            VkWriteDescriptorSet write {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = frame.GetCameraDescriptorSet(),
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .pBufferInfo = &bufferInfo
            };

            vkUpdateDescriptorSets(m_device.GetLogical(), 1, &write, 0, nullptr);
        }
    }

    VkCommandBuffer VulkanContext::getActiveCommandBuffer() const { return m_frames[m_currentFrame].GetCommandBuffer(); }
}
