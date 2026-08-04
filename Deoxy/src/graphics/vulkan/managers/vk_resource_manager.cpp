#include "vk_resource_manager.hpp"
#include "../graphical/vk_texture_create_info.hpp"
#include "../shading/vk_descriptor_set_layout.hpp"
#include "../components/vk_descriptor_pool.hpp"
#include "../components/vk_command_pool.hpp"
#include "../components/vk_allocator.hpp"
#include "../components/vk_device.hpp"
#include "../components/vk_helper.hpp"

#include <deoxy/math/scalar.hpp>

#include <array>

namespace deoxy::graphics::vulkan {
    VulkanResourceManager::VulkanResourceManager(
        VulkanDevice& device,
        VulkanAllocator& allocator,
        VulkanCommandPool& commandPool,
        VulkanDescriptorSetLayout& textureSetLayout,
        VulkanDescriptorPool& textureDescriptorPool
    ) : m_device(device),
        m_allocator(allocator),
        m_commandPool(commandPool),
        m_textureSetLayout(textureSetLayout),
        m_textureDescriptorPool(textureDescriptorPool) {
        createDefaultWhiteTexture();
    }

    MeshHandle VulkanResourceManager::CreateMesh(std::span<const Vertex> vertices, std::span<const std::uint32_t> indices) {
        // Primeiro procura um espaço liberado
        for (uint32_t i = 0; i < m_meshes.size(); ++i) {
            MeshSlot& slot = m_meshes[i];

            if (!slot.Resource.has_value()) {
                slot.Resource.emplace(
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
        CheckBool(m_meshes.size() < static_cast<size_t>(
            MeshHandle::InvalidIndex
        ), "Mesh storage has reached its maximum capacity");

        const auto index = static_cast<uint32_t>(m_meshes.size());
        m_meshes.emplace_back();

        MeshSlot& slot = m_meshes.back();
        slot.Resource.emplace(
            m_allocator, m_commandPool, m_device.GetQueue(),
            vertices, indices
        );

        return MeshHandle {
            .Index = index,
            .Generation = slot.Generation
        };
    }

    void VulkanResourceManager::DestroyMesh(MeshHandle handle) {
        MeshSlot& slot = getMeshSlot(handle);
        slot.Resource.reset();
        ++slot.Generation;
    }

    TextureHandle VulkanResourceManager::CreateTexture(const ImageData& data, const TextureCreateInfo& createInfo) {
        // Verificando configurações
        const bool anisotropyEnabled = createInfo.EnableAnisotropy && m_device.SupportsSamplerAnisotropy();
        if (anisotropyEnabled) CheckBool(createInfo.AnisotropyLevel >= 1.0f, "Texture anisotropy level must be at least 1.0");

        const float anisotropyLevel = anisotropyEnabled
            ? math::Clamp(
                createInfo.AnisotropyLevel,
                1.0f,
                m_device.GetMaxSamplerAnisotropy()
            ) : 1.0f;

        VulkanTextureCreateInfo vulkanCI {
            .Format = getTextureFormat(createInfo.ColorSpace),
            .Filter = getTextureFilter(createInfo.Filter),
            .MipmapFilter = getTextureMipmapFilter(createInfo.MipmapFilter),
            .WrapMode = getTextureWrapMode(createInfo.WrapMode),
            .GenerateMipmaps = createInfo.GenerateMipmaps,
            .AnisotropyEnabled = anisotropyEnabled,
            .MaxAnisotropy = anisotropyLevel
        };

        if (createInfo.GenerateMipmaps) {
            CheckBool(hasMipmappingSupport(
                m_device.GetPhysical(), vulkanCI.Format
            ), "Texture format doesn't support linear mipmap generation");
        }

        // Procura um espaço liberado
        for (uint32_t i = 0; i < m_textures.size(); ++i) {
            TextureSlot& slot = m_textures[i];

            if (!slot.Texture.has_value()) {
                initializeTextureSlot(vulkanCI, slot, data);

                return TextureHandle {
                    .Index = i,
                    .Generation = slot.Generation
                };
            }
        }

        // Se não encontrar, cria um novo
        CheckBool(m_textures.size() < static_cast<size_t>(
            TextureHandle::InvalidIndex
        ), "Texture storage has reached its maximum capacity");

        const auto index = static_cast<uint32_t>(m_textures.size());
        m_textures.emplace_back();

        TextureSlot& slot = m_textures.back();
        initializeTextureSlot(vulkanCI, slot, data);

        return TextureHandle {
            .Index = index,
            .Generation = slot.Generation
        };
    }

    void VulkanResourceManager::DestroyTexture(TextureHandle handle) {
        TextureSlot& slot = getTextureSlot(handle);
        slot.Texture.reset();
        ++slot.Generation;
    }

    MaterialHandle VulkanResourceManager::CreateMaterial(const MaterialCreateInfo& data) {
        // Copiando material
        MaterialCreateInfo material = data;
        if (!material.Albedo.IsValid()) material.Albedo = m_defaultTexture;
        (void)getTextureSlot(material.Albedo);

        // Procura um espaço liberado
        for (uint32_t i = 0; i < m_materials.size(); ++i) {
            MaterialSlot& slot = m_materials[i];

            if (!slot.Resource.has_value()) {
                slot.Resource = material;

                return MaterialHandle {
                    .Index = i,
                    .Generation = slot.Generation
                };
            }
        }

        // Se não encontrar, cria um novo
        CheckBool(m_materials.size() < static_cast<size_t>(
            MaterialHandle::InvalidIndex
        ), "Material storage has reached its maximum capacity");

        const auto index = static_cast<uint32_t>(m_materials.size());
        m_materials.emplace_back();

        MaterialSlot& slot = m_materials.back();
        slot.Resource = material;

        return MaterialHandle {
            .Index = index,
            .Generation = slot.Generation
        };
    }

    void VulkanResourceManager::DestroyMaterial(MaterialHandle handle) {
        MaterialSlot& slot = getMaterialSlot(handle);
        slot.Resource.reset();
        ++slot.Generation;
    }

    VulkanMesh& VulkanResourceManager::GetMesh(MeshHandle handle) {
        MeshSlot& slot = getMeshSlot(handle);
        return slot.Resource.value();
    }

    const MaterialCreateInfo& VulkanResourceManager::GetMaterial(MaterialHandle handle) {
        MaterialSlot& slot = getMaterialSlot(handle);
        return slot.Resource.value();
    }

    VkDescriptorSet VulkanResourceManager::GetTextureDescriptorSet(TextureHandle handle) {
        TextureSlot& slot = getTextureSlot(handle);
        return slot.DescriptorSet;
    }

    VulkanResourceManager::MeshSlot& VulkanResourceManager::getMeshSlot(MeshHandle handle) {
        CheckBool(handle.IsValid(), "Received an invalid mesh handle");
        CheckBool(handle.Index < m_meshes.size(), "Mesh handle index is out of bounds");

        MeshSlot& slot = m_meshes[handle.Index];
        CheckBool(slot.Generation == handle.Generation, "Mesh handle generation does not match");
        CheckBool(slot.Resource.has_value(), "Mesh handle refers to a destroyed mesh");

        return slot;
    }

    VulkanResourceManager::TextureSlot& VulkanResourceManager::getTextureSlot(TextureHandle handle) {
        CheckBool(handle.IsValid(), "Received an invalid texture handle");
        CheckBool(handle.Index < m_textures.size(), "Texture handle index is out of bounds");

        TextureSlot& slot = m_textures[handle.Index];
        CheckBool(slot.Generation == handle.Generation, "Texture handle generation does not match");
        CheckBool(slot.Texture.has_value(), "Texture handle refers to a destroyed texture");

        return slot;
    }

    VulkanResourceManager::MaterialSlot& VulkanResourceManager::getMaterialSlot(MaterialHandle handle) {
        CheckBool(handle.IsValid(), "Received an invalid material handle");
        CheckBool(handle.Index < m_materials.size(), "Material handle index is out of bounds");

        MaterialSlot& slot = m_materials[handle.Index];
        CheckBool(slot.Generation == handle.Generation, "Material handle generation does not match");
        CheckBool(slot.Resource.has_value(), "Material handle refers to a destroyed material");

        return slot;
    }

    void VulkanResourceManager::initializeTextureSlot(const VulkanTextureCreateInfo& createInfo, TextureSlot& slot, const ImageData& data) {
        CheckBool(!slot.Texture.has_value(), "Cannot initialize an occupied texture slot");

        if (slot.DescriptorSet == VK_NULL_HANDLE) {
            const std::array<VkDescriptorSetLayout, 1> layouts {m_textureSetLayout.GetHandle()};
            const std::vector<VkDescriptorSet> descriptorSets = m_textureDescriptorPool.Allocate(layouts);

            CheckBool(descriptorSets.size() == 1, "Texture descriptor allocation returned an unexpected set count");
            slot.DescriptorSet = descriptorSets[0];

            CheckBool(slot.DescriptorSet != VK_NULL_HANDLE, "Failed to allocate texture descriptor set");
        }

        slot.Texture.emplace(
            m_device.GetLogical(), m_allocator, m_commandPool, m_device.GetQueue(),
            data, createInfo
        );

        VkDescriptorImageInfo imageInfo{
            .sampler = slot.Texture->GetSampler(),
            .imageView = slot.Texture->GetImageView(),
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        };

        VkWriteDescriptorSet write{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = slot.DescriptorSet,
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = &imageInfo
        };

        vkUpdateDescriptorSets(m_device.GetLogical(), 1, &write, 0, nullptr);
    }

    void VulkanResourceManager::createDefaultWhiteTexture() {
        ImageData data {
            .Pixels = { 255, 255, 255, 255 },
            .Width = 1, .Height = 1,
        };

        m_defaultTexture = CreateTexture(data, {});
        CheckBool(m_defaultTexture.IsValid(), "Failed to create the default white texture");
    }

    bool VulkanResourceManager::hasMipmappingSupport(VkPhysicalDevice physicalDevice, VkFormat format) {
        CheckBool(physicalDevice != VK_NULL_HANDLE, "Cannot verify mipmapping support with a unspecified physical device");
        CheckBool(format != VK_FORMAT_UNDEFINED, "Cannot verify mipmapping support with a unspecified format");

        VkFormatProperties formatProps{};
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProps);

        const VkFormatFeatureFlags requiredFeatures = VK_FORMAT_FEATURE_BLIT_SRC_BIT |
                                                      VK_FORMAT_FEATURE_BLIT_DST_BIT |
                                                      VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;

        const bool hasRequired = (formatProps.optimalTilingFeatures & requiredFeatures) == requiredFeatures;
        return hasRequired;
    }

    VkFormat VulkanResourceManager::getTextureFormat(TextureColorSpace colorSpace) {
        switch (colorSpace) {
            case TextureColorSpace::SRGB: return VK_FORMAT_R8G8B8A8_SRGB;
            case TextureColorSpace::Linear: return VK_FORMAT_R8G8B8A8_UNORM;
            default: return VK_FORMAT_R8G8B8A8_SRGB;
        }
    }

    VkFilter VulkanResourceManager::getTextureFilter(TextureFilter filter) {
        switch (filter) {
            case TextureFilter::Linear: return VK_FILTER_LINEAR;
            case TextureFilter::Nearest: return VK_FILTER_NEAREST;
            default: return VK_FILTER_LINEAR;
        }
    }

    VkSamplerMipmapMode VulkanResourceManager::getTextureMipmapFilter(TextureMipmapFilter mipmapFilter) {
        switch (mipmapFilter) {
            case TextureMipmapFilter::Linear: return VK_SAMPLER_MIPMAP_MODE_LINEAR;
            case TextureMipmapFilter::Nearest: return VK_SAMPLER_MIPMAP_MODE_NEAREST;
            default: return VK_SAMPLER_MIPMAP_MODE_LINEAR;
        }
    }

    VkSamplerAddressMode VulkanResourceManager::getTextureWrapMode(TextureWrapMode wrapMode) {
        switch (wrapMode) {
            case TextureWrapMode::Repeat: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
            case TextureWrapMode::MirroredRepeat: return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
            case TextureWrapMode::ClampToEdge: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            default: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        }
    }
}
