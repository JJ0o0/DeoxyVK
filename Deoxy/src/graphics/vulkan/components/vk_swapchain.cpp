#include "vk_swapchain.hpp"
#include "vk_allocator.hpp"
#include "vk_device.hpp"
#include "vk_helper.hpp"
#include "vk_surface.hpp"

#include <deoxy/platform/logger.hpp>

#include <SDL3/SDL_video.h>

#include <algorithm>
#include <array>
#include <limits>
#include <vector>

namespace deoxy::graphics::vulkan {
    VulkanSwapchain::VulkanSwapchain(
        SDL_Window* window,
        const VulkanSurface& surface,
        const VulkanDevice& device,
        const VulkanAllocator& allocator
    )
        : m_window(window),
          m_surface(surface.GetHandle()),
          m_physicalDevice(device.GetPhysical()),
          m_device(device.GetLogical()),
          m_allocator(allocator.GetHandle()) {
        // Verificando se recebemos um ponteiro vazio para cada objeto
        CheckBool(window != nullptr, "Swapchain received a null SDL Window");
        CheckBool(m_surface != VK_NULL_HANDLE, "Swapchain received a null surface");
        CheckBool(m_physicalDevice != VK_NULL_HANDLE, "Swapchain received a null physical device");
        CheckBool(m_device != VK_NULL_HANDLE, "Swapchain received a null logical device");
        CheckBool(m_allocator != VK_NULL_HANDLE, "Swapchain received a null allocator");
        CheckBool(CanRender(), "Cannot create swapchain for a zero-sized window");

        try { create(); }
        catch (...) { destroy(); throw; }
    }

    VulkanSwapchain::~VulkanSwapchain() { destroy(); }

    void VulkanSwapchain::Recreate() {
        if (!CanRender()) return;

        CheckResult(vkDeviceWaitIdle(m_device));
        destroy();

        try { create(); }
        catch (...) { destroy(); throw; }

        #ifndef NDEBUG
                platform::Logger::Info(
                    "Recreated swapchain ({}x{})",
                    m_extent.width,
                    m_extent.height
                );
        #endif
    }

    bool VulkanSwapchain::CanRender() const {
        int width = 0;
        int height = 0;

        return getWindowPixelSize(width, height);
    }

    bool VulkanSwapchain::NeedsRecreation() const {
        if (!CanRender()) return false;

        VkSurfaceCapabilitiesKHR capabilities{};
        CheckResult(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, m_surface, &capabilities));

        const VkExtent2D desiredExtent = chooseExtent(capabilities);

        return desiredExtent.width != m_extent.width ||
               desiredExtent.height != m_extent.height;
    }

    void VulkanSwapchain::create() {
        createSwapchain();
        createImageViews();
        createDepthResources();
        createRenderFinishedSemaphores();
    }

    void VulkanSwapchain::destroy() {
        if (!m_renderFinishedSemaphores.empty()) {
            for (VkSemaphore s : m_renderFinishedSemaphores) {
                if (s != VK_NULL_HANDLE) vkDestroySemaphore(m_device, s, nullptr);
            }

            m_renderFinishedSemaphores.clear();
        }

        if (m_depthImageView != VK_NULL_HANDLE) vkDestroyImageView(m_device, m_depthImageView, nullptr);
        if (m_depthImage != VK_NULL_HANDLE) vmaDestroyImage(m_allocator, m_depthImage, m_depthAllocation);

        if (!m_imageViews.empty()) {
            for (VkImageView iv : m_imageViews) {
                if (iv != VK_NULL_HANDLE) vkDestroyImageView(m_device, iv, nullptr);
            }

            m_imageViews.clear();
        }

        if (m_swapchain != VK_NULL_HANDLE) vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
    }

    void VulkanSwapchain::createSwapchain() {
        // Pegando as capabilities da surface
        VkSurfaceCapabilitiesKHR surfaceCaps{};
        CheckResult(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, m_surface, &surfaceCaps));

        // Pegando o extent correto do swapchain (Wayland tem um valor especial pra isso)
        // E pegando o format
        const VkSurfaceFormatKHR surfaceFormat = chooseSurfaceFormat();
        m_colorFormat = surfaceFormat.format;
        m_extent = chooseExtent(surfaceCaps);

        uint32_t imageCount = surfaceCaps.minImageCount + 1;
        if (surfaceCaps.maxImageCount > 0 && imageCount > surfaceCaps.maxImageCount) {
            imageCount = surfaceCaps.maxImageCount;
        }

        // Criando a Swapchain
        VkSwapchainCreateInfoKHR swapchainCI {
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface = m_surface,
            .minImageCount = imageCount,
            .imageFormat = surfaceFormat.format,
            .imageColorSpace = surfaceFormat.colorSpace,
            .imageExtent = m_extent,
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices = nullptr,
            .preTransform = surfaceCaps.currentTransform,
            .compositeAlpha = chooseCompositeAlpha(surfaceCaps),
            .presentMode = VK_PRESENT_MODE_FIFO_KHR, // VSYNC
            .clipped = VK_TRUE,
            .oldSwapchain = VK_NULL_HANDLE
        };

        CheckResult(vkCreateSwapchainKHR(m_device, &swapchainCI, nullptr, &m_swapchain));

        // Pegando as imagens da swapchain
        uint32_t actualImageCount { 0 };
        CheckResult(vkGetSwapchainImagesKHR(m_device, m_swapchain, &actualImageCount, nullptr));

        m_images.resize(actualImageCount);
        CheckResult(vkGetSwapchainImagesKHR(m_device, m_swapchain, &actualImageCount, m_images.data()));
    }

    void VulkanSwapchain::createImageViews() {
        // Preenche a lista de image views com null handles
        m_imageViews.assign(m_images.size(), VK_NULL_HANDLE);

        // Loop para criar uma image view pra cada image
        for (size_t i = 0; i < m_images.size(); i++) {
            VkImageViewCreateInfo viewCI {
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .image = m_images[i],
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = m_colorFormat,
                .components {
                    .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .a = VK_COMPONENT_SWIZZLE_IDENTITY
                },
                .subresourceRange {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1
                }
            };

            CheckResult(vkCreateImageView(m_device, &viewCI, nullptr, &m_imageViews[i]));
        }
    }

    void VulkanSwapchain::createDepthResources() {
        // Escolhendo o formato disponível
        m_depthFormat = chooseDepthFormat();

        // Alocar e criar a imagem de depth
        VkImageCreateInfo depthImageCI {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = m_depthFormat,
            .extent {
                .width = m_extent.width, .height = m_extent.height,
                .depth = 1
            },
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
        };

        VmaAllocationCreateInfo allocCI {
            .flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO
        };

        CheckResult(vmaCreateImage(m_allocator, &depthImageCI, &allocCI, &m_depthImage, &m_depthAllocation, nullptr));

        // Criando image view do depth
        VkImageViewCreateInfo depthViewCI {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = m_depthImage,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = m_depthFormat,
            .subresourceRange {
                .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };

        CheckResult(vkCreateImageView(m_device, &depthViewCI, nullptr, &m_depthImageView));
    }

    void VulkanSwapchain::createRenderFinishedSemaphores() {
        VkSemaphoreCreateInfo semaphoreCI {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
        };

        m_renderFinishedSemaphores.assign(m_images.size(), VK_NULL_HANDLE);
        for (VkSemaphore& semaphore : m_renderFinishedSemaphores) {
            CheckResult(vkCreateSemaphore(m_device, &semaphoreCI, nullptr, &semaphore));
        }
    }

    bool VulkanSwapchain::getWindowPixelSize(int& width, int& height) const {
        CheckBool(SDL_GetWindowSizeInPixels(
            m_window,
            &width, &height
        ), std::format("Couldn't get window pixel size: {}", SDL_GetError()));

        return width > 0 && height > 0;
    }

    VkExtent2D VulkanSwapchain::chooseExtent(const VkSurfaceCapabilitiesKHR& capabilities) const {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        }

        int width = 0;
        int height = 0;

        CheckBool(getWindowPixelSize(
            width, height
        ), "Cannot create swapchain with a zero-sized window");

        return {
            .width = std::clamp(
                static_cast<uint32_t>(width),
                capabilities.minImageExtent.width,
                capabilities.maxImageExtent.width
            ),
            .height = std::clamp(
                static_cast<uint32_t>(height),
                capabilities.minImageExtent.height,
                capabilities.maxImageExtent.height
            )
        };
    }

    VkSurfaceFormatKHR VulkanSwapchain::chooseSurfaceFormat() const {
        uint32_t formatCount = 0;
        CheckResult(vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &formatCount, nullptr));
        CheckBool(formatCount > 0, "Surface has no supported formats");

        std::vector<VkSurfaceFormatKHR> formats(formatCount);
        CheckResult(vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &formatCount, formats.data()));

        for (const VkSurfaceFormatKHR& format : formats) {
            if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR) {
                return format;
            }
        }

        return formats.front();
    }

    VkCompositeAlphaFlagBitsKHR VulkanSwapchain::chooseCompositeAlpha(const VkSurfaceCapabilitiesKHR& capabilities) const {
        static constexpr std::array<VkCompositeAlphaFlagBitsKHR, 4> candidates{
            VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
            VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
            VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR
        };

        for (VkCompositeAlphaFlagBitsKHR candidate : candidates) {
            if (capabilities.supportedCompositeAlpha & candidate) return candidate;
        }

        CheckBool(false, "Surface has no supported composite alpha mode");
        return VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    }

    VkFormat VulkanSwapchain::chooseDepthFormat() const {
        static constexpr std::array<VkFormat, 2> candidates{
            VK_FORMAT_D32_SFLOAT_S8_UINT,
            VK_FORMAT_D24_UNORM_S8_UINT
        };

        for (VkFormat format : candidates) {
            VkFormatProperties2 props {
                .sType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2
            };

            vkGetPhysicalDeviceFormatProperties2(m_physicalDevice, format, &props);

            if (props.formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
                return format;
            }
        }

        CheckBool(false, "No supported depth format found");
        return VK_FORMAT_UNDEFINED;
    }
};
