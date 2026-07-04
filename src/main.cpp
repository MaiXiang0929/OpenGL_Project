#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

int main()
{
    // 1. 初始化GLFW
    glfwInit();

    // 2. 设置OpenGL版本 3.3 Core
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 3. 创建窗口
    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL", NULL, NULL);

    if (!window)
    {
        std::cout << "Failed to create window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // 4. 加载OpenGL函数
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to init GLAD" << std::endl;
        return -1;
    }

    // 5. 设置视口
    glViewport(0, 0, 800, 600);

    // 6. 渲染循环（游戏本质）
    while (!glfwWindowShouldClose(window))
    {
        // 清屏（黑色背景）
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // 交换缓冲（显示画面）
        glfwSwapBuffers(window);

        // 处理输入
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}