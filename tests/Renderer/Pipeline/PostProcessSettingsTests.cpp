// SPDX-License-Identifier: MIT
#include <cstdlib>
#include <iostream>

#include "Renderer/Pipeline/RenderSettings.h"

namespace
{
void Require(bool condition, const char* message)
{
    if (condition)
        return;

    std::cerr << "[PostProcessSettingsTests] " << message << std::endl;
    std::exit(EXIT_FAILURE);
}
}

int main()
{
    const PostProcessSettings defaults;
    Require(defaults.toneMappingEnabled,
        "Tone mapping should be enabled by default.");
    Require(defaults.exposureCompensation == 0.0f,
        "Default exposure should preserve scene luminance.");
    Require(ClampExposureCompensation(-20.0f) ==
        MinimumExposureCompensation,
        "Exposure should clamp to its lower artist-facing limit.");
    Require(ClampExposureCompensation(20.0f) ==
        MaximumExposureCompensation,
        "Exposure should clamp to its upper artist-facing limit.");
    Require(ClampExposureCompensation(1.5f) == 1.5f,
        "Exposure inside the supported range should remain unchanged.");

    std::cout << "PostProcessSettingsTests passed." << std::endl;
    return EXIT_SUCCESS;
}
