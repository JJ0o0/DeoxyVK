#include "vk_instance.hpp"
#include "vk_helper.hpp"

#include <deoxy/platform/logger.hpp>

#include <SDL3/SDL_vulkan.h>
#include <string_view>
#include <vector>

namespace deoxy::graphics::vulkan {
    VulkanInstance::VulkanInstance() {
        try {
            CheckResult(volkInitialize());

            createInstance();
            volkLoadInstance(m_instance);

            createDebugMessenger();
        } catch (...) { destroy(); throw; }
    }

    VulkanInstance::~VulkanInstance() {
        destroy();
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL VulkanInstance::debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT severity,
        VkDebugUtilsMessageTypeFlagsEXT type,
        const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
        void* userData
    ) {
        if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
            platform::Logger::Error("Vulkan Validation: {}", callbackData->pMessage);
        } else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
            platform::Logger::Warn("Vulkan Validation: {}", callbackData->pMessage);
        } else {
            platform::Logger::Info("Vulkan Validation: {}", callbackData->pMessage);
        }

        return VK_FALSE;
    }

    VkDebugUtilsMessengerCreateInfoEXT
    VulkanInstance::getDebugMessengerCreateInfo() {
        return {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
            .pfnUserCallback = debugCallback
        };
    }

    void VulkanInstance::destroy() {
        if (m_debugMessenger != VK_NULL_HANDLE) vkDestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
        if (m_instance != VK_NULL_HANDLE) vkDestroyInstance(m_instance, nullptr);

        m_debugMessenger = VK_NULL_HANDLE;
        m_instance = VK_NULL_HANDLE;
    }

    void VulkanInstance::createInstance() {
        // Informações da Aplicação
        VkApplicationInfo applicationInfo {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName = "DeoxyVK",
            .apiVersion = VK_API_VERSION_1_3
        };

        // Pegando extensões do sistema operacional
        uint32_t extensionsCount { 0 };
        char const* const* sdlExtensions { SDL_Vulkan_GetInstanceExtensions(&extensionsCount) };
        CheckBool(sdlExtensions != nullptr, "Couldn't get SDL Vulkan extensions");

        // Pegando a extensão da validation layer
        std::vector<const char*> extensions(sdlExtensions, sdlExtensions + extensionsCount);
        const char* validationLayer = VALIDATION_LAYER;

        if (ENABLE_VALIDATION) {
            CheckBool(validationLayersAvailable(), "VK_LAYER_KHRONOS_validation is not available");
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        // Criando instância
        const auto messengerCI = getDebugMessengerCreateInfo();
        VkInstanceCreateInfo instanceCI {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext = ENABLE_VALIDATION ? &messengerCI : nullptr,
            .pApplicationInfo = &applicationInfo,
            .enabledLayerCount = ENABLE_VALIDATION ? 1u : 0u,
            .ppEnabledLayerNames = ENABLE_VALIDATION ? &validationLayer : nullptr,
            .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
            .ppEnabledExtensionNames = extensions.data()
        };

        CheckResult(vkCreateInstance(&instanceCI, nullptr, &m_instance));
    }

    void VulkanInstance::createDebugMessenger() {
        if (!ENABLE_VALIDATION) return;

        // Configurando o messenger e criando
        const auto messengerCI = getDebugMessengerCreateInfo();
        CheckResult(vkCreateDebugUtilsMessengerEXT(m_instance, &messengerCI, nullptr, &m_debugMessenger));
    }

    bool VulkanInstance::validationLayersAvailable() {
        // Pega as layers
        uint32_t layerCount = 0;
        CheckResult(vkEnumerateInstanceLayerProperties(&layerCount, nullptr));

        std::vector<VkLayerProperties> layers(layerCount);
        CheckResult(vkEnumerateInstanceLayerProperties(&layerCount, layers.data()));

        // Pegando a layer com a validation
        for (const VkLayerProperties& layer : layers) {
            if (std::string_view{layer.layerName} == VALIDATION_LAYER) return true;
        }

        return false;
    }
}
