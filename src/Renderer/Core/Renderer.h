// SPDX-License-Identifier: MIT
/// @file Renderer.h
/// @brief 渲染器类的头文件
/// @details 该文件声明了 Renderer 类，它是渲染资源的所有者和渲染管线的门面，负责准备资源上下文并启动 RenderPipeline。
/// @author MaiX
/// @date 2026-08-11

#pragma once

#include <array>
#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include "Renderer/Resources/CubemapTexture.h"
#include "Renderer/Resources/Material.h"
#include "Renderer/Resources/Mesh.h"
#include "Renderer/Pipeline/RenderPipeline.h"
#include "Renderer/Pipeline/RenderSettings.h"
#include "Renderer/Scene/RenderScene.h"

/// @brief 渲染资源的所有者与管线门面。
/// @details Renderer 不再实现具体 Pass，只负责准备资源上下文并启动 RenderPipeline。
class Renderer
{
public:
    Renderer();
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    bool Init();

    MeshHandle CreateMesh(const std::vector<Vertex>& vertices);
    MaterialHandle CreateMaterial(Material material);

    PrimitiveId AddPrimitive(
        MeshHandle mesh,
        MaterialHandle material,
        const cy::Matrix4f& localToWorld,
        PrimitiveBounds bounds = {},
        bool castsShadow = true,
        bool translucent = false);

    /// @brief 便利接口：创建独立 Mesh/Material 后提交一个 Primitive。
    PrimitiveId AddPrimitive(
        const std::vector<Vertex>& vertices,
        Material material,
        const cy::Matrix4f& localToWorld,
        PrimitiveBounds bounds = {},
        bool castsShadow = true,
        bool translucent = false);
    bool UpdatePrimitiveTransform(
        PrimitiveId id,
        const cy::Matrix4f& localToWorld);

    LightId AddLight(LightSceneProxy light);
    bool UpdateLight(LightId id, const LightSceneProxy& light);

    void LoadCubemap(const std::string& directoryPath);

    void ExecutePipeline(RenderFrameData& frame);
    void ReloadShaders();
    const GpuTimingSnapshot& GetGpuTimingSnapshot() const
    {
        return m_RenderPipeline.GetGpuTimingSnapshot();
    }

    std::size_t GetMeshResourceCount() const
    {
        return m_MeshResources.size();
    }
    std::size_t GetMaterialResourceCount() const
    {
        return m_MaterialResources.size();
    }

    void SetShadowsEnabled(bool enabled) { m_ShadowsEnabled = enabled; }
    bool IsShadowsEnabled() const { return m_ShadowsEnabled; }

    void SetEditorPrimitivesEnabled(bool enabled)
    {
        m_EditorPrimitivesEnabled = enabled;
    }
    bool AreEditorPrimitivesEnabled() const
    {
        return m_EditorPrimitivesEnabled;
    }

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
    void LogMainViewStatsIfChanged(const RenderView& view);

    Mesh m_PresentMesh;
    Mesh m_SkyboxMesh;
    Mesh m_GroundMesh;

    // Renderer 唯一拥有场景 GPU 资源；Scene Proxy 只缓存解析后的非 owning 指针。
    std::vector<std::unique_ptr<Mesh>> m_MeshResources;
    std::vector<std::unique_ptr<Material>> m_MaterialResources;
    RenderScene m_RenderScene;
    CubemapTexture m_Cubemap;
    RenderPipeline m_RenderPipeline;
    TessellationSettings m_Tessellation;
    std::array<std::size_t, 9> m_LastMainViewStats{};
    bool m_HasMainViewStats = false;
    bool m_ShadowsEnabled = true;
    bool m_EditorPrimitivesEnabled = true;
};
