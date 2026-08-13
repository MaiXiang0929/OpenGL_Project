# GPU Pass 时间统计

## 目标与管线位置

`GpuPassProfiler` 在 `RenderPipeline` 的统一 Pass 边界插入 `GL_TIME_ELAPSED` Query：

```text
CPU BeginPass
    -> glBeginQuery(GL_TIME_ELAPSED)
    -> Pass 提交 GPU 命令
    -> glEndQuery(GL_TIME_ELAPSED)
CPU EndPass
```

GPU 异步执行命令并写入纳秒结果。CPU 不在当前帧读取结果，而是在后续复用 Query 槽位前检查 `GL_QUERY_RESULT_AVAILABLE`。

## Query 生命周期

Profiler 使用三帧缓冲，每帧包含六个 Query：

```text
3 buffered frames x 6 passes = 18 query objects
```

Query 由 `RenderPipeline` 中的 `GpuPassProfiler` 拥有。Renderer 会在 GLFW Window 和 OpenGL Context 销毁前析构，因此 Query 创建与删除都发生在有效 Context 生命周期内。

当待复用槽位尚未完成时，Profiler 会跳过当前整帧采样并增加 `skippedFrameCount`，不会调用阻塞式 `GL_QUERY_RESULT`，也不会覆盖仍在 GPU 上执行的 Query。只有确认六个 Query 全部可用后，才读取该帧结果。

## 输出数据

每个 Pass 提供：

- `lastMilliseconds`：最近一次成功解析的 GPU 时间。
- `averageMilliseconds`：平滑系数为 `0.1` 的指数移动平均。
- `valid`：是否至少收到过一个结果。

`GpuTimingSnapshot` 还提供六个 Pass 的时间合计、成功解析帧数与跳过帧数。Renderer 暴露只读 `GetGpuTimingSnapshot()`，可直接供后续 ImGui 性能面板使用。

日志在首次解析和每 120 个成功解析帧后输出，避免逐帧刷屏。首次运行样例：

| Pass | GPU ms |
| --- | ---: |
| Shadow | 1.248 |
| Reflection | 0.111 |
| Forward | 0.111 |
| Translucency | 0.113 |
| EditorPrimitive | 0.814 |
| Present | 0.019 |
| Pass 合计 | 2.416 |

首次结果包含 Shader、纹理和驱动缓存预热影响，只用于验证链路，不作为最终性能结论。应观察 EMA 稳定值并配合 RenderDoc 分析。

## 限制

Pass 合计只包含六个 Query 覆盖的 GPU 命令区间，不包含 CPU 工作、Swap/Present 等待、Pass 之间的驱动间隙，也不等同于显示帧时间。多个 Query 结果相加适合比较 MaiX Renderer 内部 Pass 成本，但不能直接推导 FPS。

Timer Query 初始化失败时渲染仍继续，仅禁用 GPU 时间统计。当前实现不处理平台特定的 disjoint timer 状态；OpenGL 4.0 桌面目标和本项目 NVIDIA 验证环境可直接使用 `GL_TIME_ELAPSED`。
