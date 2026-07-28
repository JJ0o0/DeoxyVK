#pragma once

#include <volk.h>
#include <vk_mem_alloc.h>

namespace deoxy::graphics::vulkan {
    class VulkanAllocator;
    class VulkanBuffer {
        public:
            VulkanBuffer(
                const VulkanAllocator& allocator,
                VkDeviceSize size,
                VkBufferUsageFlags usage,
                VmaMemoryUsage memoryUsage,
                VmaAllocationCreateFlags allocationFlags = 0
            );

            ~VulkanBuffer();

            VulkanBuffer(const VulkanBuffer&) = delete;
            VulkanBuffer& operator=(const VulkanBuffer&) = delete;

            // Buffers podem ser movidos, aqui nós expecificamos como
            VulkanBuffer(VulkanBuffer&& other) noexcept;
            VulkanBuffer& operator=(VulkanBuffer&& other) noexcept;

            void Upload(const void* data, VkDeviceSize size, VkDeviceSize offset = 0);

            VkBuffer GetHandle() const { return m_buffer; }
            VkDeviceSize GetSize() const { return m_size; }

            bool IsValid() const { return m_buffer != VK_NULL_HANDLE; }
        private:
            VmaAllocator m_allocator = nullptr;

            void destroy();

            VkBuffer m_buffer = VK_NULL_HANDLE;
            VmaAllocation m_allocation = nullptr;
            VkDeviceSize m_size = 0;
    };
}
