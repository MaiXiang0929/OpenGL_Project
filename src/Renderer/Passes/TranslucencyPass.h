// SPDX-License-Identifier: MIT
#pragma once

#include "Renderer/Pipeline/RenderPass.h"

class ForwardPass;

/// @brief 将主视图透明物体混合到 ForwardPass 已生成的颜色目标。
class TranslucencyPass final : public RenderPass
{
public:
    explicit TranslucencyPass(ForwardPass& forwardPass)
        : m_ForwardPass(forwardPass)
    {
    }

    RenderPassType GetType() const override
    {
        return RenderPassType::Translucency;
    }
    void Execute(RenderPassContext& context) override;

private:
    ForwardPass& m_ForwardPass;
};
