// SPDX-License-Identifier: MIT
#pragma once

#include <memory>
#include <vector>

#include "RenderPass.h"

class RenderPipeline
{
public:
    RenderPipeline();

    void Execute(RenderPassContext& context);

    const std::vector<std::unique_ptr<RenderPass>>& GetPasses() const
    {
        return m_Passes;
    }

private:
    std::vector<std::unique_ptr<RenderPass>> m_Passes;
};
