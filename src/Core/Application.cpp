// SPDX-License-Identifier: MIT
/// @file Application.cpp
/// @brief 应用程序核心管理类的实现文件
/// @details 该文件实现了 Application 类的核心功能，包括初始化、主循环更新、渲染及资源清理。
/// @author MaiX
/// @date 2026-07-31

#include"Application.h"
#include"Renderer.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "cyTriMesh.h"

#include <iostream>

/// @brief 构造函数
Application::Application()
    :
    m_Camera(
        cy::Vec3f(0, 0, 0),
        50.0f
    )
{
    
}

/// @brief 析构函数，自动调用 Shutdown 释放资源
Application::~Application() {
    Shutdown();
}

/// @brief 运行应用程序
void Application::Run() {
    // 初始化引擎，如果失败直接退出
    if (!Init()) {
		Shutdown();
        std::cerr << "[Error] Engine initialization failed!" << std::endl;
        return;
    }

    // 游戏主循环 (Game Loop)
    while (!glfwWindowShouldClose(m_Window)) {
        // 处理系统的底层事件（如键盘、鼠标输入触发）
        glfwPollEvents();

        // 执行逻辑更新与渲染
        Update();
        Render();

        // 交换前后缓冲区，把画面显示到屏幕上
        glfwSwapBuffers(m_Window);
    }
}

/// @brief 初始化应用程序
bool Application::Init() {
    // 初始化 GLFW
    if (!glfwInit()) {
        std::cerr << "[Error] Failed to initialize GLFW" << std::endl;
        return false;
    }

    // 配置 OpenGL 3.3 Core Profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 开启 OpenGL Debug Context
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);    // MacOS 需要设置前向兼容
#endif

    // 创建窗口 (默认 1920x1080)
    m_Window = glfwCreateWindow(m_Width, m_Height, "OpenGL Engine", nullptr, nullptr);
    if (!m_Window) {
        std::cerr << "[Error] Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

	glfwMakeContextCurrent(m_Window);   // 将窗口的上下文设置为当前线程的上下文



    // 将当前的 Application 实例指针绑定到 GLFW 窗口上
    glfwSetWindowUserPointer(m_Window, this);

    // 绑定所有的输入回调
    glfwSetFramebufferSizeCallback(m_Window, FramebufferSizeCallback);
    glfwSetMouseButtonCallback(m_Window, MouseButtonCallback);
    glfwSetCursorPosCallback(m_Window, CursorPositionCallback);
    glfwSetKeyCallback(m_Window, KeyCallback);

    // 初始化 GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "[Error] Failed to initialize GLAD" << std::endl;  

        glfwDestroyWindow(m_Window);
        m_Window = nullptr;
        glfwTerminate();

        return false;
    }

    m_Camera.SetAspectRatio(
        (float)m_Width / (float)m_Height
    );

    std::cout << "[System] Engine Initialized Successfully! " << std::endl;

    // =======================================
	// 打印初始化信息（后续再加入完整 Debug Layer）
    // =======================================
    std::cout << "\n========== Renderer Info ================\n";

    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GPU: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl; 

    // OpenGL Capability
    int maxTextureSize = 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
    std::cout << "Max Texture Size: " << maxTextureSize << std::endl;

    std::cout << "=========================================\n\n";

    // 开启 OpenGL 默认状态
	m_Renderer = new Renderer();
	m_Renderer->Init();

    // ==========================================
    // 第一阶段妥协：在 App 层临时加载网格和纹理
    // ==========================================
    cy::TriMesh mesh;
    std::string objPath = "assets/models/teapot.obj";
    if (!mesh.LoadFromFileObj(objPath.c_str())) {
        std::cerr << "Failed to load obj!" << std::endl;
        return false;
    }
    mesh.ComputeBoundingBox();
    mesh.ComputeNormals();
    m_ObjCenter = (mesh.GetBoundMax() + mesh.GetBoundMin()) * 0.5f;

    std::vector<Vertex> vertices;
    vertices.reserve(static_cast<size_t>(mesh.NF()) * 3);
    bool hasTexCoords = mesh.HasTextureVertices();
    for (int i = 0; i < mesh.NF(); ++i) {
        cy::TriMesh::TriFace face = mesh.F(i);
        cy::TriMesh::TriFace faceNormal = mesh.FN(i);
        cy::TriMesh::TriFace faceTex = hasTexCoords ? mesh.FT(i) : cy::TriMesh::TriFace();
        for (int j = 0; j < 3; ++j) {
            Vertex vertex{};
            vertex.Position = mesh.V(face.v[j]);
            vertex.Normal = mesh.VN(faceNormal.v[j]);
            if (hasTexCoords) {
                cy::Vec3f vt = mesh.VT(faceTex.v[j]);
                vertex.TexCoord = cy::Vec2f(vt.x, 1.0f - vt.y);
            }
            else {
                vertex.TexCoord = cy::Vec2f(0.0f, 0.0f);
            }
            vertices.push_back(vertex);
        }
    }
    m_Renderer->SetMesh(vertices);

    // 加载纹理（硬编码路径测试）
    m_Renderer->LoadTextures("assets/models/brick.png", "assets/models/brick-specular.png");

    glfwGetCursorPos(m_Window, &m_LastX, &m_LastY);

    m_Initialized = true;
    return true;
}

/// @brief 更新应用程序状态
void Application::Update() {

}

/// @brief 渲染应用程序
void Application::Render() {
	m_Renderer->BeginFrame();

	cy::Matrix4f projMatrix = m_Camera.GetProjectionMatrix();
	cy::Matrix4f viewMatrix = m_Camera.GetViewMatrix();
    cy::Matrix4f modelMatrix = cy::Matrix4f::Translation(-m_ObjCenter);

    cy::Matrix4f mv = viewMatrix * modelMatrix;
    cy::Matrix4f mvp = projMatrix * viewMatrix * modelMatrix;

    // 光源计算
    cy::Vec3f lightBasePos(0.0f, 10.0f, 20.0f);
    // 通过 lightRot 变量将光源绕物体中心旋转
    cy::Matrix4f lightRotMatrix = cy::Matrix4f::RotationX(m_LightRotX) * cy::Matrix4f::RotationY(m_LightRotY);
    cy::Vec4f lightPosWorld = lightRotMatrix * cy::Vec4f(lightBasePos.x, lightBasePos.y, lightBasePos.z, 1.0f);
    // 统一转换到 View Space，以便在着色器中进行光照计算
    cy::Vec4f lightPosView = viewMatrix * lightPosWorld;

    // 2. 将数据提交给 Renderer
    m_Renderer->RenderScene(mvp, mv, cy::Vec3f(lightPosView.x, lightPosView.y, lightPosView.z));

    // --- 判断并绘制 LightGizmo ---
    if (m_DrawDebugGizmos) {
        // 直接将投影矩阵、观察矩阵以及世界空间光源位置传给 Renderer
        m_Renderer->DrawLightGizmo(projMatrix, viewMatrix, cy::Vec3f(lightPosWorld.x, lightPosWorld.y, lightPosWorld.z), 1.0f);
    }

	m_Renderer->EndFrame();
}

/// @brief 关闭应用程序
void Application::Shutdown() {
	
    if (!m_Initialized) return;

	// 释放渲染器资源
	if (m_Renderer) {
		delete m_Renderer;
		m_Renderer = nullptr;
	}
    
	// 释放 GLFW 窗口资源
    if (m_Window) {
		glfwDestroyWindow(m_Window);
		m_Window = nullptr;
	}

	// 终止 GLFW
	glfwTerminate();

    m_Initialized = false;

	std::cout << "[System] Engine Shutdown Successfully!" << std::endl;
}

// ==========================================
// 静态回调函数实现（使用 UserPointer 访问实例）
// ==========================================
void Application::FramebufferSizeCallback(GLFWwindow* window, int width, int height) {
    Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    if (app) {
        app->m_Width = width;
        app->m_Height = height;

        app->m_Camera.SetAspectRatio(
            (float)width / (float)height
        );

        glViewport(0, 0, width, height);
    }
}

void Application::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) app->m_LeftDown = true;
        else if (action == GLFW_RELEASE) app->m_LeftDown = false;
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) app->m_RightDown = true;
        else if (action == GLFW_RELEASE) app->m_RightDown = false;
    }
}

void Application::CursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
    Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    double dx = xpos - app->m_LastX;
    double dy = ypos - app->m_LastY;
    app->m_LastX = xpos;
    app->m_LastY = ypos;

    app->m_CtrlDown = (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS);

    // 左键旋转
    if (app->m_LeftDown) {
        if (app->m_CtrlDown) {
            app->m_LightRotX += (float)dx * 0.01f;
            app->m_LightRotY += (float)dy * 0.01f;
        }else {
            app->m_Camera.ProcessMouseOrbit(dx, dy);
        }

    }

    // 右键缩放
    if (app->m_RightDown) {
		app->m_Camera.ProcessMouseZoom(dy);
    }
}

void Application::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    // 着色器重载（F6）
    if (key == GLFW_KEY_F6 && action == GLFW_PRESS) {
		app->m_Renderer->ReloadShaders();
    }

    // 投影模式切换（P）
    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        app->m_Camera.ToggleProjectionMode();
    }

    // --- L 键切换光源 Gizmo 显示 ---
    if (key == GLFW_KEY_L && action == GLFW_PRESS) {
        app->m_DrawDebugGizmos = !app->m_DrawDebugGizmos;
        std::cout << "[Editor] Debug Gizmos: " << (app->m_DrawDebugGizmos ? "ON" : "OFF") << std::endl;
    }

    // 退出（ESC）
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}
