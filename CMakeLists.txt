cmake_minimum_required(VERSION 3.20)
project(nvboard VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- Dependencies ---
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)
pkg_check_modules(SDL2_IMAGE REQUIRED IMPORTED_TARGET SDL2_image)
pkg_check_modules(SDL2_TTF REQUIRED IMPORTED_TARGET SDL2_ttf)
find_package(absl REQUIRED)

# --- Sources ---
set(NVBOARD_SOURCES
  src/button.cc
  src/component.cc
  src/event.cc
  src/font.cc
  src/keyboard.cc
  src/led.cc
  src/nvboard.cc
  src/render.cc
  src/segs7.cc
  src/switch.cc
  src/term.cc
  src/uart.cc
  src/vga.cc
)

# --- Library ---
add_library(nvboard STATIC ${NVBOARD_SOURCES})
target_include_directories(nvboard
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(nvboard
  PUBLIC
    PkgConfig::SDL2
    PkgConfig::SDL2_IMAGE
    PkgConfig::SDL2_TTF
    absl::span
  PRIVATE
    absl::absl_check
    absl::absl_log
    absl::log_initialize
    absl::strings
    absl::time
    absl::base
)
target_compile_options(nvboard PRIVATE
  -Wall -Wextra -Wpedantic
  -Wno-nullability-extension
  -Wno-c99-designator
  -Wno-c++20-designator
  -Wno-variadic-macro-arguments-omitted
)

# --- Install ---
include(GNUInstallDirs)

install(TARGETS nvboard
  EXPORT nvboard-targets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY include/nvboard
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(DIRECTORY resources
  DESTINATION ${CMAKE_INSTALL_DATADIR}/nvboard
)

# --- CMake config for downstream ---
install(EXPORT nvboard-targets
  FILE nvboard-targets.cmake
  NAMESPACE nvboard::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/nvboard
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/nvboard-config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/nvboard-config.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/nvboard
)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/nvboard-config-version.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/nvboard-config.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/nvboard-config-version.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/nvboard
)
