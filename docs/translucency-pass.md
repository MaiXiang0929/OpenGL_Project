# Translucency Pass

## 管线位置

主视图透明物体按以下顺序进入管线：

```text
ForwardPass（不透明颜色与深度）
    -> TranslucencyPass（透明颜色混合，保留不透明深度）
    -> EditorPrimitivePass
    -> PresentPass
```

首版只渲染主视图透明物体。反射视图仍只消费不透明列表，避免在透明路径稳定前扩大反射目标、排序和 mipmap 生命周期的耦合范围。

## CPU 数据流

`PrimitiveSceneProxy::translucent` 决定物体进入 `RenderView::translucentItems`。`BuildRenderView()` 使用变换后的世界空间包围球中心，并通过当前 `RenderView::view` 转换到观察空间。OpenGL 相机朝向观察空间 `-Z`，因此按 Z 值升序即可得到从后向前的提交顺序。

排序采用 `std::stable_sort`。当两个包围球中心深度相同时保留场景提交顺序，减少相机或浮点误差导致的顺序抖动。该方法仍是物体级近似，无法解决相交透明几何和单个网格内部三角形顺序问题。

## GPU 状态

`TranslucencyPass` 复用 `ForwardPass` 的 PBR Shader、灯光 UBO、阴影和材质绑定：

- 深度测试开启，使透明物体能被不透明几何遮挡。
- 深度写入关闭，避免先绘制的透明层遮挡后续透明层。
- 混合使用 `GL_SRC_ALPHA / GL_ONE_MINUS_SRC_ALPHA`。
- Pass 结束后恢复进入 Pass 前的深度与混合状态。

`MaterialProperties::opacity` 作为面向美术的标量上传给 PBR fragment shader，并被限制到 `[0, 1]`。物体是否进入透明路径仍由 Primitive 的 `translucent` 标志决定，避免仅修改透明度就隐式改变管线分类。

## 资源生命周期

透明 Pass 重新绑定 `ForwardPass` 拥有的颜色/深度 Framebuffer，不创建第二份目标。Forward Pass 绘制完不透明物体后只解绑；透明内容完成后才生成颜色纹理 mipmap，确保 Present 和后续消费者看到完整结果。
