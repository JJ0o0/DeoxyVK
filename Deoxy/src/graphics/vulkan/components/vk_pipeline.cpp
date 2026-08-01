#include "vk_pipeline.hpp"
#include "vk_helper.hpp"
#include "../shading/vk_push_constants.hpp"

#include <deoxy/graphics/vertex.hpp>

#include <array>
#include <cstddef>
#include <format>
#include <fstream>
#include <stdexcept>

namespace deoxy::graphics::vulkan {
    VulkanPipeline::VulkanPipeline(
        VkDevice device,
        VkFormat colorFormat, VkFormat depthFormat,
        VkDescriptorSetLayout cameraSetLayout, VkDescriptorSetLayout textureSetLayout,
        const std::filesystem::path& vertexShaderPath,
        const std::filesystem::path& fragmentShaderPath
    ) : m_device(device) {
        CheckBool(m_device != VK_NULL_HANDLE, "Pipeline received a null logical device");
        CheckBool(colorFormat != VK_FORMAT_UNDEFINED, "Pipeline received an undefined color format");
        CheckBool(depthFormat != VK_FORMAT_UNDEFINED, "Pipeline received an undefined depth format");

        create(colorFormat, depthFormat, cameraSetLayout, textureSetLayout, vertexShaderPath, fragmentShaderPath);
    }

    VulkanPipeline::~VulkanPipeline() { destroy(); }

    void VulkanPipeline::create(
        VkFormat colorFormat, VkFormat depthFormat,
        VkDescriptorSetLayout cameraSetLayout, VkDescriptorSetLayout textureSetLayout,
        const std::filesystem::path& vertexShaderPath,
        const std::filesystem::path& fragmentShaderPath
    ) {
        VkShaderModule vertexModule = VK_NULL_HANDLE;
        VkShaderModule fragmentModule = VK_NULL_HANDLE;

        try {
            // Lendo e criando os shaders
            const auto vertexCode = readShaderFile(vertexShaderPath);
            const auto fragmentCode = readShaderFile(fragmentShaderPath);

            vertexModule = createShaderModule(vertexCode);
            fragmentModule = createShaderModule(fragmentCode);

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

            const std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages {
                vertexStage, fragmentStage
            };

            // Definindo como o Vulkan avança pelos elementos do vertex buffer
            VkVertexInputBindingDescription bindingDescription {
                .binding = 0,
                .stride = sizeof(Vertex),
                .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
            };

            // Define o que cada parte de Vertex é no shader
            std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions {
                VkVertexInputAttributeDescription {
                    .location = 0,
                    .binding = 0,
                    .format = VK_FORMAT_R32G32B32_SFLOAT,
                    .offset = offsetof(Vertex, Position)
                },
                VkVertexInputAttributeDescription {
                    .location = 1,
                    .binding = 0,
                    .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                    .offset = offsetof(Vertex, Tint)
                },
                VkVertexInputAttributeDescription {
                    .location = 2,
                    .binding = 0,
                    .format = VK_FORMAT_R32G32_SFLOAT,
                    .offset = offsetof(Vertex, UV)
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
            const std::array<VkDynamicState, 2> dynamicStates {
                VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR
            };

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
                .cullMode = VK_CULL_MODE_BACK_BIT,
                .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
                .depthBiasEnable = VK_FALSE,
                .lineWidth = 1.0f
            };

            // Definindo configurações do MSAA
            VkPipelineMultisampleStateCreateInfo multisampleCI {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
                .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
                .sampleShadingEnable = VK_FALSE,
            };

            // Definindo configurações do Depth Test
            VkPipelineDepthStencilStateCreateInfo depthStencilCI {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
                .depthTestEnable = VK_TRUE,
                .depthWriteEnable = VK_TRUE,
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
                .attachmentCount = 1,
                .pAttachments = &colorBlendAttachment
            };

            // Criando informações das Push Constants
            VkPushConstantRange pushConstantRange {
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                .offset = 0,
                .size = sizeof(MeshPushConstants)
            };

            // Criando o layout da pipeline
            std::array<VkDescriptorSetLayout, 2> setLayouts {
                cameraSetLayout,
                textureSetLayout
            };

            VkPipelineLayoutCreateInfo layoutCI {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                .setLayoutCount = static_cast<uint32_t>(setLayouts.size()),
                .pSetLayouts = setLayouts.data(),
                .pushConstantRangeCount = 1,
                .pPushConstantRanges = &pushConstantRange,
            };

            CheckResult(vkCreatePipelineLayout(m_device, &layoutCI, nullptr, &m_layout));

            // Informando o formato usado pelo dynamic rendering
            VkPipelineRenderingCreateInfo renderingCI {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
                .colorAttachmentCount = 1,
                .pColorAttachmentFormats = &colorFormat,
                .depthAttachmentFormat = depthFormat,
            };

            // Juntando TOOOODOS os estados e criando o pipeline
            VkGraphicsPipelineCreateInfo pipelineCI {
                .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
                .pNext = &renderingCI,
                .stageCount = static_cast<uint32_t>(shaderStages.size()),
                .pStages = shaderStages.data(),
                .pVertexInputState = &vertexInputCI,
                .pInputAssemblyState = &inputAssemblyCI,
                .pTessellationState = nullptr,
                .pViewportState = &viewportCI,
                .pRasterizationState = &rasterizationCI,
                .pMultisampleState = &multisampleCI,
                .pDepthStencilState = &depthStencilCI,
                .pColorBlendState = &colorBlendCI,
                .pDynamicState = &dynamicStateCI,
                .layout = m_layout,
                .renderPass = VK_NULL_HANDLE,
                .subpass = 0,
                .basePipelineHandle = VK_NULL_HANDLE,
                .basePipelineIndex = -1
            };

            CheckResult(vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &m_pipeline));
        } catch (...) {
            if (fragmentModule != VK_NULL_HANDLE) vkDestroyShaderModule(m_device, fragmentModule, nullptr);
            if (vertexModule != VK_NULL_HANDLE) vkDestroyShaderModule(m_device, vertexModule, nullptr);

            destroy();
            throw;
        }

        // Limpando
        vkDestroyShaderModule(m_device, fragmentModule, nullptr);
        vkDestroyShaderModule(m_device, vertexModule, nullptr);
    }

    void VulkanPipeline::destroy() {
        if (m_pipeline != VK_NULL_HANDLE) vkDestroyPipeline(m_device, m_pipeline, nullptr);
        if (m_layout != VK_NULL_HANDLE) vkDestroyPipelineLayout(m_device, m_layout, nullptr);
    }

    VkShaderModule VulkanPipeline::createShaderModule(const std::vector<uint32_t>& code) const {
        // Criando módulo do shader
        VkShaderModuleCreateInfo moduleCI {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = code.size() * sizeof(uint32_t),
            .pCode = code.data()
        };

        VkShaderModule shaderModule = VK_NULL_HANDLE;
        CheckResult(vkCreateShaderModule(m_device, &moduleCI, nullptr, &shaderModule));

        return shaderModule;
    }

    std::vector<uint32_t> VulkanPipeline::readShaderFile(const std::filesystem::path& path) {
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
}
