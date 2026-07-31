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

    # SPDLOG
    set(SPDLOG_BUILD_EXAMPLE OFF CACHE BOOL "" FORCE)
    set(SPDLOG_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(SPDLOG_BUILD_BENCH OFF CACHE BOOL "" FORCE)
    set(SPDLOG_INSTALL OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(
        spdlog

        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.17.0
        GIT_SHALLOW TRUE

        FIND_PACKAGE_ARGS 1.17 CONFIG
    )

    # TINYFILEDIALOGS
    FetchContent_Declare(
        tinyfiledialogs

        GIT_REPOSITORY https://git.code.sf.net/p/tinyfiledialogs/code
        GIT_TAG 58f4a2
    )

    FetchContent_MakeAvailable(
        SDL3
        volk
        VulkanMemoryAllocator
        spdlog
        tinyfiledialogs
    )

    # Normaliza o Volk
    if(TARGET volk AND NOT TARGET volk::volk)
        add_library(volk::volk ALIAS volk)
    endif()

    # Criar target para tinyfiledialogs
    if(NOT TARGET tinyfiledialogs)
        add_library(tinyfiledialogs
            STATIC
                "${tinyfiledialogs_SOURCE_DIR}/tinyfiledialogs.c"
        )

        add_library(tinyfiledialogs::tinyfiledialogs ALIAS tinyfiledialogs)

        target_include_directories(tinyfiledialogs SYSTEM
            PUBLIC
                "${tinyfiledialogs_SOURCE_DIR}"
        )

        if(WIN32)
            target_link_libraries(tinyfiledialogs
                PUBLIC
                    comdlg32
                    ole32
                    user32
                    shell32
            )
        endif()
    endif()
endfunction()
