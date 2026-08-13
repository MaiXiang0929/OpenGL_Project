// SPDX-License-Identifier: MIT
#include "ReflectionPass.h"

#include "ForwardPass.h"

bool ReflectionPass::Init()
{
    return true;
}

bool ReflectionPass::Resize(unsigned int width, unsigned int height)
{
    return m_Framebuffer.Init(
        static_cast<int>(width), static_cast<int>(height));
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
    m_ForwardPass.RenderSurface(context, context.reflectionView);

    m_Framebuffer.Unbind();
    m_Framebuffer.GenerateMipmaps();
    context.reflectionTexture = m_Framebuffer.GetColorTexture();
}
