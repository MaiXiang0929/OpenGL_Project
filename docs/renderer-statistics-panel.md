# Renderer Statistics 面板

## 目标

`Renderer Statistics` 是渲染器的只读观测入口，同时提供少量实时调试参数。面板不访问 `RenderScene`、GPU Resource 或 RenderPass 的私有容器，避免编辑器 UI 与渲染管线形成反向依赖。

## 数据流

```text
RenderScene::BuildRenderView
        -> Renderer::StatisticsSnapshot

RenderPipeline CPU 提交
        -> RenderSubmissionSnapshot

OpenGL Timer Query（三帧缓冲）
        -> GpuTimingSnapshot

上述只读快照 -> RendererStatisticsPanel -> ImGui OpenGL3 Backend
```

CPU 负责在完整帧边界固化场景、提交与 GPU Query 结果；GPU 负责实际 Pass 执行和异步计时。UI 在 `PresentPass` 之后绘制到默认帧缓冲，不会写入 Forward 颜色目标。

## 面板内容

- 主视图源对象、可见对象和裁剪对象数量
- 不透明/透明 Draw 列表及 Shader、Material、Mesh 分组数量
- 唯一 Mesh/Material GPU 资源数量
- 六个 Pass 的 Draw 与 Shader/Mesh/Texture 请求和实际切换次数
- 六个 Pass 的 GPU last/EMA 时间、总时间和 resolved/skipped 帧数
- 阴影、编辑器图元、曲面细分、线框开关
- 曲面细分等级和位移强度

## 输入所有权

Application 继续持有 GLFW 回调，并将事件手动转发给 ImGui。`WantCaptureMouse` 为真时阻断相机和灯光操作，同时仍更新最后鼠标坐标，避免离开面板时视角跳变；`WantCaptureKeyboard` 为真时阻断渲染快捷键，`Esc` 始终保留为退出操作。

## 性能影响

面板读取固定大小快照，不触发 OpenGL 同步读取。GPU 时间来自既有异步 Timer Query，因此不会为了显示统计调用阻塞式查询。ImGui 自身在 Present 后增加一个独立 UI Draw 阶段，该开销不计入六个场景 Pass 的 GPU 总时间。
