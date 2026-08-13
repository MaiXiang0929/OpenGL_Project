// SPDX-License-Identifier: MIT
/// @file ShadowPass.cpp
/// @brief 阴影渲染阶段的实现文件
/// @details 该文件实现了 ShadowPass 类的核心功能，包括初始化阴影贴图、加载着色器、执行阴影渲染流程等。
/// @author MaiX
/// @date 2026-08-11


#include "ShadowPass.h"

#include "Renderer/Resources/Material.h"
#include "Renderer/Resources/Mesh.h"
#include "Renderer/Pipeline/RenderSettings.h"
#include "Renderer/Diagnostics/RenderSubmissionStats.h"
#include "Renderer/View/RenderView.h"

bool ShadowPass::Init(unsigned int width, unsigned int height)
{
    const bool shadersLoaded = ReloadShaders();
    m_ShadowMap.Init(width, height);
    return shadersLoaded;
}

bool ShadowPass::ReloadShaders()
{
    const bool standardLoaded = m_StandardShader.Load(
        "assets/shaders/shadow/shadow_depth.vert",
        "assets/shaders/shadow/shadow_depth.frag");
    const bool tessellationLoaded = m_TessellationShader.LoadTessellation(
        "assets/shaders/pbr/tessellation/pbr_tess.vert",
        "assets/shaders/pbr/tessellation/pbr_tess.tesc",
        "assets/shaders/shadow/shadow_tess.tese",
        "assets/shaders/shadow/shadow_depth.frag");
    return standardLoaded && tessellationLoaded;
}

void ShadowPass::Execute(RenderPassContext& context)
{
    context.shadowTexture = m_ShadowMap.GetDepthTexture();
    if (!context.frame.shadowsEnabled)
        return;

    m_ShadowMap.BindForWriting();
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(2.0f, 4.0f);

    Shader& shader = context.tessellation.enabled
        ? m_TessellationShader
        : m_StandardShader;
    shader.Bind();
    if (context.tessellation.enabled)
    {
        glPatchParameteri(GL_PATCH_VERTICES, 3);
        shader.SetFloat("tessellationLevel", context.tessellation.level);
        shader.SetFloat(
            "displacementScale", context.tessellation.displacementScale);
    }

    for (const RenderItem& item : context.shadowView.opaqueItems)
    {
        if (!item.castsShadow || item.mesh == nullptr || item.material == nullptr)
            continue;

        const cy::Matrix4f lightMvp = context.frame.lightVP * item.model;
        shader.SetMatrix4("lightMvp", &lightMvp.cell[0]);
        if (context.tessellation.enabled)
        {
            RenderSubmissionStats::Get().RecordMaterialBind(item.material);
            item.material->BindDisplacement(shader, 0);
            item.mesh->DrawPatches();
        }
        else
        {
            item.mesh->Draw();
        }
    }

    glDisable(GL_POLYGON_OFFSET_FILL);
    m_ShadowMap.Unbind();
}
