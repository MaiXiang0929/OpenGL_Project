# MaiX Renderer

MaiX Renderer 是一个面向技术美术、Shader TA 和渲染工程学习的轻量级实时光栅化框架。项目使用 C++17、OpenGL 4.0 Core Profile、GLSL 和 Dear ImGui，目标是把一条可解释、可调试、可扩展的现代实时渲染链路落到可运行代码中。

它不是只展示一个模型的 Demo，而是一个围绕以下问题组织的渲染实验场：

- GPU 光栅化、深度测试、纹理采样和资源绑定如何在 Pass 之间流动；
- Cook-Torrance PBR、Toon/NPR、阴影、HDR 后处理和透明混合如何共存；
- Renderer 如何拥有 GPU 资源，Scene Proxy 如何向管线提交可见对象；
- 技术美术如何在运行时修改材质、灯光、Transform，并观察结果和性能统计。

## 展示重点

当前项目可以支持一段约 60 秒的技术展示：

| 时间段 | 展示内容 |
| --- | --- |
| 0-10s | Renderer 分层、RenderScene、RenderView 和多 Pass 管线 |
| 10-25s | 金属、塑料、陶瓷、粗糙金属的 PBR Material Lab |
| 25-40s | Toon 分段光照、Rim Light、Inverted Hull Outline、Face Shadow |
| 40-50s | RGBA16F HDR、Bloom、SSAO、手动曝光、ACES Tone Mapping |
| 50-60s | Scene Window、Inspector、Material Editor、灯光和 Transform 实时编辑 |

视觉结果以实际运行和用户验收为准；自动测试只负责验证程序、资源和数学逻辑没有明显错误。

## 渲染架构

```text
Application
  ├─ Window / Input / Camera
  ├─ Editable scene state
  └─ RenderFrameData
       │
       ▼
Renderer
  ├─ RenderScene
  │   ├─ PrimitiveSceneProxy
  │   └─ LightSceneProxy
  ├─ GPU resources
  │   ├─ Mesh / Material / Shader / Texture
  │   ├─ Framebuffer / ShadowMap / Cubemap
  │   └─ strong typed MeshHandle / MaterialHandle
  └─ RenderPipeline
      ├─ ShadowPass
      ├─ ReflectionPass
      ├─ ForwardPass
      ├─ OutlinePass
      ├─ TranslucencyPass
      ├─ SSAOPass
      ├─ BloomPass
      ├─ PostProcessPass
      ├─ EditorPrimitivePass
      └─ PresentPass
           │
           ▼
        OpenGL 4.0 GPU
```

每帧的数据流如下：

1. `Application` 更新相机、模型 Transform 和灯光，并提交 `RenderFrameData`。
2. `RenderScene::BuildRenderView()` 将 Scene Proxy 转换为主视图、反射视图和阴影视图的 `RenderItem` 列表。
3. CPU 执行包围球视锥裁剪、透明排序、不透明资源排序和批次准备。
4. GPU 依次执行阴影、反射、Forward/NPR、透明、屏幕空间后处理、编辑器叠加层和最终显示。

CPU 负责场景代理、可见性、矩阵、资源句柄和材质参数；GPU 负责顶点变换、三角形光栅化、深度/混合状态、纹理采样、BRDF 光照和像素输出。

## 已实现能力

### PBR 与材质

- Cook-Torrance metallic/roughness BRDF；
- Base Color、Normal、ORM、Displacement 固定纹理槽；
- ORM 通道约定：R=AO、G=Roughness、B=Metallic；
- sRGB/Linear 色彩空间契约和材质参数约束；
- 标准、Instancing、Tessellation/Displacement 路径共享 PBR 光照；
- `--material-lab` 提供 Copper、Plastic、Ceramic、Rough Metal 四种参考材质。

### NPR 与透明

- Toon 分段漫反射、阴影色和 Rim Light；
- Inverted Hull Outline，写入 Forward HDR Scene Color；
- Face Shadow：线性 FaceLightmap、局部 Face Forward/Right、自动左右镜像和独立 Key Light；
- 独立 TranslucencyPass：后向前排序、纹理 Alpha、Alpha Blend、深度只读；
- 当前透明路径支持 Opaque 和 Alpha Blend，Masked/Additive 尚未接入。

### 阴影、HDR 与后处理

- Directional/Spot 基础阴影和 PCF；当前 Forward 最多消费 16 盏灯，使用一张 2D Shadow Map；
- Cubemap、半分辨率反射、离屏 Framebuffer 和 Present Pass；
- RGBA16F HDR Scene Color；
- 半分辨率 Bloom 提取与双向模糊；
- 深度重建观察空间位置/法线的 SSAO；
- 手动曝光、ACES Tone Mapping 和 sRGB 输出编码。

### 场景、资源与编辑器

- `RenderScene`、Primitive/Light Scene Proxy 和主/反射/阴影视图裁剪；
- Mesh/Material Handle、Renderer-owned GPU 资源和资源提交边界；
- 不透明物体按 Shader/Material/Mesh 排序并支持共享资源 Instancing；
- FBX 静态模型导入：多 Mesh、多材质分段、索引绘制、外部/嵌入纹理和异步 CPU 解析；
- ImGui Renderer Statistics、Material Editor、Scene Window、Inspector；
- 视口 Orbit/Pan/Dolly、模型/灯光拾取、ImGuizmo Transform 和选中对象聚焦；
- GPU Debug Group、每 Pass Draw/资源统计和三帧 GPU Timer Query。

## 目标角色 Face Shadow Demo

外部角色资产不会被复制进仓库。当前验证过的目标资产位于用户本地目录，运行前需保留原目录中的署名说明。

从可执行文件目录运行：

```powershell
.\OpenGL_Project.exe --face-shadow-demo `
  "D:\Desktop\Unity URP shader\重逢-荧\Lumine FBX.fbx" `
  "D:\Desktop\Unity URP shader\重逢-荧\textures\Avatar_Girl_Tex_FaceLightmap.png" `
  "Lumine Face"
```

该启动路径会：

- 导入 `Lumine FBX.fbx`；
- 按精确名称匹配 `Lumine Face`；
- 将 `Avatar_Girl_Tex_FaceLightmap.png` 作为 Linear RGBA8 Face Shadow 纹理绑定；
- 自动启用 Toon 和 Face Shadow；
- 过滤远处武器辅助分段对相机、地面、主灯和阴影视锥的包围体污染，但不删除这些 Draw。

`Avatar_Tex_Face_Shadow.png` 是近似二值遮罩，不符合当前按灯光角度比较的连续阈值契约，因此不作为本 Demo 的 Face Shadow 输入。

## 环境要求

- Windows 10/11 x64；
- Visual Studio 2022 或兼容的 MSVC x64 工具链；
- CMake 3.23 或更高版本；
- 支持 OpenGL 4.0 Core Profile 的显卡和驱动。

主要第三方依赖已经放在 `ThirdParty/`：GLFW、GLAD、ufbx、cyCodeBase、LodePNG、Dear ImGui 和 ImGuizmo。

## 构建

请在 Visual Studio Developer PowerShell (x64) 中，从仓库根目录执行：

```powershell
cmake --preset windows-ninja-debug
cmake --build --preset windows-ninja-debug
```

输出程序位于：

```text
out/build/windows-ninja-debug/OpenGL_Project.exe
```

CMake 会在构建后清理并复制最新的 `assets/` 到可执行文件目录。

## 运行入口

默认场景：

```powershell
.\out\build\windows-ninja-debug\OpenGL_Project.exe
```

PBR Material Lab：

```powershell
.\out\build\windows-ninja-debug\OpenGL_Project.exe --material-lab
```

共享资源 Instancing 基准（N 为 1-32）：

```powershell
.\out\build\windows-ninja-debug\OpenGL_Project.exe --instance-grid 16
```

透明测试场景：

```powershell
.\out\build\windows-ninja-debug\OpenGL_Project.exe --translucency-test
```

查看全部参数：

```powershell
.\out\build\windows-ninja-debug\OpenGL_Project.exe --help
```

FBX 也可以在运行中的 `File > Import FBX...` 通过 Asset Import 面板导入。该流程会异步解析 CPU 数据，并在 GPU 资源创建成功后事务式替换当前模型。

## 编辑器操作

| 操作 | 用途 |
| --- | --- |
| 鼠标左键点选 | 选择模型或可见灯光 Gizmo |
| `Alt` + 左键拖动 | Orbit 相机 |
| `Alt` + 中键拖动 | Pan 相机 |
| `Alt` + 右键拖动或滚轮 | Dolly 相机 |
| `Ctrl` + 左键拖动 | 旋转主 Spot Light |
| `F` | 聚焦当前选中对象 |
| `W` / `E` / `R` | ImGuizmo 移动 / 旋转 / 缩放 |
| `Q` | 切换世界/局部 Transform 空间 |
| `F6` | 重新加载 GLSL Shader |
| `Esc` | 退出程序 |

Material Editor 修改会通过 `Renderer::UpdateMaterial()` 和 `Renderer::UpdateMaterialTexture()` 回到 Renderer-owned 资源，并在下一帧被 Forward、Reflection、Outline 和 Translucency 消费。

## 测试与验证

构建完成后运行：

```powershell
ctest --test-dir out/build/windows-ninja-debug --output-on-failure
```

当前包含 13 项测试，覆盖：

- Frustum 和 RenderScene 可见性；
- Light Render Data、Render Batch 和 Render Pass Contract；
- Render Target 尺寸、后处理设置和材质纹理类型；
- Application 参数、相机和编辑器交互；
- Image Loader 与 FBX Model Importer。

运行时还应检查控制台中的 OpenGL 版本、Shader 初始化、Pass Draw 统计和 GPU Timer Query。视觉效果（尤其是 Face Shadow 的轴向、镜像和阈值方向）仍需人工验收。

## 项目结构

```text
src/
├─ Assets/                 FBX、图片和 CPU 导入流程
├─ Core/                   Application、Camera、Input、Frame 数据
├─ Editor/                 ImGui 面板、拾取、Gizmo 和编辑状态
├─ Platform/               Windows 文件对话框等平台边界
└─ Renderer/
   ├─ Core/                 Renderer 门面和 GPU 资源所有权
   ├─ Diagnostics/          GPU Timer、Debug Group、提交统计
   ├─ Pipeline/             RenderPipeline、Pass Contract、Settings
   ├─ Passes/               Shadow、Reflection、Forward、NPR、后处理
   ├─ Resources/            Mesh、Material、Shader、Texture、Framebuffer
   ├─ Scene/                RenderScene、Primitive/Light Proxy
   └─ View/                 RenderView、RenderItem、Batch、Frustum

assets/shaders/
├─ pbr/                     标准 PBR、Instanced、Tessellation/Displacement
├─ shadow/                  阴影深度
├─ environment/             Skybox/Cubemap
├─ reflection/              反射地面
├─ npr/                     Outline 等 NPR Shader
├─ postprocess/             SSAO、Bloom、PostProcess
├─ present/                 离屏结果显示
└─ debug/                   灯光 Gizmo 等调试 Shader

docs/                       各子系统的数据流、契约和验证记录
tests/                      数学、资源、导入器和编辑器交互测试
ThirdParty/                 固定版本第三方依赖
```

## 技术边界与下一步

当前实现刻意保持在可展示、可解释的范围内，尚未包括：

- Cascaded Shadow Maps、Point Light 阴影和多 Shadow Map 管理；
- 自动曝光、遮挡裁剪、距离裁剪和更精确的包围体；
- Alpha Cutout 阴影、Masked/Additive 和 OIT；
- Hair Highlight、通用 Shadow Ramp、动画运行时蒙皮；
- 显式 Render Graph、跨 API RHI、完整资源 generation 校验；
- Scene 层级、对象创建/删除、序列化和 Undo/Redo。

下一轮工作遵循最终 60 秒展示目标：先完成 Lumine Face Shadow 的用户视觉验收，再验收 50-60 秒 Editor 工作流；在透明材质扩展前，先明确 Alpha Cutout 阴影、排序和混合策略。性能优化只有在基准或 RenderDoc 证据显示瓶颈后才进入优先级。

## 文档索引

- [PBR Material Workflow](docs/pbr-material-workflow.md)
- [Toon Shading](docs/npr-toon.md)
- [Face Shadow](docs/npr-face-shadow.md)
- [Outline](docs/npr-outline.md)
- [Translucency Pass](docs/translucency-pass.md)
- [SSAO](docs/ssao.md)
- [HDR and Tone Mapping](docs/hdr-tone-mapping.md)
- [Bloom and Post Process](docs/bloom-postprocess.md)
- [Scene Window and Inspector](docs/editor-scene-inspector.md)
- [Viewport Transform Controls](docs/viewport-transform-controls.md)
- [GPU Pass Profiling](docs/gpu-pass-profiling.md)
- [RenderDoc Baseline](docs/renderdoc-baseline.md)

## 许可证与资产署名

项目代码和第三方依赖的许可证信息请查看对应目录中的 LICENSE/README 文件。外部角色资产不属于本仓库；使用或分发该资产时，必须保留其原始 `README.txt` 中的作者和来源说明。
