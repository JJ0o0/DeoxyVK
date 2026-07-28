#pragma once

#include <volk.h>
#include <limits>

namespace deoxy::graphics::vulkan {
    class VulkanDevice {
        public:
            VulkanDevice(VkInstance instance, VkSurfaceKHR surface);
            ~VulkanDevice();

            VulkanDevice(const VulkanDevice&) = delete;
            VulkanDevice& operator=(const VulkanDevice&) = delete;

            VkPhysicalDevice GetPhysical() const { return m_physicalDevice; }
            VkDevice GetLogical() const { return m_device; }
            VkQueue GetQueue() const { return m_queue; }
            uint32_t GetGraphicsQueueFamily() const { return m_graphicsQueueFamily; }
        private:
            void destroy();

            void selectPhysicalDevice();
            void findQueueFamilies();
            void createLogicalDevice();

            VkInstance m_instance = VK_NULL_HANDLE;
            VkSurfaceKHR m_surface = VK_NULL_HANDLE;

            VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
            VkDevice m_device = VK_NULL_HANDLE;

            VkQueue m_queue = VK_NULL_HANDLE;
            uint32_t m_graphicsQueueFamily = std::numeric_limits<uint32_t>::max();
    };
}
