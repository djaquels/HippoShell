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
# Set the include directories for the project
include_directories(
	${CMAKE_SOURCE_DIR}/include
	${CURSES_INCLUDE_DIRS}
	)
# Ncurses configuration
set(CURSES_NEED_NCURSES TRUE)
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
find_package(Curses REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(readline REQUIRED IMPORTED_TARGET readline)

target_link_libraries(hippo-shell
    PRIVATE
    nlohmann_json::nlohmann_json
    CURL::libcurl
    PkgConfig::readline
    ${CURSES_LIBRARIES}
)
