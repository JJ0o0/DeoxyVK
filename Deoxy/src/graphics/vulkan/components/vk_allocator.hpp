#pragma once

#include <volk.h>
#include <vk_mem_alloc.h>

namespace deoxy::graphics::vulkan {
    class VulkanAllocator {
        public:
            VulkanAllocator(
                VkInstance instance,
                VkPhysicalDevice physicalDevice,
                VkDevice device
            );

            ~VulkanAllocator();

            VulkanAllocator(const VulkanAllocator&) = delete;
            VulkanAllocator& operator=(const VulkanAllocator&) = delete;

            VmaAllocator GetHandle() const { return m_allocator; }
        private:
            void destroy();

            VmaAllocator m_allocator = nullptr;
    };
}
