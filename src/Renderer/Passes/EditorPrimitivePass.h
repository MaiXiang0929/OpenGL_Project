// SPDX-License-Identifier: MIT
#pragma once

#include <glad/glad.h>

#include "Renderer/Pipeline/RenderPass.h"
#include "Renderer/Resources/Shader.h"

class ForwardPass;

class EditorPrimitivePass final : public RenderPass
{
public:
    explicit EditorPrimitivePass(ForwardPass& forwardPass)
        : m_ForwardPass(forwardPass)
    {
    }

    ~EditorPrimitivePass() override;

    EditorPrimitivePass(const EditorPrimitivePass&) = delete;
    EditorPrimitivePass& operator=(const EditorPrimitivePass&) = delete;

    bool Init();
    bool ReloadShaders();

    RenderPassType GetType() const override
    {
        return RenderPassType::EditorPrimitive;
    }
    void Execute(RenderPassContext& context) override;

private:
    void CreateBillboardMesh();
    void CreateSpotConeMesh();
    void DrawBillboard(
        const RenderPassContext& context,
        const cy::Vec3f& position,
        const cy::Vec3f& color);
    void DrawSpotCone(
        const RenderPassContext& context,
        const struct LightSceneProxy& light);

    ForwardPass& m_ForwardPass;
    Shader m_BillboardShader;
    Shader m_LineShader;
    GLuint m_BillboardVao = 0;
    GLuint m_BillboardVbo = 0;
    GLuint m_ConeVao = 0;
    GLuint m_ConeVbo = 0;
    GLsizei m_ConeVertexCount = 0;
};
