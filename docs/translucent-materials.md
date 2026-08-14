# Translucent Materials

## Material and queue ownership

`Material::BlendMode` is the artist-facing surface classification. When a
primitive is submitted, `Renderer` resolves that mode into its
`PrimitiveSceneProxy`. `RenderScene::BuildRenderView()` then places the proxy
in either the opaque or translucent queue without inspecting OpenGL state.

Only `Opaque` and `AlphaBlend` are supported. Masked and additive materials
remain separate future features because they require different depth, shadow,
and blend policies.

## CPU data flow

```text
Material::BlendMode
    -> Renderer::AddPrimitive
    -> PrimitiveSceneProxy::blendMode
    -> RenderScene::BuildRenderView
    -> RenderView::opaqueItems / translucentItems
```

Each surface view calculates translucent sort depth by transforming the world
space bounds center with that view's matrix. Main and reflected views therefore
sort independently. OpenGL view space looks down `-Z`, so ascending view-space
Z produces back-to-front order.

## GPU data flow

The PBR fragment shader samples the albedo texture once as RGBA. Hardware sRGB
decode affects RGB but leaves alpha linear. RGB feeds the Cook-Torrance BRDF,
while output alpha is:

```glsl
clamp(material.opacity * albedoSample.a, 0.0, 1.0)
```

`TranslucencyPass` keeps depth testing enabled, disables depth writes, and uses
straight-alpha blending with `GL_SRC_ALPHA / GL_ONE_MINUS_SRC_ALPHA`. Its
`RenderToBoundTarget()` operation is reused by `ReflectionPass`, after opaque
reflection draws and before reflection mipmap generation. No additional render
target is allocated.

## Validation and limits

The runtime test scene shares one mesh and one 2x2 RGBA alpha checker texture
across three independently colored materials. Unit tests cover opaque versus
alpha-blended queue classification, stable main-view sorting, and independent
reflected-view sorting.

A five-second Debug runtime diagnostic produced three main translucency draws
and five reflection draws (skybox, opaque surface, and three translucent
layers), with no stderr output or skipped GPU timing queries.

Object-center sorting cannot correctly solve intersecting translucent meshes or
triangle order inside one mesh. Alpha-cutout shadows, translucent shadow
casting, order-independent transparency, and additive blending are not part of
this implementation.
