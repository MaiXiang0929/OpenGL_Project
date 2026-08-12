// SPDX-License-Identifier: MIT
/// @file RenderPipeline.cpp
/// @brief 渲染管线类的实现文件
/// @details 该文件实现了 RenderPipeline 类的核心功能，包括初始化各个渲染阶段、执行渲染流程等。
/// @author MaiX
/// @date 2026-08-11


#include "RenderPipeline.h"

RenderPipeline::RenderPipeline()
    : m_ReflectionPass(m_ForwardPass)
    , m_PresentPass(m_ForwardPass)
{
    // 顺序由 GPU 资源依赖决定：阴影和反射必须先于主颜色与 Present。
    m_Passes = {
        &m_ShadowPass,
        &m_ReflectionPass,
        &m_ForwardPass,
        &m_PresentPass
    };
}

bool RenderPipeline::Init()
{
    const bool forwardLoaded = m_ForwardPass.Init(1024, 1024);
    const bool shadowLoaded = m_ShadowPass.Init(2048, 2048);
    const bool reflectionLoaded = m_ReflectionPass.Init(512, 512);
    const bool presentLoaded = m_PresentPass.Init();
    return forwardLoaded && shadowLoaded && reflectionLoaded && presentLoaded;
}

bool RenderPipeline::ReloadShaders()
{
    return m_ForwardPass.ReloadShaders() &&
        m_ShadowPass.ReloadShaders() &&
        m_ReflectionPass.ReloadShaders() &&
        m_PresentPass.ReloadShaders();
}

void RenderPipeline::Execute(RenderPassContext& context)
{
    for (RenderPass* pass : m_Passes)
        pass->Execute(context);
}
