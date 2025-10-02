cmake_minimum_required(VERSION 3.18.1)

project(engine_renderer LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_library(engine_renderer SHARED
    engine_renderer.cpp
    camera.cpp
    egl_context.cpp
    grid_plane.cpp
    shader_program.cpp
    jni_bridge.cpp
)

target_include_directories(engine_renderer
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_options(engine_renderer
    PRIVATE
        $<$<CONFIG:Release>:-Ofast>
        $<$<CONFIG:Debug>:-O0>
        -Wall
        -Wextra
        -Werror
        -Wno-unused-parameter
        -Wno-missing-field-initializers
)

find_library(log-lib log)
find_library(android-lib android)
find_library(egl-lib EGL)
find_library(gles-lib GLESv3)

# Link against the shared C++ runtime that ships with the NDK.
find_library(gabi++ c++_shared)

if(gabi++)
    set(runtime_lib ${gabi++})
else()
    set(runtime_lib c++_shared)
endif()

target_link_libraries(engine_renderer
    PRIVATE
        ${log-lib}
        ${android-lib}
        ${egl-lib}
        ${gles-lib}
        ${runtime_lib}
)
