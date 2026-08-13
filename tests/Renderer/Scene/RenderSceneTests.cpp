// SPDX-License-Identifier: MIT
#include <cstdint>
#include <cstdlib>
#include <iostream>

#include "Renderer/Scene/RenderScene.h"
#include "Renderer/View/RenderView.h"

namespace
{
void Require(bool condition, const char* message)
{
    if (condition)
        return;

    std::cerr << "[RenderSceneTests] " << message << std::endl;
    std::exit(EXIT_FAILURE);
}

PrimitiveSceneProxy MakePrimitive(
    RenderResourceId shaderId,
    RenderResourceId materialId,
    RenderResourceId meshId,
    const cy::Vec3f& center = cy::Vec3f(0.0f, 0.0f, 0.0f),
    bool translucent = false)
{
    PrimitiveSceneProxy proxy;
    proxy.mesh = reinterpret_cast<Mesh*>(static_cast<std::uintptr_t>(1));
    proxy.material =
        reinterpret_cast<Material*>(static_cast<std::uintptr_t>(1));
    proxy.shaderId = shaderId;
    proxy.materialId = materialId;
    proxy.meshId = meshId;
    proxy.localBounds.center = center;
    proxy.localBounds.radius = 0.25f;
    proxy.translucent = translucent;
    return proxy;
}

RenderScene MakeScene()
{
    RenderScene scene;
    scene.AddPrimitive(MakePrimitive(1, 2, 1));
    scene.AddPrimitive(MakePrimitive(0, 2, 3));
    scene.AddPrimitive(MakePrimitive(0, 1, 5));
    scene.AddPrimitive(MakePrimitive(0, 1, 2));
    scene.AddPrimitive(MakePrimitive(0, 0, 0, cy::Vec3f(), true));
    scene.AddPrimitive(MakePrimitive(0, 0, 0, cy::Vec3f(5.0f, 0.0f, 0.0f)));
    return scene;
}

void RequireOrder(
    const std::vector<RenderItem>& items,
    PrimitiveId first,
    PrimitiveId second,
    PrimitiveId third,
    PrimitiveId fourth)
{
    Require(items.size() == 4, "Expected four visible opaque items.");
    Require(
        items[0].primitiveId == first &&
        items[1].primitiveId == second &&
        items[2].primitiveId == third &&
        items[3].primitiveId == fourth,
        "Opaque item order does not match the view sort policy.");
}

void TestSurfaceViewSortingAndStats()
{
    RenderScene scene = MakeScene();
    RenderView view;
    view.type = RenderViewType::Main;
    view.frustum = Frustum::FromViewProjection(cy::Matrix4f::Identity());

    scene.BuildRenderView(view);

    RequireOrder(view.opaqueItems, 3, 2, 1, 0);
    Require(view.translucentItems.size() == 1,
        "Translucent items should remain in their separate list.");
    Require(view.translucentItems.front().primitiveId == 4,
        "Opaque sorting must not reorder the translucent list.");
    Require(view.sourcePrimitiveCount == 6,
        "Source primitive count should include every proxy.");
    Require(view.visiblePrimitiveCount == 5,
        "Visible count should include opaque and translucent items.");
    Require(view.culledPrimitiveCount == 1,
        "Culled count should include the out-of-frustum primitive.");
    Require(view.opaqueDrawCount == 4,
        "Opaque draw count should match the sorted list size.");
    Require(view.opaqueShaderGroupCount == 2,
        "Shader group count should track shader transitions.");
    Require(view.opaqueMaterialGroupCount == 3,
        "Material group count should include shader transitions.");
    Require(view.opaqueMeshGroupCount == 4,
        "Mesh group count should track sorted mesh transitions.");

    scene.BuildRenderView(view);
    RequireOrder(view.opaqueItems, 3, 2, 1, 0);
}

void TestReflectionUsesSurfaceSortPolicy()
{
    RenderScene scene = MakeScene();
    RenderView view;
    view.type = RenderViewType::Reflection;
    view.frustumCullingEnabled = false;

    scene.BuildRenderView(view);

    Require(view.opaqueItems.size() == 5,
        "Disabling culling should retain every opaque primitive.");
    Require(
        view.opaqueItems[0].primitiveId == 5 &&
        view.opaqueItems[1].primitiveId == 3 &&
        view.opaqueItems[2].primitiveId == 2 &&
        view.opaqueItems[3].primitiveId == 1 &&
        view.opaqueItems[4].primitiveId == 0,
        "Reflection view should use the surface material-first policy.");
}

void TestShadowViewSorting()
{
    RenderScene scene = MakeScene();
    RenderView view;
    view.type = RenderViewType::Shadow;
    view.frustum = Frustum::FromViewProjection(cy::Matrix4f::Identity());

    scene.BuildRenderView(view);

    RequireOrder(view.opaqueItems, 3, 1, 2, 0);
}

void TestTranslucentBackToFrontStableSorting()
{
    RenderScene scene;
    const PrimitiveId nearId = scene.AddPrimitive(MakePrimitive(
        0, 0, 0, cy::Vec3f(0.0f, 0.0f, -2.0f), true));
    const PrimitiveId farFirstId = scene.AddPrimitive(MakePrimitive(
        0, 0, 0, cy::Vec3f(0.0f, 0.0f, -8.0f), true));
    const PrimitiveId farSecondId = scene.AddPrimitive(MakePrimitive(
        0, 0, 0, cy::Vec3f(1.0f, 0.0f, -8.0f), true));

    RenderView view;
    view.type = RenderViewType::Main;
    view.frustumCullingEnabled = false;
    scene.BuildRenderView(view);

    Require(view.opaqueItems.empty(),
        "A translucent-only scene must not produce opaque items.");
    Require(view.translucentItems.size() == 3,
        "Expected three translucent render items.");
    Require(
        view.translucentItems[0].primitiveId == farFirstId &&
        view.translucentItems[1].primitiveId == farSecondId &&
        view.translucentItems[2].primitiveId == nearId,
        "Translucent items must sort back-to-front and remain stable at equal depth.");
    Require(
        view.translucentItems[0].sortDepth == -8.0f &&
        view.translucentItems[2].sortDepth == -2.0f,
        "Translucent sort depth should use the world bounds center in view space.");
}
}

int main()
{
    TestSurfaceViewSortingAndStats();
    TestReflectionUsesSurfaceSortPolicy();
    TestShadowViewSorting();
    TestTranslucentBackToFrontStableSorting();
    std::cout << "RenderScene tests passed." << std::endl;
    return EXIT_SUCCESS;
}
