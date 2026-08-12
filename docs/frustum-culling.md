# Frustum Culling

## 目标

在提交 GPU Draw Call 前剔除视锥外的图元，降低无效顶点处理、光栅化和材质状态切换成本。裁剪在 CPU 的 `RenderScene::BuildRenderView()` 阶段完成，不改变 GPU Shader。

## 数据流

```text
PrimitiveSceneProxy
    ↓ Local Bounds + LocalToWorld
World-Space Bounding Sphere
    ↓ Frustum Six-Plane Test
Visible RenderItem
    ↓
Render Pass Draw List
```

`PrimitiveSceneProxy` 保存本地空间包围球。每次构建 View 时，球心通过完整 `localToWorld` 矩阵转换到世界空间；半径乘以三个变换基轴长度的最大值，以保守支持非均匀缩放。

## 视锥构建

`Frustum::FromViewProjection()` 从 OpenGL View-Projection 矩阵提取 Left、Right、Bottom、Top、Near 和 Far 六个平面。平面归一化后，通过下式测试世界空间包围球：

```text
dot(plane.normal, sphere.center) + plane.distance < -sphere.radius
```

任意平面满足该条件时，包围球完全位于视锥外并被剔除。与平面相交的图元保留，退化平面和无效 Bounds 采用保守可见策略，避免错误剔除。

## 独立 Render View

每帧构建三份可见性结果：

| View | View-Projection | 消费者 |
| --- | --- | --- |
| Main | `projection * view` | ForwardPass、EditorPrimitivePass |
| Reflection | `projection * reflectionView` | ReflectionPass |
| Shadow | `lightVP` | ShadowPass |

独立列表避免主相机裁剪错误影响反射与阴影。例如，主相机外的物体仍可能出现在镜面中，或向主场景投射可见阴影。

## 统计与性能

每个 `RenderView` 记录源图元数、可见图元数和剔除图元数，为后续编辑器统计面板与 RenderDoc 分析提供数据。当前实现每帧对三个 View 分别线性遍历场景，适用于现阶段规模。

后续扩展包括：

- 缓存世界空间 Bounds，仅在 Transform 变化时更新。
- 使用 AABB、OBB 或分层包围体提高精度。
- 引入 BVH、Octree 等空间结构降低大场景遍历成本。
- 对灯光影响范围进行裁剪。
- 增加遮挡裁剪与距离裁剪。

## 验证

CPU 测试覆盖单位视锥六个平面、平面相交保留、透视投影、相机后方剔除、无效 Bounds 保守处理，以及平移和非均匀缩放后的包围球转换。测试不依赖 OpenGL Context。
