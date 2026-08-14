# Shared-Resource Instance Benchmark

## Launch configuration

The benchmark is disabled by default. It can be enabled without changing the
normal material-map positional arguments:

```text
OpenGL_Project.exe --instance-grid 16
OpenGL_Project.exe normal.png displacement.png --instance-grid 8
```

`N` is limited to `1..32`. The default scene still submits one opaque teapot.

## CPU ownership and layout

`ApplicationOptions` parses the launch configuration before OpenGL starts.
`InstanceGrid` produces a centered X/Y grid and a conservative scene radius.
Application uploads one teapot `Mesh` and one `Material`, then submits one
`PrimitiveSceneProxy` per transform with the same handles.

```text
CLI grid size
    -> centered CPU transforms
    -> one shared MeshHandle + MaterialHandle
    -> N * N PrimitiveSceneProxy objects
    -> per-view culling and RenderItem sorting
    -> current one-draw-per-item GPU path
```

The benchmark scene radius drives camera distance, the camera far plane, ground
extent, and the shadow projection. This keeps large grids measurable without
disabling the production culling path.

## Baseline results

Environment: OpenGL 4.0, NVIDIA GeForce RTX 4060 Ti, 1920x1080 Debug build.
Each row is a separate five-second run. GPU values are the final EMA snapshot,
so they are directional rather than a statistically rigorous performance test.

| Grid | Opaque instances | Mesh resources | Material resources | Shadow draws | Reflection draws | Forward draws | GPU frame EMA |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 1x1 | 1 | 2 | 4 | 1 | 5 | 3 | 0.747 ms |
| 8x8 | 64 | 2 | 4 | 64 | 25 | 66 | 0.942 ms |
| 16x16 | 256 | 2 | 4 | 256 | 65 | 258 | 1.386 ms |

The resource counts include the separate translucent test mesh and its three
materials. They remain constant at every grid size, proving that the opaque
instances share their GPU resources. Reflection draw counts are lower because
that view performs independent frustum culling.

At 16x16, Forward records:

```text
material requests/changes = 256/1
mesh requests/changes     = 258/3
texture requests/changes  = 1030/9
```

The stable resource ordering is effective, but the renderer still submits one
draw and repeats material uniform/texture work for every opaque instance.

## RenderDoc capture

The local capture is `captures/maix_instances_16_frame146.rdc` (about 95.9 MiB).
It is excluded from source control. `renderdoccmd thumb` successfully replayed
the capture, and the thumbnail confirms all 259 scene primitives visible in the
main view plus the independently culled reflection result.

## Optimization decision

The next optimization should batch contiguous opaque items with the same
Shader, Material, and Mesh into an instanced draw in Forward, Shadow, and
Reflection views. Per-instance model transforms should move to an instance
vertex buffer or equivalent GPU input.

A standalone texture cache is deferred. Instancing will collapse most of the
redundant material and texture requests in the measured shared-resource case,
so texture caching should be evaluated again after the instanced baseline.
