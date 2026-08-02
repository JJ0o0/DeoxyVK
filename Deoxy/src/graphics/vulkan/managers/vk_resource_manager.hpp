#pragma once

#include "../components/vk_resource_slot.hpp"
#include "../components/vk_texture.hpp"
#include "../graphical/vk_mesh.hpp"

#include <deoxy/graphics/graphical_handles.hpp>
#include <deoxy/graphics/image_data.hpp>
#include <deoxy/graphics/material.hpp>
#include <deoxy/graphics/vertex.hpp>

#include <optional>
#include <cstdint>
#include <vector>
#include <volk.h>
#include <span>

namespace deoxy::graphics::vulkan {
    class VulkanDevice;
    class VulkanAllocator;
    class VulkanCommandPool;
    class VulkanDescriptorSetLayout;
    class VulkanDescriptorPool;
    class VulkanResourceManager {
        public:
            VulkanResourceManager(
                VulkanDevice& device,
                VulkanAllocator& allocator,
                VulkanCommandPool& commandPool,
                VulkanDescriptorSetLayout& textureSetLayout,
                VulkanDescriptorPool& textureDescriptorPool
            );

            MeshHandle CreateMesh(std::span<const Vertex> vertices, std::span<const uint32_t> indices);
            void DestroyMesh(MeshHandle handle);

            TextureHandle CreateTexture(const ImageData& data);
            void DestroyTexture(TextureHandle handle);

            MaterialHandle CreateMaterial(const MaterialCreateInfo& data);
            void DestroyMaterial(MaterialHandle handle);

            VulkanMesh& GetMesh(MeshHandle handle);
            const MaterialCreateInfo& GetMaterial(MaterialHandle handle);
            VkDescriptorSet GetTextureDescriptorSet(TextureHandle handle);
        private:
            VulkanDevice& m_device;
            VulkanAllocator& m_allocator;
            VulkanCommandPool& m_commandPool;
            VulkanDescriptorSetLayout& m_textureSetLayout;
            VulkanDescriptorPool& m_textureDescriptorPool;

            struct TextureSlot {
                std::optional<VulkanTexture> Texture;
                VkDescriptorSet DescriptorSet = VK_NULL_HANDLE;
                uint32_t Generation = 0;
            };

            using MeshSlot = ResourceSlot<VulkanMesh>;
            using MaterialSlot = ResourceSlot<MaterialCreateInfo>;

            MeshSlot& getMeshSlot(MeshHandle handle);
            TextureSlot& getTextureSlot(TextureHandle handle);
            MaterialSlot& getMaterialSlot(MaterialHandle handle);

            TextureHandle m_defaultTexture;
            std::vector<MeshSlot> m_meshes;
            std::vector<TextureSlot> m_textures;
            std::vector<MaterialSlot> m_materials;

            void initializeTextureSlot(TextureSlot& slot, const ImageData& data);
            void createDefaultWhiteTexture();
    };
}
