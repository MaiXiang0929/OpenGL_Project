# 不透明物体渲染排序

## 管线位置

不透明物体排序在 CPU 侧的 `RenderScene::BuildRenderView()` 中执行，位于可见性测试和不透明/透明分类之后。每个视图分别持有排序后的 `RenderItem` 列表，因此主视图、反射和阴影 Pass 可以直接使用确定的提交顺序，无需在各自内部重复排序。

GPU 仍负责顶点处理、光栅化、深度测试、材质着色和纹理采样。排序只改变绘制命令的提交顺序，不会改变 Transform、坐标空间或 Shader 数学运算。

## 稳定资源键

`PrimitiveSceneProxy` 和 `RenderItem` 保存由 Renderer 分配的 `shaderId`、`materialId` 与 `meshId`。这些 ID 在对应资源的生命周期内保持稳定，避免使用非 owning 指针地址决定排序顺序。

当前 `Renderer::AddPrimitive()` 会为每个 Primitive 分别创建一个 Mesh 和一个 Material，因此现阶段这些 ID 各自唯一。排序键的数据布局已经为后续资源共享和材质实例预留扩展能力，无需修改 RenderView 契约。

## 视图排序策略

主视图和反射视图使用：

```text
Shader -> Material -> Mesh -> Primitive
```

该顺序使使用相同材质状态的物体在主视图和反射 Pass 中相邻提交。

阴影视图使用：

```text
Shader -> Mesh -> Material -> Primitive
```

标准阴影路径不绑定表面材质纹理。Material 仍保留在排序键中，因为曲面细分阴影路径需要使用位移纹理。

透明物体不会参与该排序。后续需要在 `TranslucencyPass` 中根据视图空间深度执行从后向前排序。

## 运行时统计

主视图会输出场景源对象数、可见对象数、被裁剪对象数、不透明 Draw 数，以及 Shader、Material 和 Mesh 分组数。只有统计值发生变化时才输出日志。分组数用于描述 CPU 绘制提交的相邻关系，不代表 GPU 时间，也不能证明所有重复的 OpenGL 绑定调用都已消除。

## 性能考量

每个视图在完成裁剪后执行排序，时间复杂度为 `O(n log n)`。当前 Renderer 会分别构建主视图、反射视图和阴影视图，因此在继续增加视图类型之前，应根据场景规模分析重复遍历与排序的 CPU 成本。后续需要使用 RenderDoc，将资源分组统计与实际发生的 Shader Program、Texture 和 VAO 状态切换进行对照验证。
