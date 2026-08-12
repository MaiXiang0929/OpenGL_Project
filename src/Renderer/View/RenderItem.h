// SPDX-License-Identifier: MIT
#pragma once

#include "cyMatrix.h"
#include "Renderer/Scene/PrimitiveSceneProxy.h"

class Material;
class Mesh;

struct RenderItem
{
    PrimitiveId primitiveId = InvalidPrimitiveId;
    Mesh* mesh = nullptr;
    Material* material = nullptr;
    RenderResourceId shaderId = DefaultSurfaceShaderId;
    RenderResourceId materialId = InvalidRenderResourceId;
    RenderResourceId meshId = InvalidRenderResourceId;
    cy::Matrix4f model = cy::Matrix4f::Identity();
    bool castsShadow = true;
};
