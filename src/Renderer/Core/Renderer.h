// SPDX-License-Identifier: MIT
/// @file Renderer.h
/// @brief 渲染器类的头文件
/// @details 该文件声明了 Renderer 类，它是渲染资源的所有者和渲染管线的门面，负责准备资源上下文并启动 RenderPipeline。
/// @author MaiX
/// @date 2026-08-11

#pragma once

#include <string>
#include <vector>

#include "Renderer/Resources/CubemapTexture.h"
#include "Renderer/Resources/Material.h"
#include "Renderer/Resources/Mesh.h"
#include "Renderer/Pipeline/RenderPipeline.h"
#include "Renderer/Pipeline/RenderSettings.h"

/// @brief 渲染资源的所有者与管线门面。
/// @details Renderer 不再实现具体 Pass，只负责准备资源上下文并启动 RenderPipeline。
class Renderer
{
public:
    Renderer();
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    void Init();

    void SetMesh(const std::vector<Vertex>& vertices);
    void SetLightMesh(const std::vector<Vertex>& vertices);
    void SetMaterial(Material material);
    void LoadCubemap(const std::string& directoryPath);

    void ExecutePipeline(RenderFrameData& frame);
    void ReloadShaders();

    void SetShadowsEnabled(bool enabled) { m_ShadowsEnabled = enabled; }
    bool IsShadowsEnabled() const { return m_ShadowsEnabled; }

    void SetTessellationEnabled(bool enabled)
    {
        m_Tessellation.enabled = enabled;
    }
    bool IsTessellationEnabled() const
    {
        return m_Tessellation.enabled;
    }
    void SetTessellationLevel(float level);
    float GetTessellationLevel() const { return m_Tessellation.level; }
    void SetDisplacementScale(float scale);
    float GetDisplacementScale() const
    {
        return m_Tessellation.displacementScale;
    }
    void SetTessellationWireframe(bool enabled)
    {
        m_Tessellation.wireframe = enabled;
    }
    bool IsTessellationWireframe() const
    {
        return m_Tessellation.wireframe;
    }

private:
    Mesh m_SceneMesh;
    Mesh m_LightMesh;
    Mesh m_PresentMesh;
    Mesh m_SkyboxMesh;
    Mesh m_GroundMesh;

    Material m_MainMaterial;
    CubemapTexture m_Cubemap;
    RenderPipeline m_RenderPipeline;
    TessellationSettings m_Tessellation;
    bool m_ShadowsEnabled = true;
};
