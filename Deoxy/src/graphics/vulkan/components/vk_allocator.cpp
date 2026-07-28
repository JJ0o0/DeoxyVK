#include "vk_allocator.hpp"
#include "vk_helper.hpp"

namespace deoxy::graphics::vulkan {
    VulkanAllocator::VulkanAllocator(
        VkInstance instance,
        VkPhysicalDevice physicalDevice,
        VkDevice device
    ) {
        // Criando o alocador de memória
        VmaAllocatorCreateInfo allocatorCI {
            .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
            .physicalDevice = physicalDevice,
            .device = device,
            .instance = instance,
            .vulkanApiVersion = VK_API_VERSION_1_3
        };

        // Pegando funções do VMA usando o Volk
        VmaVulkanFunctions vkFunctions{};
        CheckResult(vmaImportVulkanFunctionsFromVolk(&allocatorCI, &vkFunctions));

        allocatorCI.pVulkanFunctions = &vkFunctions;
        CheckResult(vmaCreateAllocator(&allocatorCI, &m_allocator));
    }

    VulkanAllocator::~VulkanAllocator() { destroy(); }

    void VulkanAllocator::destroy() {
        if (m_allocator != nullptr) vmaDestroyAllocator(m_allocator);
    }
}
