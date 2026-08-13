// SPDX-License-Identifier: MIT
#include <cstdlib>
#include <iostream>

#include "Renderer/Pipeline/RenderTargetSizing.h"

namespace
{
void Require(bool condition, const char* message)
{
    if (condition)
        return;

    std::cerr << "[RenderTargetSizingTests] " << message << std::endl;
    std::exit(EXIT_FAILURE);
}
}

int main()
{
    const RenderTargetExtent hd =
        CalculateReflectionTargetExtent(1920, 1080);
    Require(hd.width == 960 && hd.height == 540,
        "Even viewport dimensions should halve exactly.");

    const RenderTargetExtent odd =
        CalculateReflectionTargetExtent(1279, 719);
    Require(odd.width == 640 && odd.height == 360,
        "Odd viewport dimensions should round up independently.");

    const RenderTargetExtent minimum =
        CalculateReflectionTargetExtent(1, 1);
    Require(minimum.width == 1 && minimum.height == 1,
        "A valid viewport must produce a non-zero reflection target.");

    const RenderTargetExtent minimized =
        CalculateReflectionTargetExtent(0, 1080);
    Require(minimized.width == 0 && minimized.height == 0,
        "A minimized viewport should not request a render target.");

    std::cout << "RenderTargetSizingTests passed." << std::endl;
    return EXIT_SUCCESS;
}
