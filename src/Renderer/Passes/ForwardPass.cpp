// SPDX-License-Identifier: MIT
#include "ForwardPass.h"

#include "Renderer/Resources/CubemapTexture.h"
#include "Renderer/Resources/Material.h"
#include "Renderer/Resources/Mesh.h"
#include "Renderer/Pipeline/RenderSettings.h"

bool ForwardPass::Init(unsigned int width, unsigned int height)
{
    const bool loaded = ReloadShaders();
    m_Framebuffer.Init(width, height);
    return loaded;
}

bool ForwardPass::ReloadShaders()
{
    const bool standardLoaded = m_StandardShader.Load(
        "assets/shaders/pbr/pbr.vert",
        "assets/shaders/pbr/pbr.frag");
    const bool tessellationLoaded = m_TessellationShader.LoadTessellation(
        "assets/shaders/pbr/tessellation/pbr_tess.vert",
        "assets/shaders/pbr/tessellation/pbr_tess.tesc",
        "assets/shaders/pbr/tessellation/pbr_tess.tese",
        "assets/shaders/pbr/pbr.frag");
    const bool skyboxLoaded = m_SkyboxShader.Load(
        "assets/shaders/environment/skybox.vert",
        "assets/shaders/environment/skybox.frag");
    const bool groundLoaded = m_GroundShader.Load(
        "assets/shaders/reflection/ground.vert",
        "assets/shaders/reflection/ground.frag");
    const bool lightLoaded = m_LightShader.Load(
        "assets/shaders/debug/light_object.vert",
        "assets/shaders/debug/light_object.frag");
    return standardLoaded && tessellationLoaded && skyboxLoaded &&
        groundLoaded && lightLoaded;
}

void ForwardPass::Execute(RenderPassContext& context)
{
    // CPU 侧切换主颜色目标；随后所有绘制命令写入同一个 Framebuffer。
    m_Framebuffer.Bind();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    RenderSkybox(context, context.frame.view);

    m_GroundShader.Bind();
    m_GroundShader.SetMatrix4("mvp", &context.frame.groundMvp.cell[0]);
    m_GroundShader.SetMatrix4("model", &context.frame.groundModel.cell[0]);
    m_GroundShader.SetMatrix4(
        "reflectionVP", &context.frame.reflectionVP.cell[0]);
    m_GroundShader.SetMatrix4("lightVP", &context.frame.lightVP.cell[0]);
    m_GroundShader.SetVec3(
        "cameraWorldPos",
        context.frame.cameraWorldPosition.x,
        context.frame.cameraWorldPosition.y,
        context.frame.cameraWorldPosition.z);
    context.cubemap.Bind(0);
    m_GroundShader.SetInt("cubemap", 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, context.reflectionTexture);
    m_GroundShader.SetInt("reflectionTex", 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, context.shadowTexture);
    m_GroundShader.SetInt("shadowMap", 2);
    m_GroundShader.SetInt(
        "shadowsEnabled", context.frame.shadowsEnabled ? 1 : 0);
    context.groundMesh.Draw();

    RenderSurface(context, false);

    m_LightShader.Bind();
    m_LightShader.SetMatrix4(
        "mvp", &context.frame.lightObjectMvp.cell[0]);
    m_LightShader.SetVec3("lightColor", 1.0f, 0.85f, 0.15f);
    context.lightMesh.Draw();

    m_Framebuffer.Unbind();
    m_Framebuffer.GenerateMipmaps();
}

void ForwardPass::RenderSurface(
    RenderPassContext& context,
    bool reflectedView)
{
    const cy::Matrix4f& mvp = reflectedView
        ? context.frame.reflectionMvp
        : context.frame.mvp;
    const cy::Matrix4f& mv = reflectedView
        ? context.frame.reflectionMv
        : context.frame.mv;
    const cy::Matrix4f& view = reflectedView
        ? context.frame.reflectionView
        : context.frame.view;
    const cy::Vec3f& lightPosition = reflectedView
        ? context.frame.reflectionLightPositionView
        : context.frame.lightPositionView;

    Shader& shader = context.tessellation.enabled
        ? m_TessellationShader
        : m_StandardShader;
    shader.Bind();
    shader.SetMatrix4("mvp", &mvp.cell[0]);
    shader.SetMatrix4("mv", &mv.cell[0]);
    shader.SetMatrix4("lightMvp", &context.frame.lightMvp.cell[0]);
    shader.SetVec3(
        "lightPos", lightPosition.x, lightPosition.y, lightPosition.z);
    shader.SetVec3("lightColor", 1.0f, 0.95f, 0.85f);
    shader.SetFloat("lightIntensity", 5.0f);

    const float viewToWorld[9] = {
        view.cell[0], view.cell[4], view.cell[8],
        view.cell[1], view.cell[5], view.cell[9],
        view.cell[2], view.cell[6], view.cell[10]
    };
    const GLint viewToWorldLocation = glGetUniformLocation(
        shader.GetProgramID(), "viewToWorld");
    if (viewToWorldLocation != -1)
        glUniformMatrix3fv(viewToWorldLocation, 1, GL_FALSE, viewToWorld);

    context.material.Bind(shader, 0);
    if (context.tessellation.enabled)
    {
        shader.SetFloat("tessellationLevel", context.tessellation.level);
        shader.SetFloat(
            "displacementScale", context.tessellation.displacementScale);
    }
    shader.SetInt("wireframePass", 0);

    context.cubemap.Bind(4);
    shader.SetInt("cubemap", 4);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, context.shadowTexture);
    shader.SetInt("shadowMap", 5);
    shader.SetInt(
        "shadowsEnabled", context.frame.shadowsEnabled ? 1 : 0);

    if (!context.tessellation.enabled)
    {
        context.sceneMesh.Draw();
        return;
    }

    glPatchParameteri(GL_PATCH_VERTICES, 3);
    context.sceneMesh.DrawPatches();
    if (!context.tessellation.wireframe || reflectedView)
        return;

    shader.SetInt("wireframePass", 1);
    glEnable(GL_POLYGON_OFFSET_LINE);
    glPolygonOffset(-1.0f, -1.0f);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    context.sceneMesh.DrawPatches();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_POLYGON_OFFSET_LINE);
    shader.SetInt("wireframePass", 0);
}

void ForwardPass::RenderSkybox(
    RenderPassContext& context,
    const cy::Matrix4f& view)
{
    if (!context.cubemap.IsValid())
        return;

    cy::Matrix4f skyboxView = view;
    skyboxView.cell[12] = 0.0f;
    skyboxView.cell[13] = 0.0f;
    skyboxView.cell[14] = 0.0f;

    m_SkyboxShader.Bind();
    m_SkyboxShader.SetMatrix4(
        "projection", &context.frame.projection.cell[0]);
    m_SkyboxShader.SetMatrix4("view", &skyboxView.cell[0]);
    context.cubemap.Bind(0);
    m_SkyboxShader.SetInt("skybox", 0);

    glDepthMask(GL_FALSE);
    glDepthFunc(GL_LEQUAL);
    context.skyboxMesh.Draw();
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
}
