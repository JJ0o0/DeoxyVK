#pragma once

#include "components/vk_allocator.hpp"
#include "components/vk_command_pool.hpp"
#include "components/vk_device.hpp"
#include "components/vk_frame.hpp"
#include "components/vk_instance.hpp"
#include "components/vk_pipeline.hpp"
#include "components/vk_surface.hpp"
#include "components/vk_swapchain.hpp"
#include "components/vk_buffer.hpp"
#include "components/vk_descriptor_pool.hpp"

#include "graphical/vk_mesh.hpp"

#include "shading/vk_uniforms.hpp"
#include "shading/vk_descriptor_set_layout.hpp"

#include <deoxy/graphics/mesh_handle.hpp>
#include <deoxy/graphics/color.hpp>
#include <deoxy/math/math.hpp>

#include <volk.h>
#include <vk_mem_alloc.h>

#include <optional>
#include <cstdint>
#include <vector>
#include <array>

namespace deoxy::platform {
    class Window;
};

namespace deoxy::graphics {
    class VulkanContext {
        public:
            VulkanContext(platform::Window& window);
            ~VulkanContext();

            VulkanContext(const VulkanContext&) = delete;
            VulkanContext& operator=(const VulkanContext&) = delete;

            bool BeginFrame();
            void EndFrame();

            void SetClearColor(Color color);

            MeshHandle CreateMesh(std::span<const Vertex> vertices, std::span<const uint32_t> indices);
            void DestroyMesh(MeshHandle handle);
            void DrawMesh(MeshHandle handle, const math::Mat4& modelMatrix);

            void SetCamera(const math::Mat4& view, const math::Mat4& projection);
        private:
            static constexpr uint32_t FRAMES_IN_FLIGHT = 2;

            Color m_clearColor { 0.05f, 0.1f, 0.2f };

            struct MeshSlot {
                std::optional<vulkan::VulkanMesh> Mesh;
                uint32_t Generation = 0;
            };

            std::vector<MeshSlot> m_meshes;
            MeshSlot& getMeshSlot(MeshHandle handle);

            void updateCameraDescriptorSets();

            // ORDEM É IMPORTANTE!
            // Construção: Cima pra Baixo
            // Destruição: Baixo pra Cima

            vulkan::VulkanInstance m_instance;
            vulkan::VulkanSurface m_surface;
            vulkan::VulkanDevice m_device;
            vulkan::VulkanAllocator m_allocator;

            vulkan::VulkanCommandPool m_commandPool;
            vulkan::VulkanSwapchain m_swapchain;

            vulkan::VulkanDescriptorSetLayout m_cameraSetLayout;
            vulkan::VulkanDescriptorPool m_descriptorPool;
            std::array<vulkan::VulkanFrame, FRAMES_IN_FLIGHT> m_frames;
            vulkan::VulkanPipeline m_pipeline;

            bool m_frameActive = false;
            bool m_swapchainSuboptimal = false;
            uint32_t m_activeImageIndex = 0;
            uint32_t m_currentFrame = 0;

            vulkan::CameraUniformData m_cameraData{};

            VkCommandBuffer getActiveCommandBuffer() const;
    };
}
