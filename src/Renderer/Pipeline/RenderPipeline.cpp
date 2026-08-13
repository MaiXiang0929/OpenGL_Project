// SPDX-License-Identifier: MIT
/// @file RenderPipeline.cpp
/// @brief 渲染管线类的实现文件
/// @details 该文件实现了 RenderPipeline 类的核心功能，包括初始化各个渲染阶段、执行渲染流程等。
/// @author MaiX
/// @date 2026-08-11


#include "RenderPipeline.h"

#include <iostream>

#include "Renderer/Diagnostics/GpuDebugScope.h"
#include "Renderer/Diagnostics/RenderSubmissionStats.h"
#include "Renderer/Core/OpenGLStateCache.h"

namespace
{
const char* GetPassDebugName(RenderPassType type)
{
    switch (type)
    {
    case RenderPassType::Shadow: return "MaiX.ShadowPass";
    case RenderPassType::Reflection: return "MaiX.ReflectionPass";
    case RenderPassType::Forward: return "MaiX.ForwardPass";
    case RenderPassType::Translucency: return "MaiX.TranslucencyPass";
    case RenderPassType::EditorPrimitive: return "MaiX.EditorPrimitivePass";
    case RenderPassType::Present: return "MaiX.PresentPass";
    }
    return "MaiX.UnknownPass";
}
}

RenderPipeline::RenderPipeline()
    : m_TranslucencyPass(m_ForwardPass)
    , m_EditorPrimitivePass(m_ForwardPass)
    , m_ReflectionPass(m_ForwardPass)
    , m_PresentPass(m_ForwardPass)
{
    // 顺序由 GPU 资源依赖决定：阴影和反射必须先于主颜色与 Present。
    m_Passes = {
        &m_ShadowPass,
        &m_ReflectionPass,
        &m_ForwardPass,
        &m_TranslucencyPass,
        &m_EditorPrimitivePass,
        &m_PresentPass
    };
}

bool RenderPipeline::Init()
{
    const bool forwardLoaded = m_ForwardPass.Init(1024, 1024);
    const bool shadowLoaded = m_ShadowPass.Init(2048, 2048);
    const bool reflectionLoaded = m_ReflectionPass.Init(512, 512);
    const bool editorPrimitivesLoaded = m_EditorPrimitivePass.Init();
    const bool presentLoaded = m_PresentPass.Init();
    const bool profilerInitialized = m_GpuProfiler.Init();
    if (!profilerInitialized)
    {
        std::cerr
            << "[RenderPipeline] GPU timing disabled because Timer Query "
            << "initialization failed."
            << std::endl;
    }
    return forwardLoaded && shadowLoaded && reflectionLoaded &&
        editorPrimitivesLoaded && presentLoaded;
}

bool RenderPipeline::ReloadShaders()
{
    return m_ForwardPass.ReloadShaders() &&
        m_ShadowPass.ReloadShaders() &&
        m_ReflectionPass.ReloadShaders() &&
        m_EditorPrimitivePass.ReloadShaders() &&
        m_PresentPass.ReloadShaders();
}

void RenderPipeline::Execute(RenderPassContext& context)
{
    RenderSubmissionStats& stats = RenderSubmissionStats::Get();
    stats.BeginFrame();
    m_GpuProfiler.BeginFrame();
    for (RenderPass* pass : m_Passes)
    {
        const RenderPassType type = pass->GetType();
        OpenGLStateCache::Get().Invalidate();
        stats.BeginPass(type);
        m_GpuProfiler.BeginPass(type);
        const GpuDebugScope debugScope(GetPassDebugName(type));
        pass->Execute(context);
        m_GpuProfiler.EndPass();
        stats.EndPass();
    }
    stats.EndFrame();
    m_GpuProfiler.EndFrame();
}
