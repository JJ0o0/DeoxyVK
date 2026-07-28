include_guard(GLOBAL)

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

    set(SHADER_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/shaders")
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
            -o "${VERTEX_OUTPUT}"

        COMMAND
            "${SLANGC_EXECUTABLE}"
            "${SHADER_SOURCE}"
            -target spirv
            -profile glsl_450
            -entry fragmentMain
            -stage fragment
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

    add_dependencies(
        "${TARGET_NAME}"
        "${SHADER_TARGET}"
    )
endfunction()
