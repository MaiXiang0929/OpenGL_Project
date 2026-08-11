// SPDX-License-Identifier: MIT
#include "RenderPipeline.h"

namespace
{
class CallbackRenderPass final : public RenderPass
{
public:
    explicit CallbackRenderPass(RenderPassType type)
        : m_Type(type)
    {
    }

    RenderPassType GetType() const override
    {
        return m_Type;
    }

    void Execute(RenderPassContext& context) override
    {
        std::function<void()>* callback = nullptr;
        switch (m_Type)
        {
        case RenderPassType::Shadow:
            if (!context.shadowsEnabled) return;
            callback = &context.shadow;
            break;
        case RenderPassType::Reflection:
            callback = &context.reflection;
            break;
        case RenderPassType::Forward:
            callback = &context.forward;
            break;
        case RenderPassType::Present:
            callback = &context.present;
            break;
        }

        if (callback && *callback)
            (*callback)();
    }

private:
    RenderPassType m_Type;
};
}

RenderPipeline::RenderPipeline()
{
    m_Passes.emplace_back(std::make_unique<CallbackRenderPass>(RenderPassType::Shadow));
    m_Passes.emplace_back(std::make_unique<CallbackRenderPass>(RenderPassType::Reflection));
    m_Passes.emplace_back(std::make_unique<CallbackRenderPass>(RenderPassType::Forward));
    m_Passes.emplace_back(std::make_unique<CallbackRenderPass>(RenderPassType::Present));
}

void RenderPipeline::Execute(RenderPassContext& context)
{
    for (const auto& pass : m_Passes)
        pass->Execute(context);
}
