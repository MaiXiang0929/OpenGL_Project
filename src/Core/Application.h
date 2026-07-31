#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

/**
 * @brief 应用程序核心管理类
 * @details 负责管理整个程序的生命周期，包括初始化、主循环更新、渲染及资源清理。
 */
class Application
{

public:
    /**
     * @brief 构造函数，初始化成员变量默认值
     */
    Application();

    /**
     * @brief 析构函数，自动调用 Shutdown 释放资源
     */
    ~Application();

    /**
     * @brief 运行应用程序
     */
    void Run();

private:
    /**
     * @brief 初始化应用程序
     */
    bool Init();
    
    /**
     * @brief 更新应用程序状态
     */
    void Update();
    
    /**
     * @brief 渲染应用程序
     */
    void Render();
    
    /**
     * @brief 关闭应用程序
     */
    void Shutdown();

private:
    GLFWwindow* m_Window; ///< GLFW 窗口对象的指针句柄
};