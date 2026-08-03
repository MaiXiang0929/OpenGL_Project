// SPDX-License-Identifier: MIT
/// @file Renderer.h
/// @brief 渲染器核心类的头文件
/// @details 该文件声明了 Renderer 类的核心功能，包括初始化、渲染循环、资源管理等。
/// @author MaiX
/// @date 2026-08-01

#pragma once

#include <vector>
#include <string>

#include <glad/glad.h>

#include "cyTriMesh.h"
#include "cyMatrix.h"

#include "Shader.h"
#include "Mesh.h"
#include "Editor/LightGizmo.h"

class Renderer
{
public:
	/// @brief 构造函数
    Renderer();

	/// @brief 析构函数，释放 OpenGL 资源
    ~Renderer();

	// 禁止复制
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

	/// @brief 初始化 Renderer
    void Init();

	/// @brief 开始一帧渲染
    void BeginFrame();

    /// @brief 渲染场景
    /// @param mvp 
    /// @param mv 
    /// @param lightPosView 
    void RenderScene(
        const cy::Matrix4f& mvp,
        const cy::Matrix4f& mv,
        const cy::Vec3f& lightPosView
    );

	/// @brief 结束一帧渲染
    void EndFrame();

	/// @brief 上传模型数据到GPU
	/// @param vertices 顶点坐标
    /// @param normals 法线
    /// @param texCoords 纹理坐标
    void SetMesh(const std::vector<Vertex>& vertices);

    // --- 新增：暴露给外部的纹理加载接口 ---
    void LoadTextures(
        const std::string& diffusePath,
        const std::string& specularPath
    );

    // --- 新增：重新加载着色器的接口 ---
    void ReloadShaders();

    // --- 修改：适配你 LightGizmo API 的绘制接口 ---
    void DrawLightGizmo(
        const cy::Matrix4f& proj,
        const cy::Matrix4f& view,
        const cy::Vec3f& lightWorldPos,
        float scale
    );

private:

    // Shader
    Shader m_MainShader;

    // Editor Debug
    LightGizmo m_LightGizmo;

    // Mesh GPU Resource
    Mesh m_Mesh;

    // Texture GPU Resource
    GLuint m_DiffuseTexture = 0;
    GLuint m_SpecularTexture = 0;

    // Mesh Info
private:

    // 内部 PNG 加载逻辑
    GLuint LoadTexturePNG(
        const std::string& filePath
    );
};
