add_library(engine_core STATIC
    camera.cpp
    engine_assembly.cpp
    kinematics_system.cpp
    gltf_loader.cpp
    grid_plane.cpp
    json_utils.cpp
    mesh.cpp
    shader_program.cpp
)

target_include_directories(engine_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../..
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_options(engine_core
    PRIVATE
        $<$<CONFIG:Release>:-Ofast>
        $<$<CONFIG:Debug>:-O0>
        -Wall
        -Wextra
        -Werror
        -Wno-unused-parameter
        -Wno-missing-field-initializers
)
