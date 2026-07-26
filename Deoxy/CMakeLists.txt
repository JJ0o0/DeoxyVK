add_library(Deoxy STATIC)
add_library(Deoxy::Deoxy ALIAS Deoxy)

target_sources(Deoxy
    PRIVATE
        "src/vma_implementation.cpp"
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
    PUBLIC
        SDL3::SDL3
        volk::volk
        glm::glm
    PRIVATE
        GPUOpen::VulkanMemoryAllocator
)
