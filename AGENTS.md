# MaiX Renderer：现代实时光栅化渲染器路线图

## 1. 项目定位

MaiX Renderer 是一个基于 C++17 与 OpenGL 4.0 Core Profile 的轻量级实时光栅化渲染框架。项目目标不是只完成一个展示 Demo，而是逐步建立可解释、可扩展、适合技术美术工作流的渲染架构，重点覆盖：

- GPU 光栅化流程与资源管理
- Cook-Torrance PBR 与 GLSL Shader 开发
- 多 Pass Render Pipeline
- 材质参数和纹理工作流
- NPR/Anime 渲染扩展
- RenderDoc 性能分析

目标岗位方向：渲染技术美术、Shader TA、引擎技术美术。

## 2. 总体架构

```text
Application
  ├─ Window / Input / Camera
  ├─ 游戏侧场景状态
  └─ 每帧提交 RenderFrameData
           │
           ▼
Renderer（渲染资源所有者与管线入口）
  ├─ RenderScene（渲染侧场景代理）
  │   ├─ PrimitiveSceneProxy
  │   └─ LightSceneProxy
  ├─ GPU 资源
  │   ├─ Mesh / Material / Shader / Texture
  │   ├─ Framebuffer
  │   └─ ShadowMap / Cubemap
  └─ RenderPipeline
      ├─ ShadowPass
      ├─ ReflectionPass
      ├─ ForwardPass
      └─ PresentPass
           │
           ▼
        OpenGL GPU
```

### 2.1 当前目录结构

```text
src/
├─ Core/                 应用生命周期、窗口、输入、相机
├─ Editor/               灯光调试 Gizmo 等编辑器叠加层
├─ Renderer/
│  ├─ Core/              Renderer 门面与 GPU 资源所有权
│  ├─ Pipeline/          RenderPipeline、RenderPass 契约、RenderSettings
│  ├─ Passes/            Shadow、Reflection、Forward、Present
│  ├─ Resources/         Mesh、Material、Shader、Texture、Framebuffer
│  ├─ Scene/             RenderScene 与 Scene Proxy
│  └─ View/              RenderView 与 RenderItem
└─ main.cpp

assets/shaders/
├─ pbr/                  标准 PBR 与曲面细分/位移 Shader
├─ shadow/               阴影深度 Shader
├─ environment/         Skybox Shader
├─ reflection/          反射地面 Shader
├─ present/              离屏结果显示 Shader
└─ debug/                灯光 Gizmo 等调试 Shader
```

## 3. 每帧渲染数据流

```text
逻辑场景对象
    │ Application 更新 Transform / Light
    ▼
RenderScene
    │ BuildRenderView()
    ▼
RenderView
    ├─ opaqueItems
    ├─ translucentItems
    └─ lights
    │
    ├─ ShadowPass：写入 shadow map
    ├─ ReflectionPass：生成反射纹理
    ├─ ForwardPass：写入 HDR/离屏颜色目标
    └─ PresentPass：将离屏结果绘制到窗口
```

CPU 负责场景代理、可见项列表、矩阵和资源绑定准备；GPU 负责顶点变换、三角形光栅化、深度测试、纹理采样、BRDF 光照和最终像素输出。当前每个 `RenderItem` 在 Pass 内独立计算 `model`、`MV`、`MVP` 与 `lightMVP`，为后续裁剪、排序和批处理保留扩展点。

## 4. 当前进度（2026-08）

### 已完成

- [x] Renderer 目录按 Core / Pipeline / Passes / Resources / Scene / View 拆分
- [x] Shader 目录按渲染用途拆分
- [x] `RenderPipeline` 按固定顺序执行 Shadow、Reflection、Forward、EditorPrimitive、Present
- [x] `RenderScene`、`PrimitiveSceneProxy`、`LightSceneProxy` 已建立
- [x] `RenderView`、`RenderItem` 已建立，支持 opaque/translucent 分类
- [x] Renderer 持有 Primitive 的 Mesh 与 Material，Scene Proxy 使用非 owning 指针
- [x] Primitive Proxy 已具备局部空间包围球，并能根据 `localToWorld` 计算保守的世界空间包围球
- [x] `BuildRenderView` 已对主视图、反射视图和阴影视图执行包围球视锥体裁剪，并记录源对象、可见对象和被裁剪对象数量
- [x] 视锥平面提取、球体相交和变换后包围球已有独立 `FrustumTests`，并在 `docs/frustum-culling.md` 记录数据流与限制
- [x] Primitive/RenderItem 已具备稳定 Shader、Material、Mesh 排序 ID；主/反射视图按 Shader / Material / Mesh 排序，阴影视图按 Shader / Mesh / Material 排序
- [x] 不透明排序已有多 Primitive 的 `RenderSceneTests`，主视图运行时输出 Draw 与资源分组统计，排序策略记录于 `docs/opaque-render-sorting.md`
- [x] Forward fragment shader 从 Blinn-Phong 升级为 Cook-Torrance BRDF
- [x] PBR metallic / roughness / AO 参数接入 Material
- [x] Albedo、Normal、Specular、Displacement 纹理接口保留
- [x] 标准 PBR 与曲面细分/位移路径可切换
- [x] Directional/Spot 阴影基础流程与 PCF
- [x] Cubemap、反射地面、离屏 Framebuffer、Present 流程
- [x] Light Gizmo 已迁移为独立 `EditorPrimitivePass`，在 Present 前写入 Forward 颜色目标
- [x] CMake 构建时清理并复制最新 assets
- [x] CMake 配置、编译、链接通过；本阶段未启动可执行文件

### 部分完成

- [ ] RenderView 已分类透明物体，但尚无独立 `TranslucencyPass`
- [ ] 场景支持多灯光代理，但当前 PBR Shader 只消费第一盏灯
- [ ] RenderPipeline 已具备 Pass 边界，但资源依赖仍主要通过共享 Frame Context 传递
- [ ] Forward 使用离屏颜色目标，但 HDR、曝光和色调映射尚未独立实现
- [ ] 视锥体裁剪已完成包围球粗裁剪，但遮挡裁剪、距离裁剪和更精确的包围体尚未实现

### 尚未开始

- [ ] 不透明排序已完成，但共享 Mesh/Material、状态缓存和批处理尚未实现
- [ ] 真正的 TranslucencyPass 与透明排序
- [ ] HDR 合成、Bloom、Tone Mapping、SSAO
- [ ] Cascaded Shadow Maps（CSM）
- [ ] NPR Anime Shader：Toon、Face Shadow、Outline、Rim Light、Hair Highlight
- [ ] ImGui Scene Window、Inspector、Material Editor
- [ ] RenderDoc 性能采集和 GPU Pass 统计文档

## 5. 分阶段实施计划

### 阶段一：材质与 PBR 基础（当前阶段，最高优先级）

目标：完成可复用 Material 工作流和可信的 Cook-Torrance 光照。

1. 统一 `MaterialProperties` 与纹理槽位。
2. 完善 base color、metallic、roughness、AO、normal map 的 Shader 输入。
3. 保持标准网格与曲面细分网格共用 PBR 光照模型。
4. 增加典型金属、塑料、陶瓷、粗糙表面测试材质。
5. 记录坐标空间、BRDF 数学和性能开销。

验收：材质参数改变会实时影响外观，法线/位移/曲面细分不回归。

### 阶段二：RenderScene 与可见性

目标：让渲染器从“固定单物体 Demo”进入多物体场景阶段。

1. 为 Primitive Proxy 补充世界空间包围球/包围盒。
2. 在 `BuildRenderView` 中加入视锥体裁剪。
3. 增加不透明物体排序，减少材质和 Shader 切换。
4. 设计资源句柄或稳定 ID，逐步减少裸指针边界。
5. 增加多物体和多灯光验证场景。

验收：不可见物体不提交 Draw Call，物体数量增加时管线仍可维护。

### 阶段三：阴影与后处理

目标：形成现代实时渲染中常见的 HDR 场景链路。

1. 将 ShadowMap 资源和 ShadowPass 生命周期显式化。
2. 实现 CSM，处理级联划分、稳定投影和级联混合。
3. 建立 HDR color target、亮度提取和 Bloom pass。
4. 增加曝光与 Tone Mapping。
5. 增加 SSAO，并定义其与主光照的组合方式。

验收：明暗范围稳定，Bloom/SSAO 可独立开关，RenderDoc 能看到清晰 Pass 边界。

### 阶段四：透明与材质表现扩展

目标：补齐透明渲染并为艺术表现提供可切换路径。

1. 实现 `TranslucencyPass` 和透明物体深度/排序策略。
2. 增加 PBR 材质实例和参数覆盖。
3. 增加 NPR 管线入口。
4. 实现 Toon Lighting、Face Shadow、Outline、Rim Light、Hair Highlight。
5. 允许 PBR / NPR 按材质或视图模式切换。

验收：透明物体顺序正确，二次元材质参数可由工具实时修改。

### 阶段五：编辑器与性能分析

目标：形成技术美术可操作、可观察的工作流。

1. 集成 ImGui。
2. 实现 Scene Window、Inspector、Material Editor。
3. 暴露 Transform、Light、Base Color、Metallic、Roughness、Texture 等参数。
4. 增加 GPU debug label、Pass 时间和 Draw Call 统计。
5. 使用 RenderDoc 记录基线、瓶颈和优化结果。

验收：运行时调整参数立即反映到画面，性能数据可复现并有优化前后对比。

## 6. 近期下一步

下一次实现前仍遵循“先给方案、确认后执行”的协作流程。建议优先顺序为：

1. 将当前只消费第一盏灯的 PBR 接口演进为可扩展的多灯光 GPU 数据提交，并定义灯光数量上限与缓冲布局。
2. 实现独立 `TranslucencyPass`，补齐从后向前排序、混合状态和深度写入策略。
3. 让多个 Primitive 可共享 Mesh/Material 资源，并基于 RenderDoc 结果决定是否增加跨 Draw 的 OpenGL 状态缓存或批处理。
4. 将视图统计接入后续 ImGui/性能面板，并增加 GPU Pass 时间与真实状态切换基线。

## 7. 当前技术债与约束

- OpenGL 资源仍由各 Resource 类直接管理，尚未抽象成跨 API RHI。
- Renderer 通过 `unique_ptr` 持有 Primitive 的 Mesh 与 Material，但 Scene Proxy 和 `RenderItem` 仍使用非 owning 裸指针；同时 Renderer 尚未公开 Primitive/Light 移除接口，资源生命周期边界仍需收紧。
- RenderPass 之间仍通过共享 `RenderPassContext` 和 Pass 之间的直接引用传递视图及纹理资源，资源读写依赖尚未显式声明，后续可引入 Render Graph。
- 当前裁剪只使用世界空间包围球，非均匀缩放取最大轴形成保守半径；细长物体可能产生误保留，但不会错误剔除。遮挡裁剪与距离裁剪尚未实现。
- 主视图、反射视图和阴影视图每帧分别遍历场景并重建 `RenderItem` 列表；对象规模扩大后需要评估重复 CPU 遍历、容器填充和包围体变换成本。
- 不透明列表已按稳定资源 ID 排序，但 `AddPrimitive` 当前仍为每个 Primitive 创建独立 Mesh/Material，且各 Resource 类仍自行绑定/解绑 OpenGL 状态；排序已建立确定的提交顺序，实际状态切换收益需在资源共享和 RenderDoc 验证后实现。
- `RenderView` 已生成透明列表，但现有 Pass 不消费 `translucentItems`，透明物体当前不会进入正确的混合渲染路径。
- Forward 和 Reflection 颜色目标仍以固定分辨率初始化，窗口 resize 只更新窗口 viewport 与帧尺寸，没有重建对应的 Framebuffer 附件。
- 场景可保存多灯光代理，EditorPrimitivePass 也能遍历显示多灯光，但当前 PBR 光照与阴影路径仍只消费第一盏灯。
- 透明、HDR、后处理和多灯光会显著扩大 GPU 资源与调试范围，应逐步加入 RenderDoc 基线。
- 当前只完成编译、链接和 `FrustumTests` 自动验证，尚未启动图形程序；Visual Studio 2026 调试时应重点确认 PBR、阴影、反射、曲面细分、EditorPrimitive 灯光可视化及快捷键没有回归。

## 8. 最终展示目标

最终 Demo 以 60 秒展示完整工作流：

```text
0-10s   MaiX Renderer 与架构概览
10-25s  PBR：金属、塑料、粗糙材质
25-40s  NPR：Toon、Outline、Face Shadow
40-50s  HDR、Bloom、SSAO、Tone Mapping
50-60s  Editor 中实时修改材质和灯光参数
```

项目最终应能清楚展示：C++、OpenGL、GLSL、PBR、NPR、Render Pipeline、RenderDoc 与技术美术工具开发能力。
