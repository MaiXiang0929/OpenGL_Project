cmake_minimum_required(VERSION 3.15)

project(OpenGL_Project)

set(CMAKE_CXX_STANDARD 17)

# 强制 MSVC 编译器使用 UTF-8 编码读取源文件
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

include_directories(
    libs/glfw/include
    libs/glad/include
    libs/cyCodeBase
)

add_executable(OpenGL_Project
    src/main.cpp
    libs/glad/src/glad.c
 "src/shader.h")

# GLFW库路径（注意版本可能不同）
set(GLFW_LIB ${CMAKE_SOURCE_DIR}/libs/glfw/lib-vc2022/glfw3.lib)

target_link_libraries(OpenGL_Project
    ${GLFW_LIB}
    opengl32
)

# 编译后自动将源码目录下的 assets 文件夹拷贝到可执行文件输出目录
add_custom_command(TARGET OpenGL_Project POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:OpenGL_Project>/assets
)
