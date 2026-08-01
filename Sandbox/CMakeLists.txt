add_executable(Sandbox)

target_sources(Sandbox
    PRIVATE
        "src/main.cpp"
        "src/sandbox_application.cpp"
        "src/camera.cpp"
)

target_include_directories(Sandbox
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

set_target_properties(Sandbox
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY
            "${CMAKE_BINARY_DIR}/bin/$<CONFIG>"
        VS_DEBUGGER_WORKING_DIRECTORY
            "$<TARGET_FILE_DIR:Sandbox>"
)

deoxy_compile_shader(
    Sandbox
    basic
)

target_link_libraries(Sandbox
    PRIVATE
        Deoxy::Deoxy
)
