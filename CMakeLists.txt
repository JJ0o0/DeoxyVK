cmake_minimum_required(VERSION 3.24)
project(
    DeoxyVK
    VERSION 0.0.1
    LANGUAGES C CXX
)

# VARIÁVEIS DO CMAKE
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

option(DEOXY_BUILD_SANDBOX "Build the Deoxy sandbox application" ON)

# DEPENDÊNCIAS
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(dependencies)
deoxy_setup_dependencies()

# SUBDIRETÓRIOS
add_subdirectory(Deoxy)

if(DEOXY_BUILD_SANDBOX)
    include(shader_compilation)
    add_subdirectory(Sandbox)
endif()
