// SPDX-License-Identifier: MIT
#pragma once

#include "Renderer/Resources/Framebuffer.h"
#include "Renderer/Pipeline/RenderPass.h"
#include "Renderer/Resources/Shader.h"

/// @brief 主颜色 Pass，负责场景正向渲染和主 HDR/颜色目标。
class ForwardPass final : public RenderPass
{
public:
    bool Init(unsigned int width, unsigned int height);
    bool ReloadShaders();

    RenderPassType GetType() const override { return RenderPassType::Forward; }
    void Execute(RenderPassContext& context) override;

    /// ReflectionPass 复用同一套材质 Shader，确保反射与主画面一致。
    void RenderSurface(RenderPassContext& context, bool reflectedView);
    void RenderSkybox(
        RenderPassContext& context,
        const cy::Matrix4f& view);

    GLuint GetColorTexture() const { return m_Framebuffer.GetColorTexture(); }

private:
    Shader m_StandardShader;
    Shader m_TessellationShader;
    Shader m_SkyboxShader;
    Shader m_GroundShader;
    Shader m_LightShader;
    Framebuffer m_Framebuffer;
};
