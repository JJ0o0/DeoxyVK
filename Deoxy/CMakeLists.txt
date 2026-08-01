add_library(Deoxy STATIC)
add_library(Deoxy::Deoxy ALIAS Deoxy)

target_sources(Deoxy
    PRIVATE
        "src/vma_implementation.cpp"
        "src/stb_implementation.cpp"

        "src/core/application.cpp"

        "src/graphics/vulkan/vk_context.cpp"
        "src/graphics/vulkan/components/vk_allocator.cpp"
        "src/graphics/vulkan/components/vk_command_pool.cpp"
        "src/graphics/vulkan/components/vk_device.cpp"
        "src/graphics/vulkan/components/vk_frame.cpp"
        "src/graphics/vulkan/components/vk_instance.cpp"
        "src/graphics/vulkan/components/vk_pipeline.cpp"
        "src/graphics/vulkan/components/vk_surface.cpp"
        "src/graphics/vulkan/components/vk_swapchain.cpp"
        "src/graphics/vulkan/components/vk_buffer.cpp"
        "src/graphics/vulkan/components/vk_depth_buffer.cpp"
        "src/graphics/vulkan/components/vk_commands.cpp"
        "src/graphics/vulkan/components/vk_descriptor_pool.cpp"
        "src/graphics/vulkan/components/vk_image.cpp"
        "src/graphics/vulkan/components/vk_sampler.cpp"
        "src/graphics/vulkan/components/vk_texture.cpp"

        "src/graphics/vulkan/graphical/vk_mesh.cpp"

        "src/graphics/vulkan/shading/vk_descriptor_set_layout.cpp"

        "src/graphics/renderer.cpp"
        "src/graphics/mesh_generator.cpp"
        "src/graphics/image_loader.cpp"

        "src/input/input.cpp"

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
)

target_link_libraries(Deoxy
    PRIVATE
        SDL3::SDL3
        volk::volk
        GPUOpen::VulkanMemoryAllocator

        spdlog::spdlog
        tinyfiledialogs::tinyfiledialogs
        stb::stb
)
