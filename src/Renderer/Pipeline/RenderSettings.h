// SPDX-License-Identifier: MIT
#pragma once

struct TessellationSettings
{
    bool enabled = false;
    float level = 8.0f;
    float displacementScale = 0.35f;
    bool wireframe = false;
};

struct PostProcessSettings
{
    bool toneMappingEnabled = true;
    float exposureCompensation = 0.0f;
};

constexpr float MinimumExposureCompensation = -8.0f;
constexpr float MaximumExposureCompensation = 8.0f;

constexpr float ClampExposureCompensation(float exposure)
{
    return exposure < MinimumExposureCompensation
        ? MinimumExposureCompensation
        : exposure > MaximumExposureCompensation
            ? MaximumExposureCompensation
            : exposure;
}
