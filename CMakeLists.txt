cmake_minimum_required(VERSION 3.15)

project(OpenGL_Project)

set(CMAKE_CXX_STANDARD 17)

include_directories(
    libs/glfw/include
    libs/glad/include
)

add_executable(OpenGL_Project
    src/main.cpp
    libs/glad/src/glad.c
)

# GLFW库路径（注意版本可能不同）
set(GLFW_LIB ${CMAKE_SOURCE_DIR}/libs/glfw/lib-vc2022/glfw3.lib)

target_link_libraries(OpenGL_Project
    ${GLFW_LIB}
    opengl32
)

