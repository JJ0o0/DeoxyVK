include_guard(GLOBAL)
include(FetchContent)

function(deoxy_setup_dependencies)
    set(
        FETCHCONTENT_BASE_DIR
            "${CMAKE_BINARY_DIR}/Vendored"
        CACHE PATH
            "Directory used for vendored dependencies"
        FORCE
    )

    # Vulkan (Volk só usa os Headers)
    find_package(Vulkan REQUIRED)

    # SDL3
    set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
    set(SDL_TESTS OFF CACHE BOOL "" FORCE)
    set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(
        SDL3

        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG release-3.4.12
        GIT_SHALLOW TRUE

        FIND_PACKAGE_ARGS CONFIG
    )

    # Volk
    FetchContent_Declare(
        volk

        GIT_REPOSITORY https://github.com/zeux/volk.git
        GIT_TAG 1.4.350
        GIT_SHALLOW TRUE

        OVERRIDE_FIND_PACKAGE
    )

    # Vulkan Memory Allocator
    FetchContent_Declare(
        VulkanMemoryAllocator

        GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
        GIT_TAG v3.4.0
        GIT_SHALLOW TRUE

        FIND_PACKAGE_ARGS CONFIG
    )

    # GLM
    FetchContent_Declare(
        glm

        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG 1.0.3
        GIT_SHALLOW TRUE

        FIND_PACKAGE_ARGS CONFIG
    )

    FetchContent_MakeAvailable(
        SDL3
        volk
        VulkanMemoryAllocator
        glm
    )

    # Normaliza o Volk
    if(TARGET volk AND NOT TARGET volk::volk)
        add_library(volk::volk ALIAS volk)
    endif()
endfunction()
