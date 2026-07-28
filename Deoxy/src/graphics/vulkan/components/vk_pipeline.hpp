#pragma once

#include <volk.h>

#include <filesystem>
#include <vector>
#include <cstdint>

namespace deoxy::graphics::vulkan {
    class VulkanPipeline {
        public:
            VulkanPipeline(
                VkDevice device, VkFormat colorFormat,
                const std::filesystem::path& vertexShaderPath,
                const std::filesystem::path& fragmentShaderPath
            );

            ~VulkanPipeline();

            VulkanPipeline(const VulkanPipeline&) = delete;
            VulkanPipeline& operator=(const VulkanPipeline&) = delete;

            VulkanPipeline(VulkanPipeline&&) = delete;
            VulkanPipeline& operator=(VulkanPipeline&&) = delete;

            VkPipeline GetHandle() const { return m_pipeline; }
            VkPipelineLayout GetLayout() const { return m_layout; }
        private:
            VkDevice m_device = VK_NULL_HANDLE;

            void create(
                VkFormat colorFormat,
                const std::filesystem::path& vertexShaderPath,
                const std::filesystem::path& fragmentShaderPath
            );

            void destroy();

            VkShaderModule createShaderModule(const std::vector<uint32_t>& code) const;
            static std::vector<uint32_t> readShaderFile(const std::filesystem::path& path);

            VkPipeline m_pipeline = VK_NULL_HANDLE;
            VkPipelineLayout m_layout = VK_NULL_HANDLE;
    };
}
