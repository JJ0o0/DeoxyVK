add_executable(Sandbox)

target_sources(Sandbox
    PRIVATE
        "src/main.cpp"
)

target_link_libraries(Sandbox
    PRIVATE
        Deoxy::Deoxy
)
