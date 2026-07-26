add_executable(Sandbox)

target_sources(Sandbox
    PRIVATE
        "src/main.cpp"
        "src/sandbox_application.cpp"
)

target_include_directories(Sandbox
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(Sandbox
    PRIVATE
        Deoxy::Deoxy
)
