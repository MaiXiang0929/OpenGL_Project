# MaiX Renderer 项目升级规划

## 项目定位

当前 OpenGL_Project 已经具备实时渲染 Demo 的基础能力，但原定位偏向：

> 基于 OpenGL 的计算机图形学练习项目

为了匹配 2027 秋招技术美术（Rendering TA）方向，需要升级为：

> MaiX Renderer  
> A lightweight real-time rendering framework developed with C++ and OpenGL, focusing on physically based rendering, shader development and technical art workflows.

中文定位：

基于 C++17 与 OpenGL 的实时渲染框架，用于探索现代渲染管线、Shader 开发以及技术美术工作流。

---

# 一、项目目标

最终目标：

- 展示 GPU 渲染流程理解
- 展示 Shader 开发能力
- 展示实时渲染技术
- 展示技术美术工作流
- 展示性能分析能力

目标岗位：

- 渲染技术美术
- Shader TA
- 引擎技术美术

---

# 二、项目架构调整

当前：

```
src
├── Core
├── Editor
├── Renderer
└── main.cpp
```

建议：

```
src
│
├── Core
│   ├── Application
│   ├── Window
│   └── Input
│
├── Renderer
│   ├── RenderPipeline
│   ├── RenderPass
│   ├── Shader
│   ├── Texture
│   ├── Mesh
│   ├── Material
│   ├── Framebuffer
│   └── ShadowMap
│
├── Scene
│   ├── Model
│   ├── Light
│   └── Transform
│
├── Editor
│   ├── LightGizmo
│   ├── Inspector
│   └── MaterialEditor
│
└── main.cpp
```

---

# 三、Renderer 架构升级

当前：

```cpp
Renderer::Render()
{
    RenderShadow();
    RenderReflection();
    RenderScene();
    RenderScreen();
}
```

升级为：

```
RenderPipeline

    |
    |
    +-- ShadowPass
    |
    +-- ReflectionPass
    |
    +-- ForwardPass
    |
    +-- PostProcessPass
    |
    +-- PresentPass
```

目标：

实现类似游戏引擎的多 Pass 渲染流程。

---

# 四、Material 系统（最高优先级）

当前：

Shader 内部直接定义材质参数。

升级：

```
Material

    Shader

    Albedo Texture

    Normal Texture

    Metallic

    Roughness

    AO
```

目标：

实现类似 Unity Material Inspector 的工作流。

例如：

```
Material Editor

Albedo

Metallic 0.5

Roughness 0.2

Normal Map
```

---

# 五、PBR Renderer

替换基础 Blinn-Phong。

实现：

## Cook-Torrance BRDF

支持：

- Albedo
- Normal
- Metallic
- Roughness
- Ambient Occlusion


目标：

展示：

- 金属
- 塑料
- 陶瓷
- 粗糙表面

效果。

README描述：

Implemented physically based rendering pipeline based on Cook-Torrance BRDF.

---

# 六、Shadow System

当前已有：

- Shadow Map
- PCF Filtering


继续优化：

基础：

- Directional Shadow Mapping


进阶：

- Cascaded Shadow Mapping (CSM)

---

# 七、Post Processing

利用已有 Framebuffer。

新增：

```
PostProcess

├── HDR
├── Bloom
├── Tone Mapping
└── SSAO
```

新的流程：

```
Scene

↓

HDR Framebuffer

↓

Bloom Pass

↓

Tone Mapping

↓

Screen
```

---

# 八、NPR Anime Shader（重点）

用于匹配米哈游等二次元游戏方向。

新增：

```
Shaders

├── PBR

└── NPR
    ├── Toon Lighting
    ├── Face Shadow
    ├── Outline
    ├── Rim Light
    └── Hair Highlight
```

目标：

实现：

PBR 模式

↓

Anime Rendering 模式

---

# 九、Editor 工具

加入 ImGui。

实现：

## Scene Window

显示场景对象。


## Inspector

修改：

- Transform
- Light
- Material


## Material Editor

实时修改：

- Metallic
- Roughness
- Color
- Texture

---

# 十、RenderDoc 性能分析

增加：

Performance Analysis 文档。

记录：

- Draw Call 数量
- GPU Pass 时间
- Texture 使用情况


例如：

```
Shadow Pass

GPU Time: 1.2ms

Draw Calls: 100
```

并展示优化过程。

---

# 十一、开发优先级

## 第一阶段

目标：

1个月

完成：

- Material System
- PBR
- HDR


---

## 第二阶段

目标：

1个月

完成：

- Bloom
- Tone Mapping
- SSAO


---

## 第三阶段

目标：

1个月

完成：

- NPR Anime Shader
- ImGui Editor
- RenderDoc分析


---

# 十二、最终作品展示视频

60秒 Demo：

## 0-10秒

MaiX Renderer

C++ OpenGL Real-time Rendering Framework


## 10-25秒

PBR展示：

- 金属
- 粗糙材质


## 25-40秒

NPR展示：

- Toon
- Outline
- Face Shadow


## 40-50秒

Post Processing：

- Bloom
- SSAO


## 50-60秒

Editor：

实时调整材质参数。

---

# 最终目标

将项目从：

OpenGL 学习项目

升级为：

Rendering Technical Artist Portfolio Project

核心能力：

- C++
- OpenGL
- GLSL/HLSL
- PBR
- NPR
- Render Pipeline
- RenderDoc
- Technical Art Workflow
