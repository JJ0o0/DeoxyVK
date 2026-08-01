include_guard(GLOBAL)

function(deoxy_copy_assets TARGET_NAME)
    if(NOT TARGET "${TARGET_NAME}")
        message(FATAL_ERROR "deoxy_copy_assets: target '${TARGET_NAME}' doesn't exist")
    endif()

    set(ASSETS_SOURCE_DIRECTORY "${PROJECT_SOURCE_DIR}/assets")
    set(ASSETS_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>/assets")

    if(NOT EXISTS "${ASSETS_SOURCE_DIRECTORY}")
        message(FATAL_ERROR "Assets directory not found at '${ASSETS_SOURCE_DIRECTORY}'")
    endif()

    set(ASSETS_TARGET "${TARGET_NAME}_assets")

    add_custom_target("${ASSETS_TARGET}" ALL
        COMMAND
            "${CMAKE_COMMAND}" -E make_directory
            "${ASSETS_OUTPUT_DIRECTORY}"
        COMMAND
            "${CMAKE_COMMAND}" -E copy_directory_if_different
            "${ASSETS_SOURCE_DIRECTORY}"
            "${ASSETS_OUTPUT_DIRECTORY}"
        COMMENT "Copying assets for '${TARGET_NAME}'"
        VERBATIM
    )

    add_dependencies(
        "${TARGET_NAME}"
        "${ASSETS_TARGET}"
    )
endfunction()

find_program(SLANGC_EXECUTABLE
    NAMES slangc
    REQUIRED
)

function(deoxy_compile_shader TARGET_NAME SHADER_NAME)
    if (NOT TARGET "${TARGET_NAME}")
        message(FATAL_ERROR
            "deoxy_compile_shader: target '${TARGET_NAME}' doesn't exists"
        )
    endif()

    set(SHADER_SOURCE
        "${PROJECT_SOURCE_DIR}/assets/shaders/${SHADER_NAME}.slang"
    )

    if (NOT EXISTS "${SHADER_SOURCE}")
        message(FATAL_ERROR
            "Shader source not found at '${SHADER_SOURCE}'!"
        )
    endif()

    set(SHADER_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>/assets/shaders")
    set(VERTEX_OUTPUT "${SHADER_OUTPUT_DIRECTORY}/${SHADER_NAME}.vert.spv")
    set(FRAGMENT_OUTPUT "${SHADER_OUTPUT_DIRECTORY}/${SHADER_NAME}.frag.spv")

    add_custom_command(
        OUTPUT
            "${VERTEX_OUTPUT}"
            "${FRAGMENT_OUTPUT}"
        COMMAND
            "${CMAKE_COMMAND}" -E make_directory
            "${SHADER_OUTPUT_DIRECTORY}"
        COMMAND
            "${SLANGC_EXECUTABLE}"
            "${SHADER_SOURCE}"
            -target spirv
            -profile glsl_450
            -entry vertexMain
            -stage vertex
            -matrix-layout-column-major
            -o "${VERTEX_OUTPUT}"
        COMMAND
            "${SLANGC_EXECUTABLE}"
            "${SHADER_SOURCE}"
            -target spirv
            -profile glsl_450
            -entry fragmentMain
            -stage fragment
            -matrix-layout-column-major
            -o "${FRAGMENT_OUTPUT}"
        DEPENDS "${SHADER_SOURCE}"
        COMMENT "Compiling shader '${SHADER_NAME}'"
        VERBATIM
    )

    set(SHADER_TARGET "${TARGET_NAME}_${SHADER_NAME}_shader")

    add_custom_target("${SHADER_TARGET}"
        DEPENDS
            "${VERTEX_OUTPUT}"
            "${FRAGMENT_OUTPUT}"
    )

    set(ASSETS_TARGET "${TARGET_NAME}_assets")

    if(TARGET "${ASSETS_TARGET}")
        add_dependencies(
            "${SHADER_TARGET}"
            "${ASSETS_TARGET}"
        )
    endif()

    add_dependencies(
        "${TARGET_NAME}"
        "${SHADER_TARGET}"
    )
endfunction()
