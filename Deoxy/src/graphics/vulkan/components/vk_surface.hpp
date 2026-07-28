#pragma once

#include <volk.h>

struct SDL_Window;

namespace deoxy::graphics::vulkan {
    class VulkanSurface {
        public:
            VulkanSurface(VkInstance instance, SDL_Window* window);
            ~VulkanSurface();

            VulkanSurface(const VulkanSurface&) = delete;
            VulkanSurface& operator=(const VulkanSurface&) = delete;

            VkSurfaceKHR GetHandle() const { return m_surface; }
        private:
            void destroy();

            VkInstance m_instance = VK_NULL_HANDLE;
            VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    };
}
