#pragma once

#include <volk.h>

namespace deoxy::graphics::vulkan {
    class VulkanInstance {
        public:
            VulkanInstance();
            ~VulkanInstance();

            VulkanInstance(const VulkanInstance&) = delete;
            VulkanInstance& operator=(const VulkanInstance&) = delete;

            VkInstance GetHandle() const { return m_instance; }
        private:
            static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
                VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                VkDebugUtilsMessageTypeFlagsEXT type,
                const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
                void* userData
            );

            VkDebugUtilsMessengerCreateInfoEXT getDebugMessengerCreateInfo();

            void destroy();
            void createInstance();
            void createDebugMessenger();
            bool validationLayersAvailable();

            #ifndef NDEBUG
                static constexpr bool ENABLE_VALIDATION = true;
            #else
                static constexpr bool ENABLE_VALIDATION = false;
            #endif

            static constexpr const char* VALIDATION_LAYER = "VK_LAYER_KHRONOS_validation";

            VkInstance m_instance = VK_NULL_HANDLE;
            VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
    };
}
