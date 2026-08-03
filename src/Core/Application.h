// SPDX-License-Identifier: MIT
/// @file Application.h
/// @brief 应用程序核心管理类的头文件
/// @details 该类负责管理整个程序的生命周期，包括初始化、主循环更新、渲染及资源清理。
/// @author MaiX
/// @date 2026-07-31

#pragma once

#include <string>
#include "cyVector.h"

#include "Camera.h"

struct GLFWwindow;
class Renderer;

/// @brief 应用程序核心管理类
/// @details 该类负责管理整个程序的生命周期，包括初始化、主循环更新、渲染及资源清理。
class Application
{

public:
	/// @brief 构造函数
    Application();

	/// @brief 析构函数，自动调用 Shutdown 释放资源
    ~Application();

	// 禁止复制
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

	/// @brief 运行应用程序
    void Run();

private:
    GLFWwindow* m_Window        = nullptr; ///< GLFW 窗口对象的指针句柄

    Renderer* m_Renderer        = nullptr; ///< 渲染器对象的指针句柄

	Camera m_Camera;                     ///< 摄像机对象

    bool m_Initialized          = false;    ///< 应用程序是否已初始化

    unsigned int m_Width        = 1920;  ///< 窗口宽度
    unsigned int m_Height       = 1080; ///< 窗口高度

    // --- Mouse state ---
    double m_LastX              = 0.0;
    double m_LastY              = 0.0;
    bool m_LeftDown             = false;
    bool m_RightDown            = false;

	// --- light ---
    bool m_CtrlDown             = false;
    float m_LightRotX           = 0.0f;
    float m_LightRotY           = 0.0f;

    cy::Vec3f m_ObjCenter; // 模型的中心点

    // --- Debug 可视化开关 ---
    bool m_DrawDebugGizmos      = true;

private:
    /// @brief 初始化应用程序
    /// @return 初始化成功返回 true，否则返回 false
    bool Init();
    
	/// @brief 更新应用程序状态
    void Update();
    
	/// @brief 渲染应用程序
    void Render();
    
	/// @brief 关闭应用程序
    void Shutdown();

    // 回调函数必须是 static 的，配合 UserPointer 使用
    static void FramebufferSizeCallback(
        GLFWwindow* window,
        int width,
        int height);

    static void MouseButtonCallback(GLFWwindow* window,
        int button,
        int action,
        int mods);

    static void CursorPositionCallback(GLFWwindow* window,
        double xpos,
        double ypos);

    static void KeyCallback(GLFWwindow* window,
        int key,
        int scancode,
        int action,
        int mods);   
};
