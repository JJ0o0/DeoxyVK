#pragma once

#include "../graphical/vk_texture_create_info.hpp"
#include "../components/vk_resource_slot.hpp"
#include "../components/vk_texture.hpp"
#include "../graphical/vk_mesh.hpp"

#include <deoxy/graphics/lighting/point_light.hpp>
#include <deoxy/graphics/lighting/spot_light.hpp>
#include <deoxy/graphics/graphical_handles.hpp>
#include <deoxy/graphics/image_data.hpp>
#include <deoxy/graphics/material.hpp>
#include <deoxy/graphics/texture.hpp>
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
    struct FrameUniformData;
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

            TextureHandle CreateTexture(const ImageData& data, const TextureCreateInfo& createInfo);
            void DestroyTexture(TextureHandle handle);

            MaterialHandle CreateMaterial(const MaterialCreateInfo& data);
            void DestroyMaterial(MaterialHandle handle);

            std::optional<PointLightHandle> CreatePointLight(const PointLight& light);
            void WritePointLights(FrameUniformData& data) const;
            void UpdatePointLight(PointLightHandle handle, const PointLight& light);
            void DestroyPointLight(PointLightHandle handle);
            bool IsPointLightValid(PointLightHandle handle) const;

            std::optional<SpotLightHandle> CreateSpotLight(const SpotLight& light);
            void WriteSpotLights(FrameUniformData& data) const;
            void UpdateSpotLight(SpotLightHandle handle, const SpotLight& light);
            void DestroySpotLight(SpotLightHandle handle);
            bool IsSpotLightValid(SpotLightHandle handle) const;

            VulkanMesh& GetMesh(MeshHandle handle);
            const MaterialCreateInfo& GetMaterial(MaterialHandle handle);
            VkDescriptorSet GetTextureDescriptorSet(TextureHandle handle);

            const PointLight& GetPointLight(PointLightHandle handle);
            const SpotLight& GetSpotLight(SpotLightHandle handle);
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
            using PointLightSlot = ResourceSlot<PointLight>;
            using SpotLightSlot = ResourceSlot<SpotLight>;

            MeshSlot& getMeshSlot(MeshHandle handle);
            TextureSlot& getTextureSlot(TextureHandle handle);
            MaterialSlot& getMaterialSlot(MaterialHandle handle);
            PointLightSlot& getPointLightSlot(PointLightHandle handle);
            SpotLightSlot& getSpotLightSlot(SpotLightHandle handle);

            TextureHandle m_defaultTexture;
            std::vector<MeshSlot> m_meshes;
            std::vector<TextureSlot> m_textures;
            std::vector<MaterialSlot> m_materials;
            std::vector<PointLightSlot> m_pointLights;
            std::vector<SpotLightSlot> m_spotLights;

            void initializeTextureSlot(const VulkanTextureCreateInfo& createInfo, TextureSlot& slot, const ImageData& data);
            void createDefaultWhiteTexture();
            bool hasMipmappingSupport(VkPhysicalDevice physicalDevice, VkFormat format);

            VkFormat getTextureFormat(TextureColorSpace colorSpace);
            VkFilter getTextureFilter(TextureFilter filter);
            VkSamplerMipmapMode getTextureMipmapFilter(TextureMipmapFilter mipmapFilter);
            VkSamplerAddressMode getTextureWrapMode(TextureWrapMode wrapMode);
    };
}
