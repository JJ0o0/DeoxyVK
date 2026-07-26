add_library(Deoxy STATIC)
add_library(Deoxy::Deoxy ALIAS Deoxy)

target_sources(Deoxy
    PRIVATE
        "src/vma_implementation.cpp"

        "src/core/application.cpp"

        "src/graphics/vulkan/vk_context.cpp"
        "src/graphics/renderer.cpp"

        "src/platform/message_box.cpp"
        "src/platform/logger.cpp"
        "src/platform/window.cpp"
)

target_include_directories(Deoxy
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(Deoxy
    PUBLIC
        cxx_std_20
)

target_compile_definitions(Deoxy
    PUBLIC
        # Necessário para usar Volk em vez dos protótipos Vulkan
        VK_NO_PROTOTYPES

        # Faz o GLM usar profundidade Vulkan de 0 até 1
        GLM_FORCE_DEPTH_ZERO_TO_ONE
)

target_link_libraries(Deoxy
    PRIVATE
        SDL3::SDL3
        volk::volk
        glm::glm
        GPUOpen::VulkanMemoryAllocator

        spdlog::spdlog
        tinyfiledialogs::tinyfiledialogs
)
