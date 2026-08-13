# RenderDoc 与渲染提交基线

## 环境与捕获

基线采集环境：

- OpenGL 4.0 Core Profile
- NVIDIA GeForce RTX 4060 Ti
- RenderDoc 命令行捕获，启用 API Validation
- 场景：1 个不透明茶壶、3 个共享 Mesh 的透明平面、3 盏灯、反射地面

本地捕获文件为 `captures/maix_baseline_frame2749.rdc`，大小约 78 MiB。`.rdc` 属于机器生成的大文件，由 `captures/.gitignore` 排除，不进入源码版本控制。捕获已通过 `renderdoccmd thumb` 成功解析，缩略图确认茶壶、透明层、反射和灯光 Gizmo 均正常。

## GPU Pass 标记

`RenderPipeline` 使用可选 `GpuDebugScope` 为每个 Pass 写入以下调试组：

```text
MaiX.ShadowPass
MaiX.ReflectionPass
MaiX.ForwardPass
MaiX.TranslucencyPass
MaiX.EditorPrimitivePass
MaiX.PresentPass
```

项目仍以 OpenGL 4.0 为最低版本。`GpuDebugScope` 在运行时查询 `glPushDebugGroup` / `glPopDebugGroup` 及 KHR 后缀版本；驱动不支持时退化为空操作，不改变渲染流程。

## CPU 提交基线

日志格式中的 `请求/变化` 表示 CPU 发起的绑定请求数，以及相邻提交真正需要切换资源的次数。首次测量结果：

| Pass | Draw | Shader | Material | Mesh | Texture |
| --- | ---: | ---: | ---: | ---: | ---: |
| Shadow | 1 | 1/1 | 0/0 | 1/1 | 0/0 |
| Reflection | 2 | 2/2 | 1/1 | 2/2 | 7/7 |
| Forward | 3 | 3/3 | 1/1 | 3/3 | 10/9 |
| Translucency | 3 | 1/1 | 3/3 | 3/1 | 2/2 |
| EditorPrimitive | 3 | 3/3 | 0/0 | 3/3 | 0/0 |
| Present | 1 | 1/1 | 0/0 | 1/1 | 1/1 |

Forward 与 Translucency 都复用同一个 PBR Shader，但它们是独立 Pass，因此缓存边界会在 Pass 开始时失效。Forward 中天空盒、反射地面和不透明表面使用不同 Shader/Mesh，切换是必要的。

## 优化决策

透明 Pass 的 `mesh=3/1` 是当前最明确的冗余：三张透明平面共享同一 VAO，三次 Draw 只有第一次需要绑定。基于该证据实现最小 `OpenGLStateCache`：

- 只缓存当前 VAO。
- 每个 Pass 开始时失效，避免与未经过缓存的 OpenGL 调用产生错误假设。
- `Mesh::Draw()` 与 Editor Primitive 的 VAO 提交统一经过缓存。
- 不再在每次 Mesh Draw 后绑定 VAO 0。

优化后透明 Pass 保持 3 个 Draw，但实际只执行 1 次 VAO 绑定。Shader 请求基本都是一次请求对应一次变化；纹理仅有少量重复，当前场景收益不足以证明引入更广泛状态缓存的复杂度，因此暂不缓存 Program 和 Texture。

## 性能与限制

Recorder 只在 CPU 侧统计 Renderer 已接入的提交入口，不等同于 GPU 时间。它用于定位冗余 API 调用和验证排序效果；Pass GPU 时长、驱动内部开销和带宽瓶颈仍应以 RenderDoc Event Browser、Pipeline State 和 GPU Counter 为准。

当前日志在首帧或统计变化时输出，不逐帧刷屏。未来若引入 ImGui 性能面板，可以直接消费同一份 `PassSubmissionStats`，但需要先将只读快照接口从 Recorder 中公开。
