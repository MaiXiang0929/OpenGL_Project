# Render Target Resize

## Ownership and data flow

`Application` owns the GLFW framebuffer size and writes it to
`RenderFrameData::viewportWidth/viewportHeight`. It does not resize OpenGL
resources directly.

Before executing any pass, `RenderPipeline` compares that extent with the
targets owned by `ForwardPass` and `ReflectionPass`:

```text
GLFW framebuffer callback (CPU)
    -> Application camera aspect ratios (CPU)
    -> RenderFrameData viewport extent (CPU)
    -> RenderPipeline extent comparison (CPU)
    -> Framebuffer color/depth attachment recreation (GPU resource)
    -> each pass binds its target and viewport (GPU state)
    -> PresentPass restores the window viewport
```

The Forward target matches the window framebuffer. The Reflection target uses
half resolution in each dimension, rounded up for odd dimensions. This keeps
the same aspect ratio while reducing reflection color/depth memory and pixel
shader cost to approximately one quarter of the main target.

## Lifetime rules

- Targets are created lazily on the first frame with a valid non-zero extent.
- A target is recreated only when its required extent changes.
- A zero width or height represents a minimized window. The pipeline skips the
  frame without deleting the last valid GPU resources.
- `Framebuffer::Init` rejects zero dimensions and reports FBO completeness to
  its owning pass.
- Main and reflection views share the main camera projection, so both targets
  keep the same aspect ratio. The presentation plane is scaled to that aspect
  before projection to avoid distorting the rendered texture.

## Current limitation

Resize recreates attachments immediately on the render thread. Continuous
window dragging can therefore cause repeated allocations. This is acceptable
for the current renderer; a future render graph can pool targets or debounce
resize events if profiling shows allocation stalls.
