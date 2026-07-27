#pragma once

#include <volk.h>
#include <vk_mem_alloc.h>

#include <filesystem>
#include <cstdint>
#include <string>
#include <limits>
#include <vector>

namespace deoxy::platform {
    class Window;
};

namespace deoxy::graphics {
    class VulkanContext {
        public:
            VulkanContext(platform::Window& window);
            ~VulkanContext();

            VulkanContext(const VulkanContext&) = delete;
            VulkanContext& operator=(const VulkanContext&) = delete;

            void DrawFrame();
        private:
            // FUNÇÕES
            static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
                VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                VkDebugUtilsMessageTypeFlagsEXT type,
                const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
                void* userData
            );

            void check(bool result, const std::string& errorMessage);
            void check(VkResult result);
            void checkSwapchain(VkResult result);

            bool validationLayersAvailable();

            void createInstance();
            void setupDebugMessenger();
            void createSurface(platform::Window& window);

            void selectPhysicalDevice();
            void findQueueFamilies();
            void createDevice();

            void setupVMA();

            void createSwapchain(platform::Window& window);
            void createSwapchainImageViews();
            void createDepthResources();

            void createCommandPool();
            void createCommandBuffer();
            void createGraphicsPipeline();
            void createSyncObjects();

            void recordCommandBuffer(uint32_t imageIndex);
            void transitionImage(
                VkCommandBuffer commandBuffer,
                VkImage image,
                VkImageLayout oldLayout,
                VkImageLayout newLayout,
                VkPipelineStageFlags2 srcStage,
                VkAccessFlags2 srcAccess,
                VkPipelineStageFlags2 dstStage,
                VkAccessFlags2 dstAccess
            );

            VkShaderModule createShaderModule(const std::vector<uint32_t>& code);
            static std::vector<uint32_t> readShaderFile(const std::filesystem::path& path);

            // VARIÁVEIS
            static constexpr uint32_t FRAMES_IN_FLIGHT = 1;

            #ifndef NDEBUG
                static constexpr bool ENABLE_VALIDATION = true;
            #else
                static constexpr bool ENABLE_VALIDATION = false;
            #endif

            VkInstance m_instance = VK_NULL_HANDLE;
            VkSurfaceKHR m_surface = VK_NULL_HANDLE;

            VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
            VkDevice m_device = VK_NULL_HANDLE;

            VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;

            VkQueue m_queue = VK_NULL_HANDLE;
            uint32_t m_graphicsQueueFamily = std::numeric_limits<std::uint32_t>::max();

            VmaAllocator m_allocator = nullptr;

            VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
            VkExtent2D m_swapchainExtent{};
            VkFormat m_swapchainFormat = VK_FORMAT_UNDEFINED;
            std::vector<VkImage> m_swapchainImages{};
            std::vector<VkImageView> m_swapchainImageViews{};

            VkImage m_depthImage = VK_NULL_HANDLE;
            VkImageView m_depthImageView = VK_NULL_HANDLE;
            VmaAllocation m_depthAllocation = nullptr;
            VkFormat m_depthFormat = VK_FORMAT_UNDEFINED;

            VkCommandPool m_commandPool = VK_NULL_HANDLE;
            VkCommandBuffer m_commandBuffer = VK_NULL_HANDLE;

            VkSemaphore m_imageAvailableSemaphore = VK_NULL_HANDLE;
            std::vector<VkSemaphore> m_renderFinishedSemaphore{};

            VkFence m_inFlightFence;

            VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
            VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;
    };
}
