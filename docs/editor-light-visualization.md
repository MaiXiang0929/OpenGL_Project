# Editor Light Visualization

## 目标

使用独立编辑器渲染阶段显示光源位置与影响范围，同时保持场景光照、阴影和反射结果不受辅助图形影响。

## 管线位置

```text
ShadowPass
ReflectionPass
ForwardPass
EditorPrimitivePass
PresentPass
```

`EditorPrimitivePass` 在主场景渲染完成后写入 Forward 场景颜色目标。辅助图形因此会显示在最终 Scene Viewport 中，但不会进入 Shadow Map、Reflection Texture 或 PBR 光照计算。

## 可视化内容

- Point/Spot Light：使用固定屏幕像素尺寸的 Billboard 标记世界空间位置。
- Spot Light：额外绘制线框圆锥，表示方向、范围和外锥角。
- Directional Light：当前不绘制位置 Billboard；后续应使用方向箭头表达其无位置属性的语义。

Billboard 在裁剪空间展开，不随观察距离改变屏幕尺寸。Spot Light 圆锥使用静态单位线框网格，GPU 根据 `position`、`direction`、`range` 和 `outerConeAngle` 转换到世界空间。

## 数据流

```text
Application 更新 LightSceneProxy
        ↓
RenderScene::BuildRenderView
        ↓
RenderView::lights
        ↓
EditorPrimitivePass
        ↓
Forward framebuffer
        ↓
PresentPass
```

CPU 负责维护灯光代理、选择可视化类型并提交参数；GPU 负责 Billboard 展开、圆锥定向以及辅助图元光栅化。

## 渲染状态

- 默认关闭深度测试和深度写入，确保光源辅助图形始终可见。
- 启用 Alpha Blend，用于 Billboard 边缘和半透明圆锥线框。
- Pass 完成后恢复进入 Pass 前的深度测试、深度写入和混合状态。
- `L` 键控制整个 `EditorPrimitivePass` 的显示，不影响实际灯光与阴影。

## 资源所有权

`EditorPrimitivePass` 持有 Billboard/线框 Shader 以及对应 VAO、VBO，并负责初始化和清理。`Application` 不再直接提交编辑器 OpenGL 绘制命令，Renderer 与 Pipeline 统一管理场景视口的渲染顺序。

## 扩展方向

- 为 Directional Light 增加方向箭头。
- 为 Point Light 增加影响范围线框球。
- 增加深度感知与始终可见两种显示模式。
- 增加 Picking ID，使 Billboard 和线框支持编辑器选择。
