cmake_minimum_required(VERSION 3.16)
project(MyRaylibProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(app
    src/main.cpp
    src/scene/scene.cpp
)

include_directories(
    "${CMAKE_SOURCE_DIR}/libs/raylib"
    "${CMAKE_SOURCE_DIR}/src"
)

target_compile_definitions(app PRIVATE GRAPHICS_API_OPENGL_43)

# Link static raylib + system libs
target_link_libraries(app
    ${CMAKE_SOURCE_DIR}/libs/raylib/raylib4.3/libraylib.a
    m
    pthread
    dl
    rt
    X11
    GL
)