#include "vk_surface.hpp"
#include "vk_helper.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

namespace deoxy::graphics::vulkan {
    VulkanSurface::VulkanSurface(VkInstance instance, SDL_Window* window)
        : m_instance(instance) {
        try {
            // Verificando se recebemos um ponteiro vazio
            CheckBool(window != nullptr, "Surface received a null SDL Window");

            // Criando a surface via SDL
            CheckBool(SDL_Vulkan_CreateSurface(
                window, m_instance,
                nullptr,
                &m_surface
            ), "Could not create SDL Vulkan Surface");
        } catch (...) { destroy(); throw; }
    }

    VulkanSurface::~VulkanSurface() { destroy(); }

    void VulkanSurface::destroy() {
        if (m_surface != VK_NULL_HANDLE) vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    }
}
