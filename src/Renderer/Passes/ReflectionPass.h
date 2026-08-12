// SPDX-License-Identifier: MIT
#pragma once

#include "Renderer/Resources/Framebuffer.h"
#include "Renderer/Pipeline/RenderPass.h"

class ForwardPass;

/// @brief 反射 Pass：用镜像视图重绘天空盒和 teapot 到独立纹理。
class ReflectionPass final : public RenderPass
{
public:
    explicit ReflectionPass(ForwardPass& forwardPass)
        : m_ForwardPass(forwardPass)
    {
    }

    bool Init(unsigned int width, unsigned int height);
    bool ReloadShaders();

    RenderPassType GetType() const override
    {
        return RenderPassType::Reflection;
    }
    void Execute(RenderPassContext& context) override;

private:
    ForwardPass& m_ForwardPass;
    Framebuffer m_Framebuffer;
};
