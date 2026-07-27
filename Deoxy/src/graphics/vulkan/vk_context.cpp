#include "vk_context.hpp"

#include <deoxy/platform/window.hpp>
#include <deoxy/platform/logger.hpp>
#include <deoxy/graphics/vertex.hpp>

#include <SDL3/SDL_vulkan.h>

#include <string_view>
#include <algorithm>
#include <stdexcept>
#include <iterator>
#include <fstream>
#include <cstddef>
#include <limits>
#include <format>
#include <vector>
#include <array>

namespace deoxy::graphics {
    VulkanContext::VulkanContext(platform::Window& window) : m_window(&window) {
        check(volkInitialize());

        createInstance();
        volkLoadInstance(m_instance);

        setupDebugMessenger();

        createSurface(window);
        selectPhysicalDevice();
        findQueueFamilies();
        createDevice();

        volkLoadDevice(m_device);

        createCommandPool();
        createCommandBuffers();

        setupVMA();
        createSwapchain(window);
        createSwapchainImageViews();
        createDepthResources();
        createGraphicsPipeline();
        createGeometryBuffers();
        createSyncObjects();
    }

    VulkanContext::~VulkanContext() {
        if (m_device != VK_NULL_HANDLE) vkDeviceWaitIdle(m_device);

        if (!m_frames.empty()) {
            for (FrameData& f : m_frames) {
                if (f.ImageAvailableSemaphore != VK_NULL_HANDLE) vkDestroySemaphore(m_device, f.ImageAvailableSemaphore, nullptr);
                if (f.InFlightFence != VK_NULL_HANDLE) vkDestroyFence(m_device, f.InFlightFence, nullptr);
            }
        }

        if (m_indexBuffer != VK_NULL_HANDLE) vmaDestroyBuffer(m_allocator, m_indexBuffer, m_indexAllocation);
        if (m_vertexBuffer != VK_NULL_HANDLE) vmaDestroyBuffer(m_allocator, m_vertexBuffer, m_vertexAllocation);

        if (m_graphicsPipeline != VK_NULL_HANDLE) vkDestroyPipeline(m_device, m_graphicsPipeline, nullptr);
        if (m_pipelineLayout != VK_NULL_HANDLE) vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);

        cleanupSwapchain();

        if (m_commandPool != VK_NULL_HANDLE) vkDestroyCommandPool(m_device, m_commandPool, nullptr);

        if (m_allocator != nullptr) vmaDestroyAllocator(m_allocator);
        if (m_device != VK_NULL_HANDLE) vkDestroyDevice(m_device, nullptr);
        if (m_surface != VK_NULL_HANDLE) vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
        if (m_debugMessenger != VK_NULL_HANDLE) vkDestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
        if (m_instance != VK_NULL_HANDLE) vkDestroyInstance(m_instance, nullptr);
    }

    void VulkanContext::DrawFrame() {
        // Detectando o resize para a reconstrução da swapchain
        int windowWidth = 0;
        int windowHeight = 0;

        if (!getWindowPixelSize(windowWidth, windowHeight)) return;

        VkSurfaceCapabilitiesKHR surfaceCaps{};
        check(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, m_surface, &surfaceCaps));

        const VkExtent2D desiredExtent = chooseSwapchainExtent(surfaceCaps);
        const bool extentChanged = desiredExtent.width != m_swapchainExtent.width || desiredExtent.height != m_swapchainExtent.height;

        if (extentChanged) {
            recreateSwapchain();
            return;
        }

        FrameData& frame = m_frames[m_currentFrame];
        // Espera a GPU terminar de usar os recursos do frame
        check(vkWaitForFences(m_device, 1, &frame.InFlightFence, VK_TRUE, UINT64_MAX));

        uint32_t imageIndex = 0;

        // Pede uma imagem disponível da swapchain
        VkResult acquireResult = vkAcquireNextImageKHR(
            m_device, m_swapchain,
            UINT64_MAX,
            frame.ImageAvailableSemaphore,
            VK_NULL_HANDLE,
            &imageIndex
        );

        if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapchain();
            return;
        }

        if (acquireResult != VK_SUCCESS && acquireResult != VK_SUBOPTIMAL_KHR) {
            check(acquireResult);
        }

        const bool swapchainSuboptimal = acquireResult == VK_SUBOPTIMAL_KHR;

        // Só reseta depois que sabemos que vamos realmente enviar um frame
        check(vkResetFences(m_device, 1, &frame.InFlightFence));
        check(vkResetCommandBuffer(frame.CommandBuffer, 0));

        // Gravando o command buffer
        recordCommandBuffer(frame.CommandBuffer, imageIndex);

        // Enviando para a GPU
        VkSemaphoreSubmitInfo waitSemaphoreInfo {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .semaphore = frame.ImageAvailableSemaphore,
            .value = 0,
            .stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            .deviceIndex = 0
        };

        VkCommandBufferSubmitInfo commandBufferInfo {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
            .commandBuffer = frame.CommandBuffer,
            .deviceMask = 1
        };

        VkSemaphore renderFinishedSemaphore = m_renderFinishedSemaphore[imageIndex];
        VkSemaphoreSubmitInfo signalSemaphoreInfo {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .semaphore = renderFinishedSemaphore,
            .value = 0,
            .stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
            .deviceIndex = 0
        };

        VkSubmitInfo2 submitInfo {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
            .waitSemaphoreInfoCount = 1,
            .pWaitSemaphoreInfos = &waitSemaphoreInfo,
            .commandBufferInfoCount = 1,
            .pCommandBufferInfos = &commandBufferInfo,
            .signalSemaphoreInfoCount = 1,
            .pSignalSemaphoreInfos = &signalSemaphoreInfo
        };

        check(vkQueueSubmit2(m_queue, 1, &submitInfo, frame.InFlightFence));

        // Presentar a imagem
        VkPresentInfoKHR presentInfo {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &renderFinishedSemaphore,
            .swapchainCount = 1,
            .pSwapchains = &m_swapchain,
            .pImageIndices = &imageIndex
        };

        VkResult presentResult = vkQueuePresentKHR(m_queue, &presentInfo);

        const bool mustRecreate = presentResult == VK_ERROR_OUT_OF_DATE_KHR ||
                                  presentResult == VK_SUBOPTIMAL_KHR ||
                                  swapchainSuboptimal;

        if (presentResult != VK_SUCCESS &&
            presentResult != VK_SUBOPTIMAL_KHR &&
            presentResult != VK_ERROR_OUT_OF_DATE_KHR
        ) check(presentResult);

        m_currentFrame = (m_currentFrame + 1) % FRAMES_IN_FLIGHT;

        if (mustRecreate) recreateSwapchain();
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL VulkanContext::debugCallback(
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

    void VulkanContext::check(bool result, const std::string& errorMessage) {
        if (!result) {
            std::string msg = std::format("Vulkan call returned an error ({})", errorMessage);
            throw std::runtime_error{msg};
        }
    }

    void VulkanContext::check(VkResult result) {
        if (result != VK_SUCCESS) {
            std::string msg = std::format("Vulkan call returned an error ({})", static_cast<int>(result));
            throw std::runtime_error{msg};
        }
    }

    bool VulkanContext::validationLayersAvailable() {
        // Pega as layers
        uint32_t layerCount = 0;
        check(vkEnumerateInstanceLayerProperties(&layerCount, nullptr));

        std::vector<VkLayerProperties> layers(layerCount);
        check(vkEnumerateInstanceLayerProperties(&layerCount, layers.data()));

        // Pegando a layer com a validation
        for (const VkLayerProperties& layer : layers) {
            if (std::string_view{layer.layerName} == "VK_LAYER_KHRONOS_validation") return true;
        }

        return false;
    }

    void VulkanContext::createInstance() {
        // Informações da Aplicação
        VkApplicationInfo applicationInfo {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName = "DeoxyVK",
            .apiVersion = VK_API_VERSION_1_3
        };

        // Pegando extensões do sistema operacional
        uint32_t extensionsCount { 0 };
        char const* const* sdlExtensions { SDL_Vulkan_GetInstanceExtensions(&extensionsCount) };
        check(sdlExtensions != nullptr, "Couldn't get SDL Vulkan extensions");

        // Pegando a extensão da validation layer
        std::vector<const char*> extensions(sdlExtensions, sdlExtensions + extensionsCount);
        const char* validationLayer = "VK_LAYER_KHRONOS_validation";

        if (ENABLE_VALIDATION) {
            check(validationLayersAvailable(), "VK_LAYER_KHRONOS_validation is not available");
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        // Criando instância
        VkInstanceCreateInfo instanceCI {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pApplicationInfo = &applicationInfo,
            .enabledLayerCount = ENABLE_VALIDATION ? 1u : 0u,
            .ppEnabledLayerNames = ENABLE_VALIDATION ? &validationLayer : nullptr,
            .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
            .ppEnabledExtensionNames = extensions.data()
        };

        check(vkCreateInstance(&instanceCI, nullptr, &m_instance));
    }

    void VulkanContext::setupDebugMessenger() {
        if (!ENABLE_VALIDATION) return;

        // Configurando o messenger e criando
        VkDebugUtilsMessengerCreateInfoEXT messengerCI {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
            .pfnUserCallback = debugCallback
        };

        check(vkCreateDebugUtilsMessengerEXT(m_instance, &messengerCI, nullptr, &m_debugMessenger));
    }

    void VulkanContext::createSurface(platform::Window& window) {
        // Criando a surface via SDL
        check(SDL_Vulkan_CreateSurface(
            window.GetHandle(), m_instance,
            nullptr,
            &m_surface
        ), "Could not create SDL Vulkan Surface");
    }

    void VulkanContext::selectPhysicalDevice() {
        // Listar todos os dispositivos físicos que suportam Vulkan
        uint32_t deviceCount { 0 };
        check(vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr));
        check(deviceCount > 0, "No Vulkan-compatible GPU found");

        // Pegar os dispositivos de verdade
        std::vector<VkPhysicalDevice> devices(deviceCount);
        check(vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data()));

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

    void VulkanContext::findQueueFamilies() {
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

            const bool supportsPresentation = SDL_Vulkan_GetPresentationSupport(m_instance, m_physicalDevice, i);

            if (supportsGraphics && supportsPresentation) {
                m_graphicsQueueFamily = i;
                found = true;
                break;
            }
        }

        check(found, "No graphics queue with presentation support found");
    }

    void VulkanContext::createDevice() {
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

        check(vkCreateDevice(m_physicalDevice, &deviceCI, nullptr, &m_device));

        // Pegar o Queue do logical device e salvar
        vkGetDeviceQueue(m_device, m_graphicsQueueFamily, 0, &m_queue);
    }

    void VulkanContext::setupVMA() {
        // Criando o alocador de memória
        VmaAllocatorCreateInfo allocatorCI {
            .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
            .physicalDevice = m_physicalDevice,
            .device = m_device,
            .instance = m_instance,
            .vulkanApiVersion = VK_API_VERSION_1_3
        };

        // Pegando funções do VMA usando o Volk
        VmaVulkanFunctions vkFunctions{};
        check(vmaImportVulkanFunctionsFromVolk(&allocatorCI, &vkFunctions));

        allocatorCI.pVulkanFunctions = &vkFunctions;
        check(vmaCreateAllocator(&allocatorCI, &m_allocator));
    }

    void VulkanContext::createSwapchain(platform::Window& window) {
        // Pegando as capabilities da surface
        VkSurfaceCapabilitiesKHR surfaceCaps{};
        check(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, m_surface, &surfaceCaps));

        // Pegando o extent correto do swapchain (Wayland tem um valor especial pra isso)
        m_swapchainExtent = chooseSwapchainExtent(surfaceCaps);

        // Criando a Swapchain
        m_swapchainFormat = VK_FORMAT_B8G8R8A8_SRGB;
        VkSwapchainCreateInfoKHR swapchainCI {
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface = m_surface,
            .minImageCount = surfaceCaps.minImageCount,
            .imageFormat = m_swapchainFormat,
            .imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR,
            .imageExtent = m_swapchainExtent,
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .preTransform = surfaceCaps.currentTransform,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = VK_PRESENT_MODE_FIFO_KHR // VSYNC
        };

        check(vkCreateSwapchainKHR(m_device, &swapchainCI, nullptr, &m_swapchain));

        // Pegando as imagens da swapchain
        uint32_t imageCount { 0 };
        check(vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, nullptr));

        m_swapchainImages.resize(imageCount);
        check(vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, m_swapchainImages.data()));
    }

    void VulkanContext::createSwapchainImageViews() {
        // Ajusta o tamanho da lista de image views para ser o mesmo da lista de images
        m_swapchainImageViews.resize(m_swapchainImages.size());

        // Loop para criar uma image view pra cada image
        for (size_t i = 0; i < m_swapchainImages.size(); i++) {
            VkImageViewCreateInfo viewCI {
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .image = m_swapchainImages[i],
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = m_swapchainFormat,
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

            check(vkCreateImageView(m_device, &viewCI, nullptr, &m_swapchainImageViews[i]));
        }
    }

    void VulkanContext::recreateSwapchain() {
        int width = 0;
        int height = 0;

        if (!getWindowPixelSize(width, height)) return;

        check(vkDeviceWaitIdle(m_device));

        cleanupSwapchain();

        createSwapchain(*m_window);
        createSwapchainImageViews();
        createDepthResources();
        createRenderFinishedSemaphores();

        #ifndef NDEBUG
            platform::Logger::Info("Recreated swapchain ({}x{})", m_swapchainExtent.width, m_swapchainExtent.height);
        #endif
    }

    void VulkanContext::cleanupSwapchain() {
        if (!m_renderFinishedSemaphore.empty()) {
            for (VkSemaphore s : m_renderFinishedSemaphore) {
                if (s != VK_NULL_HANDLE) vkDestroySemaphore(m_device, s, nullptr);
            }

            m_renderFinishedSemaphore.clear();
        }

        if (m_depthImageView != VK_NULL_HANDLE) vkDestroyImageView(m_device, m_depthImageView, nullptr);
        if (m_depthImage != VK_NULL_HANDLE) vmaDestroyImage(m_allocator, m_depthImage, m_depthAllocation);

        if (!m_swapchainImageViews.empty()) {
            for (VkImageView iv : m_swapchainImageViews) {
                if (iv != VK_NULL_HANDLE) vkDestroyImageView(m_device, iv, nullptr);
            }

            m_swapchainImageViews.clear();
        }

        if (m_swapchain != VK_NULL_HANDLE) vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
    }

    // Pegar o extent ideal para o nosso swapchain atual
    VkExtent2D VulkanContext::chooseSwapchainExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        }

        int width = 0;
        int height = 0;

        getWindowPixelSize(width, height);

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

    void VulkanContext::createDepthResources() {
        // Verificando o formato disponível
        std::vector<VkFormat> depthFormatList { VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };
        for (VkFormat& format : depthFormatList) {
            VkFormatProperties2 formatProps {
                .sType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2
            };

            vkGetPhysicalDeviceFormatProperties2(m_physicalDevice, format, &formatProps);

            if (formatProps.formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
                m_depthFormat = format;
                break;
            }
        }

        // Alocar e criar a imagem de depth
        VkImageCreateInfo depthImageCI {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = m_depthFormat,
            .extent {
                .width = m_swapchainExtent.width, .height = m_swapchainExtent.height,
                .depth = 1
            },
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
        };

        VmaAllocationCreateInfo allocCI {
            .flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO
        };

        check(vmaCreateImage(m_allocator, &depthImageCI, &allocCI, &m_depthImage, &m_depthAllocation, nullptr));

        // Criando image view do depth
        VkImageViewCreateInfo depthViewCI {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = m_depthImage,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = m_depthFormat,
            .subresourceRange {
                .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
                .levelCount = 1, .layerCount = 1
            }
        };

        check(vkCreateImageView(m_device, &depthViewCI, nullptr, &m_depthImageView));
    }

    void VulkanContext::createCommandPool() {
        // Criando a Command Pool usando a graphics queue family
        VkCommandPoolCreateInfo poolCI {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = m_graphicsQueueFamily
        };

        check(vkCreateCommandPool(m_device, &poolCI, nullptr, &m_commandPool));
    }

    void VulkanContext::createCommandBuffers() {
        // Criando os Command Buffers
        std::array<VkCommandBuffer, FRAMES_IN_FLIGHT> commandBuffers{};

        VkCommandBufferAllocateInfo allocInfo {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = m_commandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = static_cast<uint32_t>(commandBuffers.size())
        };

        check(vkAllocateCommandBuffers(m_device, &allocInfo, commandBuffers.data()));

        for (size_t i = 0; i < m_frames.size(); i++) m_frames[i].CommandBuffer = commandBuffers[i];
    }

    void VulkanContext::createGraphicsPipeline() {
        // Lendo e criando os shaders
        const auto vertexCode = readShaderFile("shaders/basic.vert.spv");
        const auto fragmentCode = readShaderFile("shaders/basic.frag.spv");

        VkShaderModule vertexModule = createShaderModule(vertexCode);
        VkShaderModule fragmentModule = createShaderModule(fragmentCode);

        // Criando os shaders na pipeline
        VkPipelineShaderStageCreateInfo vertexStage {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = vertexModule,
            .pName = "main"
        };

        VkPipelineShaderStageCreateInfo fragmentStage {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = fragmentModule,
            .pName = "main"
        };

        VkPipelineShaderStageCreateInfo shaderStages[] { vertexStage, fragmentStage };

        // Definindo como o Vulkan avança pelos elementos do vertex buffer
        VkVertexInputBindingDescription bindingDescription {
            .binding = 0,
            .stride = sizeof(Vertex),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
        };

        // Define o que cada parte de Vertex é no shader
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions {
            VkVertexInputAttributeDescription {
                .location = 0,
                .binding = 0,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = offsetof(Vertex, Position)
            },
            VkVertexInputAttributeDescription {
                .location = 1,
                .binding = 0,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = offsetof(Vertex, Color)
            },
        };

        // Define como andar pelo buffer e como interpretar cada atributo
        VkPipelineVertexInputStateCreateInfo vertexInputCI {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .vertexBindingDescriptionCount = 1,
            .pVertexBindingDescriptions = &bindingDescription,
            .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
            .pVertexAttributeDescriptions = attributeDescriptions.data()
        };

        // Define como três vértices forma um triângulo
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyCI {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            .primitiveRestartEnable = VK_FALSE
        };

        // Criando o viewport
        VkPipelineViewportStateCreateInfo viewportCI {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .viewportCount = 1,
            .pViewports = nullptr,
            .scissorCount = 1,
            .pScissors = nullptr
        };

        // Definindo estados dinâmicos
        const std::array<VkDynamicState, 2> dynamicStates { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
        VkPipelineDynamicStateCreateInfo dynamicStateCI {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
            .pDynamicStates = dynamicStates.data()
        };

        // Definindo informações sobre a rasterização
        VkPipelineRasterizationStateCreateInfo rasterizationCI {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .depthClampEnable = VK_FALSE,
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode = VK_POLYGON_MODE_FILL,
            .cullMode = VK_CULL_MODE_NONE,
            .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
            .depthBiasEnable = VK_FALSE,
            .lineWidth = 1.0f
        };

        // Definindo configurações do MSAA
        VkPipelineMultisampleStateCreateInfo multisampleCI {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            .sampleShadingEnable = VK_FALSE,
            .minSampleShading = 1.0f,
            .pSampleMask = nullptr,
            .alphaToCoverageEnable = VK_FALSE,
            .alphaToOneEnable = VK_FALSE
        };

        // Definindo configurações do Depth Test
        VkPipelineDepthStencilStateCreateInfo depthStencilCI {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            .depthTestEnable = VK_FALSE,
            .depthWriteEnable = VK_FALSE,
            .depthCompareOp = VK_COMPARE_OP_LESS,
            .depthBoundsTestEnable = VK_FALSE,
            .stencilTestEnable = VK_FALSE
        };

        // Configurando color blending
        VkPipelineColorBlendAttachmentState colorBlendAttachment {
            .blendEnable = VK_FALSE,
            .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
            .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
            .colorBlendOp = VK_BLEND_OP_ADD,
            .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
            .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
            .alphaBlendOp = VK_BLEND_OP_ADD,
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                              VK_COLOR_COMPONENT_G_BIT |
                              VK_COLOR_COMPONENT_B_BIT |
                              VK_COLOR_COMPONENT_A_BIT
        };

        VkPipelineColorBlendStateCreateInfo colorBlendCI {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .logicOpEnable = VK_FALSE,
            .logicOp = VK_LOGIC_OP_COPY,
            .attachmentCount = 1,
            .pAttachments = &colorBlendAttachment
        };

        // Criando o layout da pipeline
        VkPipelineLayoutCreateInfo layoutCI {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = 0,
            .pSetLayouts = nullptr,
            .pushConstantRangeCount = 0,
            .pPushConstantRanges = nullptr
        };

        check(vkCreatePipelineLayout(m_device, &layoutCI, nullptr, &m_pipelineLayout));

        // Informando o formato usado pelo dynamic rendering
        VkPipelineRenderingCreateInfo renderingCI {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
            .viewMask = 0,
            .colorAttachmentCount = 1,
            .pColorAttachmentFormats = &m_swapchainFormat,
            .depthAttachmentFormat = VK_FORMAT_UNDEFINED,
            .stencilAttachmentFormat = VK_FORMAT_UNDEFINED
        };

        // Juntando TOOOODOS os estados e criando o pipeline
        VkGraphicsPipelineCreateInfo pipelineCI {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = &renderingCI,
            .stageCount = static_cast<uint32_t>(std::size(shaderStages)),
            .pStages = shaderStages,
            .pVertexInputState = &vertexInputCI,
            .pInputAssemblyState = &inputAssemblyCI,
            .pTessellationState = nullptr,
            .pViewportState = &viewportCI,
            .pRasterizationState = &rasterizationCI,
            .pMultisampleState = &multisampleCI,
            .pDepthStencilState = &depthStencilCI,
            .pColorBlendState = &colorBlendCI,
            .pDynamicState = &dynamicStateCI,
            .layout = m_pipelineLayout,
            .renderPass = VK_NULL_HANDLE,
            .subpass = 0,
            .basePipelineHandle = VK_NULL_HANDLE,
            .basePipelineIndex = -1
        };

        check(vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &m_graphicsPipeline));

        // Limpando
        vkDestroyShaderModule(m_device, fragmentModule, nullptr);
        vkDestroyShaderModule(m_device, vertexModule, nullptr);
    }

    // Criando os semáforos de render finished
    void VulkanContext::createRenderFinishedSemaphores() {
        VkSemaphoreCreateInfo semaphoreCI {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
        };

        m_renderFinishedSemaphore.resize(m_swapchainImages.size(), VK_NULL_HANDLE);
        for (VkSemaphore& semaphore : m_renderFinishedSemaphore) {
            check(vkCreateSemaphore(m_device, &semaphoreCI, nullptr, &semaphore));
        }
    }

    void VulkanContext::createSyncObjects() {
        // Criando estruturas de criação para os semáforos e as cercas
        VkSemaphoreCreateInfo semaphoreCI {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
        };

        VkFenceCreateInfo fenceCI {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT   // FENCE JÁ LIGADA! Se não: loop infinito no startup
        };

        for (FrameData& frame : m_frames) {
            check(vkCreateSemaphore(m_device, &semaphoreCI, nullptr, &frame.ImageAvailableSemaphore));
            check(vkCreateFence(m_device, &fenceCI, nullptr, &frame.InFlightFence));
        }

        createRenderFinishedSemaphores();
    }

    void VulkanContext::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
        // Iniciando o command buffer
        VkCommandBufferBeginInfo beginInfo {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
        };

        check(vkBeginCommandBuffer(commandBuffer, &beginInfo));

        // Prepara a imagem para receber renderização
        transitionImage(
            commandBuffer, m_swapchainImages[imageIndex],
            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_PIPELINE_STAGE_2_NONE, 0,
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT
        );

        // Configura o attachment de cor
        VkRenderingAttachmentInfo colorAttachment {
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = m_swapchainImageViews[imageIndex],
            .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue {
                .color {
                    .float32 {
                        0.05f, 0.1f, 0.2f, 1.0f
                    }
                }
            }
        };

        VkRenderingInfo renderingInfo {
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .renderArea {
                .offset = {0, 0},
                .extent = m_swapchainExtent
            },
            .layerCount = 1,
            .colorAttachmentCount = 1,
            .pColorAttachments = &colorAttachment
        };

        // Limpa a imagem
        vkCmdBeginRendering(commandBuffer, &renderingInfo);
            // Bindando a pipeline
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);
                // Criando o viewport
                VkViewport viewport {
                    .x = 0.0f, .y = 0.0f,
                    .width = static_cast<float>(m_swapchainExtent.width), .height = static_cast<float>(m_swapchainExtent.height),
                    .minDepth = 0.0f, .maxDepth = 1.0f
                };

                // Criando o scissor
                VkRect2D scissor {
                    .offset = {0, 0},
                    .extent = m_swapchainExtent
                };

                vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
                vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

                VkDeviceSize vertexOffset = 0;
                vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_vertexBuffer, &vertexOffset);
                vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);

                vkCmdDrawIndexed(commandBuffer, m_indexCount, 1, 0, 0, 0);
        vkCmdEndRendering(commandBuffer);

        // Prepara a imagem para aparecer na tela
        transitionImage(
            commandBuffer, m_swapchainImages[imageIndex],
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
            VK_PIPELINE_STAGE_2_NONE, 0
        );

        check(vkEndCommandBuffer(commandBuffer));
    }

    void VulkanContext::transitionImage(
        VkCommandBuffer commandBuffer,
        VkImage image,
        VkImageLayout oldLayout,
        VkImageLayout newLayout,
        VkPipelineStageFlags2 srcStage,
        VkAccessFlags2 srcAccess,
        VkPipelineStageFlags2 dstStage,
        VkAccessFlags2 dstAccess
    ) {
        VkImageMemoryBarrier2 barrier{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = srcStage,
            .srcAccessMask = srcAccess,
            .dstStageMask = dstStage,
            .dstAccessMask = dstAccess,
            .oldLayout = oldLayout,
            .newLayout = newLayout,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = image,
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };

        VkDependencyInfo dependencyInfo{
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &barrier
        };

        vkCmdPipelineBarrier2(commandBuffer, &dependencyInfo);
    }

    VkShaderModule VulkanContext::createShaderModule(const std::vector<uint32_t>& code) {
        // Criando módulo do shader
        VkShaderModuleCreateInfo moduleCI {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = code.size() * sizeof(uint32_t),
            .pCode = code.data()
        };

        VkShaderModule shaderModule = VK_NULL_HANDLE;
        check(vkCreateShaderModule(m_device, &moduleCI, nullptr, &shaderModule));

        return shaderModule;
    }

    std::vector<uint32_t> VulkanContext::readShaderFile(const std::filesystem::path& path) {
        // FUNÇÃO DE LER ARQUIVO
        std::ifstream file(path, std::ios::ate | std::ios::binary);
        if (!file) {
            throw std::runtime_error(std::format("Couldn't open shader at '{}'", path.string()));
        }

        const std::streamsize fileSize = file.tellg();
        if (fileSize <= 0 || fileSize % 4 != 0) {
            throw std::runtime_error(std::format("Invalid SPIR-V file at '{}'", path.string()));
        }

        std::vector<uint32_t> code(static_cast<size_t>(fileSize) / sizeof(uint32_t));

        file.seekg(0);
        file.read(reinterpret_cast<char*>(code.data()), fileSize);
        if (!file) {
            throw std::runtime_error(std::format("Couldn't read shader at '{}'", path.string()));
        }

        return code;
    }

    void VulkanContext::createGeometryBuffers() {
        // Cria as informações
        const std::array<Vertex, 3> vertices {
            Vertex {
                .Position = { 0.0f, -0.5f, 0.0f },
                .Color = { 1.0f, 0.0f, 0.0f }
            },
            Vertex {
                .Position = { 0.5f, 0.5f, 0.0f },
                .Color = { 0.0f, 1.0f, 0.0f }
            },
            Vertex {
                .Position = { -0.5f, 0.5f, 0.0f },
                .Color = { 0.0f, 0.0f, 1.0f }
            }
        };

        const std::array<uint32_t, 3> indices {
            0, 1, 2
        };

        // Cria o staging buffer para o vertex
        const VkDeviceSize vertexBufferSize = sizeof(vertices);
        VkBuffer stagingVertexBuffer = VK_NULL_HANDLE;
        VmaAllocation stagingVertexAllocation = nullptr;
        createBuffer(
            vertexBufferSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
            stagingVertexBuffer,
            stagingVertexAllocation
        );

        // Copia os dados da CPU
        check(vmaCopyMemoryToAllocation(
            m_allocator,
            vertices.data(),
            stagingVertexAllocation,
            0,
            vertexBufferSize
        ));

        // Cria o VBO
        createBuffer(
            vertexBufferSize,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE, 0,
            m_vertexBuffer,
            m_vertexAllocation
        );

        // Copia staging para a GPU
        copyBuffer(stagingVertexBuffer, m_vertexBuffer, vertexBufferSize);

        // Destrói o staging
        vmaDestroyBuffer(m_allocator, stagingVertexBuffer, stagingVertexAllocation);

        // Cria o staging buffer para o index
        const VkDeviceSize indexBufferSize = sizeof(indices);
        VkBuffer stagingIndexBuffer = VK_NULL_HANDLE;
        VmaAllocation stagingIndexAllocation = nullptr;
        createBuffer(
            indexBufferSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
            stagingIndexBuffer,
            stagingIndexAllocation
        );

        // Copia os dados da CPU
        check(vmaCopyMemoryToAllocation(
            m_allocator,
            indices.data(),
            stagingIndexAllocation,
            0,
            indexBufferSize
        ));

        // Cria o EBO
        createBuffer(
            indexBufferSize,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE, 0,
            m_indexBuffer,
            m_indexAllocation
        );

        // Copia staging para a GPU
        copyBuffer(stagingIndexBuffer, m_indexBuffer, indexBufferSize);

        // Destrói o staging
        vmaDestroyBuffer(m_allocator, stagingIndexBuffer, stagingIndexAllocation);

        m_indexCount = static_cast<uint32_t>(indices.size());
    }

    // Helper para criar buffers
    void VulkanContext::createBuffer(
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VmaMemoryUsage memoryUsage,
        VmaAllocationCreateFlags allocationFlags,
        VkBuffer& buffer,
        VmaAllocation& allocation
    ) {
        VkBufferCreateInfo bufferCI {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = size,
            .usage = usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE
        };

        VmaAllocationCreateInfo allocationCI {
            .flags = allocationFlags,
            .usage = memoryUsage
        };

        check(vmaCreateBuffer(m_allocator, &bufferCI, &allocationCI, &buffer, &allocation, nullptr));
    }

    // Helper para copiar buffers
    void VulkanContext::copyBuffer(VkBuffer src, VkBuffer dest, VkDeviceSize size) {
        VkCommandBufferAllocateInfo allocInfo {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = m_commandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1
        };

        VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
        check(vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer));

        VkCommandBufferBeginInfo beginInfo {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
        };

        check(vkBeginCommandBuffer(commandBuffer, &beginInfo));
            VkBufferCopy copyRegion {
                .srcOffset = 0,
                .dstOffset = 0,
                .size = size
            };

            vkCmdCopyBuffer(commandBuffer, src, dest, 1, &copyRegion);
        check(vkEndCommandBuffer(commandBuffer));

        VkCommandBufferSubmitInfo commandBufferInfo {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
            .commandBuffer = commandBuffer,
            .deviceMask = 1
        };

        VkSubmitInfo2 submitInfo {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
            .commandBufferInfoCount = 1,
            .pCommandBufferInfos = &commandBufferInfo
        };

        check(vkQueueSubmit2(m_queue, 1, &submitInfo, VK_NULL_HANDLE));
        check(vkQueueWaitIdle(m_queue));

        vkFreeCommandBuffers(m_device, m_commandPool, 1, &commandBuffer);
    }

    // Retorna a área cliente em pixels
    bool VulkanContext::getWindowPixelSize(int& width, int& height) {
        check(SDL_GetWindowSizeInPixels(
            m_window->GetHandle(),
            &width, &height
        ), std::format("Couldn't get window pixel size: {}", SDL_GetError()));

        return width > 0 && height > 0;
    }
}
