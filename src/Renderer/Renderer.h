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
#include "Framebuffer.h"
#include "ShadowMap.h"
#include "RenderPipeline.h"
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
	/// @details 该函数会设置视口大小，并为离屏渲染阶段做准备。
	/// @param viewportWidth 当前视口宽度
    /// @param viewportHeight 当前视口高度
    void BeginFrame(unsigned int viewportWidth, unsigned int viewportHeight);

    /// @brief 开始物体离屏渲染阶段
    void BeginObjectPass();

    /// @brief 结束物体离屏渲染阶段并更新颜色纹理的 MipMap
    void EndObjectPass();

    /// @brief 开始从聚光灯视角写入深度贴图
    void BeginShadowPass();

    /// @brief 将 OBJ 作为阴影投射物绘制到深度贴图
    /// @param lightMvp OBJ 局部空间到光源裁剪空间的变换
    void RenderShadowCaster(const cy::Matrix4f& lightMvp);

    /// @brief 结束深度 pass，并恢复会影响后续颜色 pass 的渲染状态
    void EndShadowPass();

    /// @brief 设置阴影功能是否启用；默认启用
    void SetShadowsEnabled(bool enabled) { m_ShadowsEnabled = enabled; }

    /// @brief 查询当前是否需要生成并采样阴影贴图
    bool IsShadowsEnabled() const { return m_ShadowsEnabled; }

    /// @brief 渲染场景
    /// @param mvp 
    /// @param mv 
    /// @param lightPosView 
    void RenderScene(
        const cy::Matrix4f& mvp,
        const cy::Matrix4f& mv,
        const cy::Vec3f& lightPosView,
        const cy::Matrix4f& view,
        const cy::Matrix4f& lightMvp
    );

    /// @brief 将离屏渲染纹理绘制到方形平面
    /// @param mvp 平面的模型-观察-投影矩阵
    void RenderPlane(const cy::Matrix4f& mvp);
    void LoadTessellationTextures(const std::string& normalPath, const std::string& displacementPath);
    void RenderTessellatedPlane(const cy::Matrix4f& mvp, const cy::Matrix4f& lightMvp,
        const cy::Vec3f& lightPosition, const cy::Vec3f& cameraPosition);
    void SetTessellationLevel(float level);
    void SetTriangulationVisible(bool visible) { m_ShowTriangulation = visible; }
    float GetTessellationLevel() const { return m_TessellationLevel; }
    bool IsTriangulationVisible() const { return m_ShowTriangulation; }

	/// @brief 结束一帧渲染
    void EndFrame();

    /// Execute the renderer-owned pass graph for the current frame.
    void ExecutePipeline(RenderPassContext& context) { m_RenderPipeline.Execute(context); }

	/// @brief 上传模型数据到GPU
	/// @param vertices 顶点坐标
    /// @param normals 法线
    /// @param texCoords 纹理坐标
    void SetMesh(const std::vector<Vertex>& vertices);
    void SetLightMesh(const std::vector<Vertex>& vertices);
    void RenderLightObject(const cy::Matrix4f& mvp);

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

    void SetDiffuseTexture(GLuint textureID);

    /// @brief 获取 FBO 引用
    Framebuffer& GetFramebuffer() { return m_Framebuffer; }

    // --- 新增：天空盒相关接口 ---
    void LoadCubemap(const std::string& dirPath);
    void RenderSkybox(const cy::Matrix4f& projection, const cy::Matrix4f& view);

    // --- 新增：反射 Pass 与地面平面接口 ---
    void BeginReflectionPass();
    void EndReflectionPass();
    void RenderGroundPlane(
        const cy::Matrix4f& mvp,
        const cy::Matrix4f& model,
        const cy::Matrix4f& reflectionVP,
        const cy::Vec3f& cameraWorldPos,
        const cy::Matrix4f& lightVP
    );

private:

    // Shader
	Shader m_MainShader;                ///< 物体渲染的主着色器
    Shader m_PlaneShader;               ///< 显示离屏纹理的平面着色器
    Shader m_TessShader;
    Shader m_TessWireShader;
    Shader m_TessShadowShader;
    Shader m_LightObjectShader;
    Shader m_SkyboxShader;              ///< 天空盒着色器
    Shader m_GroundShader;              ///< 反射地面着色器
    Shader m_ShadowDepthShader;         ///< 从聚光灯视角写深度的着色器

    // Framebuffer
    Framebuffer m_Framebuffer;
    Framebuffer m_ReflectionFramebuffer; ///< 反射渲染目标（512×512）
    ShadowMap m_ShadowMap;               ///< 可供颜色 pass 采样的深度比较纹理
    ShadowMap m_TessShadowMap;
    RenderPipeline m_RenderPipeline;
    bool m_ShadowsEnabled = true;         ///< S 键控制；默认显示阴影

    // Editor Debug
    LightGizmo m_LightGizmo;

    // Mesh GPU Resource
	Mesh m_Mesh;                        ///< 物体模型网格
    Mesh m_LightMesh;
    Mesh m_PlaneMesh;                   ///< 由两个三角形组成的正方形平面
    Mesh m_SkyboxMesh;                  ///< 天空盒立方体网格
    Mesh m_GroundPlaneMesh;             ///< 反射地面网格（XZ 平面四边形）

    // Texture GPU Resource
    GLuint m_DiffuseTexture = 0;
    GLuint m_SpecularTexture = 0;
    GLuint m_CubemapTexture = 0;            ///< 天空盒立方体贴图
    GLuint m_NormalTexture = 0;
    GLuint m_DisplacementTexture = 0;
    float m_TessellationLevel = 32.0f;
    bool m_ShowTriangulation = false;

    // Mesh Info
private:

    // 内部 PNG 加载逻辑
    GLuint LoadTexturePNG(
        const std::string& filePath
    );
};
