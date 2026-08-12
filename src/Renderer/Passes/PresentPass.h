// SPDX-License-Identifier: MIT
#pragma once

#include "Renderer/Pipeline/RenderPass.h"
#include "Renderer/Resources/Shader.h"

class ForwardPass;

/// @brief Present Pass：将 ForwardPass 的颜色纹理绘制到默认窗口目标。
class PresentPass final : public RenderPass
{
public:
    explicit PresentPass(ForwardPass& forwardPass)
        : m_ForwardPass(forwardPass)
    {
    }

    bool Init();
    bool ReloadShaders();

    RenderPassType GetType() const override
    {
        return RenderPassType::Present;
    }
    void Execute(RenderPassContext& context) override;

private:
    ForwardPass& m_ForwardPass;
    Shader m_Shader;
};
