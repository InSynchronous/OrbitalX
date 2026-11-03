cmake_minimum_required(VERSION 3.10)

project(OrbitalX LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glfw3 REQUIRED)

set(BGFX_ROOT ${CMAKE_SOURCE_DIR}/submodules/bgfx)
set(BX_ROOT   ${CMAKE_SOURCE_DIR}/submodules/bx)
set(BIMG_ROOT ${CMAKE_SOURCE_DIR}/submodules/bimg)

set(BGFX_BUILD ${BGFX_ROOT}/.build/linux64_gcc/bin)

add_library(bgfx STATIC IMPORTED)
set_target_properties(bgfx PROPERTIES
    IMPORTED_LOCATION "${BGFX_BUILD}/libbgfxRelease.a"
)

add_library(bx STATIC IMPORTED)
set_target_properties(bx PROPERTIES
    IMPORTED_LOCATION "${BGFX_BUILD}/libbxRelease.a"
)

add_library(bimg STATIC IMPORTED)
set_target_properties(bimg PROPERTIES
    IMPORTED_LOCATION "${BGFX_BUILD}/libbimgRelease.a"
)

# Dear ImGui
set(IMGUI_ROOT ${CMAKE_SOURCE_DIR}/submodules/imgui)

add_library(imgui STATIC
    ${IMGUI_ROOT}/imgui.cpp
    ${IMGUI_ROOT}/imgui_draw.cpp
    ${IMGUI_ROOT}/imgui_tables.cpp
    ${IMGUI_ROOT}/imgui_widgets.cpp
    ${IMGUI_ROOT}/backends/imgui_impl_glfw.cpp
)

target_include_directories(imgui PUBLIC
    ${IMGUI_ROOT}
    ${IMGUI_ROOT}/backends
)

target_link_libraries(imgui PUBLIC glfw)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# OrbitalX
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    src/*.cpp
)

add_executable(OrbitalX ${SOURCES})

target_compile_definitions(OrbitalX PRIVATE
    BX_CONFIG_DEBUG=0
)

target_compile_options(OrbitalX PRIVATE
  -O3
  -march=native
)

target_include_directories(OrbitalX PRIVATE
    ${BGFX_ROOT}/include
    ${BX_ROOT}/include
    ${BIMG_ROOT}/include
)

target_link_libraries(OrbitalX PRIVATE
    bgfx
    bx
    bimg
    glfw
    dl
    pthread
    imgui
)
