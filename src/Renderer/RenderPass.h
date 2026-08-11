// SPDX-License-Identifier: MIT
#pragma once

#include <functional>

enum class RenderPassType
{
    Shadow,
    Reflection,
    Forward,
    Present
};

struct RenderPassContext
{
    std::function<void()> shadow;
    std::function<void()> reflection;
    std::function<void()> forward;
    std::function<void()> present;
    bool shadowsEnabled = true;
};

class RenderPass
{
public:
    virtual ~RenderPass() = default;
    virtual RenderPassType GetType() const = 0;
    virtual void Execute(RenderPassContext& context) = 0;
};
