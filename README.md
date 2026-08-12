# OpenGL Project

基于 OpenGL 4.0 Core Profile 的交互式计算机图形学练习项目。

项目使用 C++17 实现，当前包含 OBJ 模型加载、Blinn-Phong 光照、材质纹理、天空盒、环境反射、离屏渲染、平面反射和聚光灯阴影等功能。

## 功能

- 加载 OBJ/MTL 模型
- 自动生成缺失的顶点法线
- 根据模型包围盒自动调整相机距离和地面尺寸
- 加载漫反射贴图与高光贴图
- Blinn-Phong 光照
- Cubemap 天空盒与环境反射
- 基于 Framebuffer Object 的离屏渲染
- 基于反射相机的平面反射
- 2048 × 2048 聚光灯阴影贴图
- 3 × 3 PCF 阴影过滤
- 透视投影与正交投影切换
- 着色器运行时重新加载
- 光源位置调试图标

## 渲染流程

每帧主要执行以下渲染阶段：

1. 从光源视角渲染模型深度，生成阴影贴图。
2. 使用镜像相机将天空盒和模型渲染到反射纹理。
3. 将天空盒、反射地面、模型和光源图标渲染到离屏颜色缓冲。
4. 将离屏颜色纹理绘制到窗口中的显示平面。

CPU 侧负责模型加载、资源管理、输入处理和矩阵计算；GPU 侧负责顶点变换、光照、环境反射、阴影采样和最终像素输出。

## 环境要求

- Windows 10/11 x64
- Visual Studio 2022，包含“使用 C++ 的桌面开发”组件
- CMake 3.15 或更高版本
- 支持 OpenGL 4.0 Core Profile 的显卡及驱动

项目已在 `ThirdParty/` 中包含 GLAD、GLFW、cyCodeBase 和 LodePNG，无需额外下载这些依赖。

## 构建

在项目根目录执行：

```powershell
cmake --preset windows-ninja-debug
cmake --build --preset windows-ninja-debug
```

These commands must be run from a Visual Studio Developer PowerShell (x64),
so that the MSVC compiler and Ninja are available on `PATH`.

构建完成后，可执行文件位于：

```text
out/build/windows-ninja-debug/OpenGL_Project.exe
```

CMake 会在构建后自动将 `assets/` 复制到可执行文件目录。

也可以直接使用 Visual Studio 打开项目根目录，通过内置 CMake 支持进行配置和构建。

## 运行

不传入参数时，程序会加载内置茶壶模型：

```powershell
.\out\build\windows-ninja-debug\OpenGL_Project.exe
```

也可以通过第一个命令行参数加载其他 OBJ 模型：

```powershell
.\out\build\windows-ninja-debug\OpenGL_Project.exe path\to\model.obj
```

模型引用的 MTL 文件和 PNG 纹理应位于有效的相对路径中。当前渲染器会使用 MTL 中首个有效的漫反射贴图和高光贴图。

## 操作

| 输入 | 功能 |
| --- | --- |
| 鼠标左键拖动 | 旋转模型观察相机 |
| 鼠标右键拖动 | 调整模型观察距离 |
| `Ctrl` + 鼠标左键拖动 | 绕模型旋转光源 |
| `Alt` + 鼠标左键拖动 | 旋转最终显示平面的相机 |
| `Alt` + 鼠标右键拖动 | 调整最终显示平面的观察距离 |
| `P` | 切换透视投影与正交投影 |
| `S` | 开启或关闭阴影 |
| `L` | 显示或隐藏光源调试图标 |
| `F6` | 重新加载 GLSL 着色器 |
| `Esc` | 退出程序 |

按住 `Alt` 时，显示平面相机操作具有最高优先级。

## 项目结构

```text
OpenGL_Project/
├── assets/
│   ├── models/              # OBJ、MTL、PNG 和 Cubemap 资源
│   └── shaders/
│       ├── pbr/             # Cook-Torrance 与 Tessellation Shader
│       ├── shadow/          # 阴影深度 Shader
│       ├── environment/     # Skybox Shader
│       ├── reflection/      # 平面反射地面 Shader
│       ├── present/         # 最终显示 Shader
│       └── debug/           # Gizmo 与调试 Shader
├── docs/
│   └── assignments/         # 课程作业笔记
├── src/
│   ├── Core/                # 应用生命周期、窗口输入和相机
│   ├── Editor/              # 光源调试图标
│   ├── Renderer/
│   │   ├── Core/            # Renderer 门面与资源所有权
│   │   ├── Pipeline/        # Pipeline、Pass 契约与渲染设置
│   │   ├── Passes/          # Shadow、Reflection、Forward、Present
│   │   ├── Scene/           # RenderScene 与渲染侧 Scene Proxy
│   │   ├── View/            # RenderView 与每视图绘制列表
│   │   └── Resources/       # Mesh、Material、Shader、Texture 与 FBO
│   └── main.cpp             # 程序入口
├── ThirdParty/              # 第三方依赖
├── CMakeLists.txt
└── README.md
```

## 核心模块

- `Application`：管理 GLFW 窗口、输入、模型加载和每帧渲染流程。
- `Renderer/Core`：持有场景 GPU 资源并启动渲染管线。
- `RenderPipeline`：拥有并按顺序执行 Shadow、Reflection、Forward、Present 渲染 Pass。
- `RenderPass`：定义单个渲染阶段的执行契约，Pass 之间通过当前帧上下文传递工作。
- `Renderer/Passes`：实现各个具体 GPU 渲染阶段。
- `RenderScene`：保存与游戏逻辑解耦的 Primitive 和 Light 渲染代理。
- `RenderView`：描述一次相机观察，并保存按材质类型分类的 `RenderItem` 列表。
- `Renderer/Resources`：封装 Mesh、Material、Shader、Texture 和 Framebuffer 等 GPU 资源。
- `Camera`：生成视图矩阵和透视/正交投影矩阵。
- `Mesh`：管理 VAO、VBO 和网格绘制。
- `Shader`：负责 GLSL 编译、链接和 Uniform 设置。
- `Framebuffer`：管理离屏颜色与深度附件。
- `ShadowMap`：管理阴影深度纹理和深度专用 FBO。
- `LightGizmo`：绘制光源位置调试图标。

## 课程参考

本项目参考 University of Utah 的 Interactive Computer Graphics 课程：

- Course: Interactive Computer Graphics
- School: School of Computing, University of Utah
- Website: https://graphics.cs.utah.edu/courses/cs6610/spring2021/

`docs/assignments/` 中保留了 Project 1 至 Project 3 的学习笔记；当前主程序已在这些基础上继续扩展离屏渲染、反射与阴影功能。

## 第三方库

- GLFW
- GLAD
- cyCodeBase
- LodePNG

各第三方库的许可信息请参阅其在 `ThirdParty/` 目录中的 LICENSE 或 README 文件。
