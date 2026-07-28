#include "vk_buffer.hpp"
#include "vk_allocator.hpp"
#include "vk_helper.hpp"

#include <utility>

namespace deoxy::graphics::vulkan {
    VulkanBuffer::VulkanBuffer(
        const VulkanAllocator& allocator,
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VmaMemoryUsage memoryUsage,
        VmaAllocationCreateFlags allocationFlags
    ) : m_allocator(allocator.GetHandle()), m_size(size) {
        CheckBool(m_allocator != nullptr, "Buffer received a null allocator");
        CheckBool(m_size > 0, "Buffer received a zero size");
        CheckBool(usage != 0, "Buffer received no usage flags");

        VkBufferCreateInfo bufferCI {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = m_size,
            .usage = usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE
        };

        VmaAllocationCreateInfo allocationCI {
            .flags = allocationFlags,
            .usage = memoryUsage
        };

        // Guardando em variáveis locais para só atualizar as váriaveis da classe após a criação funcionar
        VkBuffer buffer = VK_NULL_HANDLE;
        VmaAllocation allocation = nullptr;

        // Essa classe já vai jogar um runtime error
        // É só olhar a definição no 'vk_helper.hpp'
        vulkan::CheckResult(vmaCreateBuffer(
            m_allocator,
            &bufferCI,
            &allocationCI,
            &buffer, &allocation,
            nullptr
        ));

        m_buffer = buffer;
        m_allocation = allocation;
    }

    VulkanBuffer::~VulkanBuffer() { destroy(); }

    // FATO NÃO TÃO ENGRAÇADO: Descobri que dava pra usar exchange ao inves de fazer MANUALMENTE
    VulkanBuffer::VulkanBuffer(VulkanBuffer&& other) noexcept
        : m_allocator(std::exchange(other.m_allocator, nullptr)),
          m_buffer(std::exchange(other.m_buffer, VK_NULL_HANDLE)),
          m_allocation(std::exchange(other.m_allocation, nullptr)),
          m_size(std::exchange(other.m_size, 0)) {
    }

    VulkanBuffer& VulkanBuffer::operator=(VulkanBuffer&& other) noexcept {
        if (this == &other) return *this;

        destroy();

        m_allocator = std::exchange(other.m_allocator, nullptr);
        m_buffer = std::exchange(other.m_buffer, VK_NULL_HANDLE);
        m_allocation = std::exchange(other.m_allocation, nullptr);
        m_size = std::exchange(other.m_size, 0);

        return *this;
    }

    void VulkanBuffer::Upload(const void* data, VkDeviceSize size, VkDeviceSize offset) {
        CheckBool(data != nullptr, "Cannot upload null data to Buffer");
        CheckBool(size > 0, "Cannot upload zero bytes to Buffer");
        CheckBool(offset <= m_size, "Buffer upload offset is out of bounds");

        // Evitar overflow (offset + size <= m_size)
        CheckBool(size <= m_size - offset, "Buffer upload exceeds its size");

        CheckResult(vmaCopyMemoryToAllocation(
            m_allocator,
            data,
            m_allocation,
            offset, size
        ));
    }

    void VulkanBuffer::destroy() {
        if (m_allocator != nullptr && m_buffer != VK_NULL_HANDLE) vmaDestroyBuffer(m_allocator, m_buffer, m_allocation);

        // MEXER COM MEMÓRIA É PERIGOSO! Vou resestar todos os valores POR PRECAUÇÃO
        m_buffer = VK_NULL_HANDLE;
        m_allocation = nullptr;
        m_size = 0;
        m_allocator = nullptr;
    }
}
