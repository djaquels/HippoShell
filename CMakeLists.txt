cmake_minimum_required(VERSION 3.14)
project(hippo-shell)

# Use C++17
set(CMAKE_CXX_STANDARD 17)

# Add vcpkg integration if available
if(DEFINED ENV{VCPKG_ROOT})
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING "")
endif()

#include local libs directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Include sources
add_executable(hippo-shell
    src/main.cpp
    src/ollama_client.cpp
    src/command_runner.cpp
    src/context.cpp 
)

# Link libraries installed with vcpkg
find_package(nlohmann_json CONFIG REQUIRED)
find_package(CURL REQUIRED)

target_link_libraries(hippo-shell
    PRIVATE
    nlohmann_json::nlohmann_json
    CURL::libcurl
)
