#pragma once

#include <volk.h>
#include <vk_mem_alloc.h>

#include <cstddef>
#include <cstdint>
#include <vector>

struct SDL_Window;
namespace deoxy::graphics::vulkan {
    class VulkanSurface;
    class VulkanDevice;
    class VulkanAllocator;
    class VulkanDepthBuffer;
    class VulkanSwapchain {
        public:
            VulkanSwapchain(
                SDL_Window* window,
                const VulkanSurface& surface,
                const VulkanDevice& device,
                const VulkanAllocator& allocator
            );

            ~VulkanSwapchain();

            VulkanSwapchain(const VulkanSwapchain&) = delete;
            VulkanSwapchain& operator=(const VulkanSwapchain&) = delete;

            VulkanSwapchain(VulkanSwapchain&&) = delete;
            VulkanSwapchain& operator=(VulkanSwapchain&&) = delete;

            void Recreate();

            bool CanRender() const;
            bool NeedsRecreation() const;

            VkSwapchainKHR GetHandle() const { return m_swapchain; }
            VkExtent2D GetExtent() const { return m_extent; }

            VkFormat GetColorFormat() const { return m_colorFormat; }
            VkFormat GetDepthFormat() const { return m_depthFormat; }

            size_t GetImageCount() const { return m_images.size(); }
            VkImage GetImage(uint32_t index) const { return m_images.at(index); }
            VkImageView GetImageView(uint32_t index) const { return m_imageViews.at(index); }

            const VulkanDepthBuffer& GetDepthBuffer(uint32_t index) const;

            VkSemaphore GetRenderFinishedSemaphore(uint32_t imageIndex) const { return m_renderFinishedSemaphores.at(imageIndex); }
        private:
            SDL_Window* m_window = nullptr;

            VkSurfaceKHR m_surface = VK_NULL_HANDLE;
            VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
            VkDevice m_device = VK_NULL_HANDLE;
            VmaAllocator m_allocator = nullptr;

            void create();
            void destroy();

            void createSwapchain();
            void createImageViews();
            void createDepthResources();
            void createRenderFinishedSemaphores();

            bool getWindowPixelSize(int& width, int& height) const;

            VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR& capabilities) const;
            VkSurfaceFormatKHR chooseSurfaceFormat() const;
            VkCompositeAlphaFlagBitsKHR chooseCompositeAlpha(const VkSurfaceCapabilitiesKHR& capabilities) const;
            VkFormat chooseDepthFormat() const;

            VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;

            VkExtent2D m_extent{};
            VkFormat m_colorFormat = VK_FORMAT_UNDEFINED;
            VkFormat m_depthFormat = VK_FORMAT_UNDEFINED;

            std::vector<VkImage> m_images;
            std::vector<VkImageView> m_imageViews;
            std::vector<VulkanDepthBuffer> m_depthBuffers;

            std::vector<VkSemaphore> m_renderFinishedSemaphores;
    };
}
