// SPDX-License-Identifier: MIT
#include "ReflectionPass.h"

#include "ForwardPass.h"

bool ReflectionPass::Init(unsigned int width, unsigned int height)
{
    m_Framebuffer.Init(width, height);
    return true;
}

bool ReflectionPass::ReloadShaders()
{
    return true;
}

void ReflectionPass::Execute(RenderPassContext& context)
{
    m_Framebuffer.Bind();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_ForwardPass.RenderSkybox(context, context.frame.reflectionView);
    m_ForwardPass.RenderSurface(context, true);

    m_Framebuffer.Unbind();
    m_Framebuffer.GenerateMipmaps();
    context.reflectionTexture = m_Framebuffer.GetColorTexture();
}
