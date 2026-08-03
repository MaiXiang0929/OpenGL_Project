cmake_minimum_required(VERSION 3.15)

project(OpenGL_Project)

# ============================
# C++ 标准
# ============================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# MSVC 使用 UTF-8 编码
add_compile_options(
    "$<$<CXX_COMPILER_ID:MSVC>:/utf-8>"
)

# ============================
# 目录定义
# ============================
set(PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

set(SRC_DIR
    ${PROJECT_ROOT}/src
)

set(THIRD_PARTY_DIR
    ${PROJECT_ROOT}/ThirdParty
)

set(ASSET_DIR
    ${PROJECT_ROOT}/assets
)

# ============================
# Source Files
# ============================

set(SOURCES

    # 程序入口
    src/main.cpp

    # Core
    src/Core/Application.cpp
    src/Core/Camera.cpp

    # Renderer
    src/Renderer/Renderer.cpp
    src/Renderer/Shader.cpp
    src/Renderer/Mesh.cpp

    # Editor
    src/Editor/LightGizmo.cpp

    # GLAD
    ThirdParty/glad/src/glad.c

    # LodePNG
    ThirdParty/lodepng/lodepng.cpp
)

# ============================
# 创建可执行文件
# ============================
add_executable(
    ${PROJECT_NAME}
    ${SOURCES}
 )

# ============================
# Include Path
# ============================
target_include_directories(
    ${PROJECT_NAME}

    PRIVATE

    # src
    ${SRC_DIR}

    # Core
    ${SRC_DIR}/Core

    # Renderer
    ${SRC_DIR}/Renderer

    # Editor
    ${SRC_DIR}/Editor

    # GLAD
    ${THIRD_PARTY_DIR}/glad/include

    # GLFW
    ${THIRD_PARTY_DIR}/glfw/include

    # cyCodeBase
    ${THIRD_PARTY_DIR}/cyCodeBase

    # LodePNG
    ${THIRD_PARTY_DIR}/lodepng
)

# ============================
# GLFW
# ============================
target_link_directories(
    ${PROJECT_NAME}

    PRIVATE

    ${THIRD_PARTY_DIR}/glfw/lib-vc2022
)

target_link_libraries(
    ${PROJECT_NAME}

    PRIVATE

    glfw3.lib

    opengl32.lib
)

# ============================
# Copy Assets
# ============================
add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD

    COMMAND ${CMAKE_COMMAND} -E copy_directory

    ${ASSET_DIR}

    $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
)

# ============================
# Visual Studio 文件结构
# ============================
source_group(
    TREE ${PROJECT_ROOT}

    FILES

    ${SOURCES}
)