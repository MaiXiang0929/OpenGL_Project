// SPDX-License-Identifier: MIT
/// @file Renderer.cpp
/// @brief 渲染器类的实现文件
/// @details 该文件实现了 Renderer 类的核心功能，包括初始化渲染管线、设置场景网格和材质、加载立方体贴图、执行渲染管线等。
/// @author MaiX
/// @date 2026-08-11


#include "Renderer.h"

#include <algorithm>
#include <iostream>
#include <utility>

Renderer::Renderer() = default;
Renderer::~Renderer() = default;

void Renderer::Init()
{
    glEnable(GL_DEPTH_TEST);

    if (!m_RenderPipeline.Init())
        std::cerr << "[Renderer] RenderPipeline 初始化失败。" << std::endl;

    // Present Pass 使用的屏幕四边形，只负责显示 ForwardPass 颜色纹理。
    const std::vector<Vertex> presentVertices = {
        {{-1,-1,0},{0,0,1},{0,0}}, {{ 1,-1,0},{0,0,1},{1,0}}, {{ 1, 1,0},{0,0,1},{1,1}},
        {{-1,-1,0},{0,0,1},{0,0}}, {{ 1, 1,0},{0,0,1},{1,1}}, {{-1, 1,0},{0,0,1},{0,1}}
    };
    m_PresentMesh.Upload(presentVertices);

    const float s = 1.0f;
    const std::vector<Vertex> skyboxVertices = {
        {{ s,-s,-s},{0,0,0},{0,0}}, {{ s, s,-s},{0,0,0},{0,0}}, {{ s, s, s},{0,0,0},{0,0}},
        {{ s,-s,-s},{0,0,0},{0,0}}, {{ s, s, s},{0,0,0},{0,0}}, {{ s,-s, s},{0,0,0},{0,0}},
        {{-s,-s, s},{0,0,0},{0,0}}, {{-s, s, s},{0,0,0},{0,0}}, {{-s, s,-s},{0,0,0},{0,0}},
        {{-s,-s, s},{0,0,0},{0,0}}, {{-s, s,-s},{0,0,0},{0,0}}, {{-s,-s,-s},{0,0,0},{0,0}},
        {{ s, s,-s},{0,0,0},{0,0}}, {{-s, s,-s},{0,0,0},{0,0}}, {{-s, s, s},{0,0,0},{0,0}},
        {{ s, s,-s},{0,0,0},{0,0}}, {{-s, s, s},{0,0,0},{0,0}}, {{ s, s, s},{0,0,0},{0,0}},
        {{ s,-s, s},{0,0,0},{0,0}}, {{-s,-s, s},{0,0,0},{0,0}}, {{-s,-s,-s},{0,0,0},{0,0}},
        {{ s,-s, s},{0,0,0},{0,0}}, {{-s,-s,-s},{0,0,0},{0,0}}, {{ s,-s,-s},{0,0,0},{0,0}},
        {{ s,-s, s},{0,0,0},{0,0}}, {{ s, s, s},{0,0,0},{0,0}}, {{-s, s, s},{0,0,0},{0,0}},
        {{ s,-s, s},{0,0,0},{0,0}}, {{-s, s, s},{0,0,0},{0,0}}, {{-s,-s, s},{0,0,0},{0,0}},
        {{-s,-s,-s},{0,0,0},{0,0}}, {{-s, s,-s},{0,0,0},{0,0}}, {{ s, s,-s},{0,0,0},{0,0}},
        {{-s,-s,-s},{0,0,0},{0,0}}, {{ s, s,-s},{0,0,0},{0,0}}, {{ s,-s,-s},{0,0,0},{0,0}}
    };
    m_SkyboxMesh.Upload(skyboxVertices);

    const std::vector<Vertex> groundVertices = {
        {{-1,0,-1},{0,1,0},{0,0}}, {{ 1,0,-1},{0,1,0},{0,0}}, {{ 1,0, 1},{0,1,0},{0,0}},
        {{-1,0,-1},{0,1,0},{0,0}}, {{ 1,0, 1},{0,1,0},{0,0}}, {{-1,0, 1},{0,1,0},{0,0}}
    };
    m_GroundMesh.Upload(groundVertices);

    LoadCubemap("assets/models/cubemap");
}

void Renderer::SetMesh(const std::vector<Vertex>& vertices)
{
    m_SceneMesh.Upload(vertices);
}

void Renderer::SetLightMesh(const std::vector<Vertex>& vertices)
{
    m_LightMesh.Upload(vertices);
}

void Renderer::SetMaterial(Material material)
{
    m_MainMaterial = std::move(material);
}

void Renderer::LoadCubemap(const std::string& directoryPath)
{
    if (m_Cubemap.Load(directoryPath))
    {
        std::cout << "[Renderer] Cubemap loaded from: "
                  << directoryPath << std::endl;
    }
}

void Renderer::ExecutePipeline(RenderFrameData& frame)
{
    frame.shadowsEnabled = m_ShadowsEnabled;

    // Renderer 在这里完成资源注入，Application 和 RenderPipeline 都不直接拥有资源。
    RenderPassContext context{
        frame,
        m_SceneMesh,
        m_LightMesh,
        m_PresentMesh,
        m_SkyboxMesh,
        m_GroundMesh,
        m_MainMaterial,
        m_Cubemap,
        m_Tessellation
    };
    m_RenderPipeline.Execute(context);
}

void Renderer::ReloadShaders()
{
    if (m_RenderPipeline.ReloadShaders())
        std::cout << "[Renderer] Shaders reloaded successfully!" << std::endl;
    else
        std::cerr << "[Renderer] One or more shaders failed to reload."
                  << std::endl;
}

void Renderer::SetTessellationLevel(float level)
{
    m_Tessellation.level = std::clamp(level, 1.0f, 64.0f);
}

void Renderer::SetDisplacementScale(float scale)
{
    m_Tessellation.displacementScale = std::max(scale, 0.0f);
}
