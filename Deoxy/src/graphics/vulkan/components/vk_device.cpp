#include "vk_device.hpp"
#include "vk_helper.hpp"

#include <deoxy/platform/logger.hpp>

#include <SDL3/SDL_vulkan.h>
#include <vector>

namespace deoxy::graphics::vulkan {
    VulkanDevice::VulkanDevice(VkInstance instance, VkSurfaceKHR surface)
        : m_instance(instance), m_surface(surface) {
        try {
            selectPhysicalDevice();
            findQueueFamilies();
            createLogicalDevice();

            volkLoadDevice(m_device);
        } catch (...) { destroy(); throw; }
    }

    VulkanDevice::~VulkanDevice() { destroy(); }

    void VulkanDevice::destroy() {
        if (m_device != VK_NULL_HANDLE) vkDestroyDevice(m_device, nullptr);
    }

    void VulkanDevice::selectPhysicalDevice() {
        // Listar todos os dispositivos físicos que suportam Vulkan
        uint32_t deviceCount { 0 };
        CheckResult(vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr));
        CheckBool(deviceCount > 0, "No Vulkan-compatible GPU found");

        // Pegar os dispositivos de verdade
        std::vector<VkPhysicalDevice> devices(deviceCount);
        CheckResult(vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data()));

        // Fallback pra integrada ou pra primeira
        m_physicalDevice = devices[0];
        for (VkPhysicalDevice device : devices) {
            // Pegando as propriedades do dispositivo
            VkPhysicalDeviceProperties props{};
            vkGetPhysicalDeviceProperties(device, &props);

            // Escolhe a discreta
            if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                m_physicalDevice = device;
                break;
            }
        }

        // Pegando as informações e mostrando
        VkPhysicalDeviceProperties2 deviceProperties {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2
        };

        vkGetPhysicalDeviceProperties2(m_physicalDevice, &deviceProperties);
        platform::Logger::Info("GPU: {}", deviceProperties.properties.deviceName);
    }

    void VulkanDevice::findQueueFamilies() {
        // Pegando a quantidade de queue family
        uint32_t queueFamilyCount { 0 };
        vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, nullptr);

        // Pegando as queue families de verdade
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, queueFamilies.data());

        // Determinando as queue families
        bool found = false;
        for (size_t i = 0; i < queueFamilies.size(); i++) {
            const bool supportsGraphics = queueFamilies[i].queueCount > 0 && (
                queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT
            );

            VkBool32 supportsPresentation = VK_FALSE;
            CheckResult(vkGetPhysicalDeviceSurfaceSupportKHR(
                m_physicalDevice,
                static_cast<uint32_t>(i),
                m_surface,
                &supportsPresentation
            ));

            if (supportsGraphics && supportsPresentation) {
                m_graphicsQueueFamily = static_cast<uint32_t>(i);
                found = true;
                break;
            }
        }

        CheckBool(found, "No graphics queue with presentation support found");
    }

    void VulkanDevice::createLogicalDevice() {
        // Referenciar a queue family
        const float qfpriorities { 1.0f };
        VkDeviceQueueCreateInfo queueCI {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = m_graphicsQueueFamily,
            .queueCount = 1,
            .pQueuePriorities = &qfpriorities
        };

        // Determinando extensões necessárias pra aplicação (Vulkan 1.3 já dá quase tudo
        // porém, ainda precisamos de VK_KHR_swapchain pra ter algo na tela)
        const std::vector<const char*> deviceExtensions { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

        // Recursos do Vulkan
        VkPhysicalDeviceVulkan12Features features12 {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
            .descriptorIndexing = VK_TRUE,
            .shaderSampledImageArrayNonUniformIndexing = VK_TRUE,
            .descriptorBindingVariableDescriptorCount = VK_TRUE,
            .runtimeDescriptorArray = VK_TRUE,
            .bufferDeviceAddress = VK_TRUE
        };

        VkPhysicalDeviceVulkan13Features features13 {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
            .pNext = &features12,
            .synchronization2 = VK_TRUE,
            .dynamicRendering = VK_TRUE
        };

        VkPhysicalDeviceFeatures features10 {
            .samplerAnisotropy = VK_TRUE
        };

        // Criando o Device
        VkDeviceCreateInfo deviceCI {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = &features13,
            .queueCreateInfoCount = 1,
            .pQueueCreateInfos = &queueCI,
            .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
            .ppEnabledExtensionNames = deviceExtensions.data(),
            .pEnabledFeatures = &features10
        };

        CheckResult(vkCreateDevice(m_physicalDevice, &deviceCI, nullptr, &m_device));

        // Pegar o Queue do logical device e salvar
        vkGetDeviceQueue(m_device, m_graphicsQueueFamily, 0, &m_queue);
    }
}
