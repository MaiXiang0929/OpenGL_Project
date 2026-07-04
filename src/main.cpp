#include <iostream>
// 注意：GLAD 必须在 GLFW 之前包含，它会引入所需的 OpenGL 头文件
#include <glad/glad.h> 
#include <GLFW/glfw3.h>

// 屏幕宽高
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

// 1. 顶点着色器源码 (完全对应视频 27:39 处的逻辑，采用 330 core 核心模式)
// 视频中教授使用了 uniform mvp 矩阵，为了保持绘制三角形的简单直观，这里暂不引入复杂的矩阵变换，聚焦于管线本身
const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 pos;\n" // 对应视频中 location 0 的顶点属性
"void main()\n"
"{\n"
"   gl_Position = vec4(pos.x, pos.y, pos.z, 1.0);\n" // 转换为齐次坐标并输出给光栅化器
"}\0";

// 2. 片元着色器源码 (完全对应视频 23:27 处的逻辑，输出纯红色)
const char* fragmentShaderSource = "#version 330 core\n"
"layout (location = 0) out vec4 color;\n" // 输出到第0个渲染目标
"void main()\n"
"{\n"
"   color = vec4(1.0f, 0.0f, 0.0f, 1.0f);\n" // 对应视频中的 vec4(1, 0, 0, 1) 纯红色
"}\0";

// 窗口大小改变的回调函数
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

int main() {
    // ==========================================
    // 第一步：初始化 GLFW 窗口并配置 OpenGL 上下文
    // ==========================================
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // 配置 GLFW：指定使用 OpenGL 3.3 Core Profile (对应视频推荐版本)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Mac 必须加上这行
#endif

    // 创建窗口对象
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Modern OpenGL Triangle", NULL, NULL);
    if (window == NULL) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // 初始化 GLAD (运行时动态加载所有 OpenGL 函数指针)
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // ==========================================
    // 第二步：运行时编译和链接着色器 (对应视频 36:14 的动态编译逻辑)
    // ==========================================

    // 编译顶点着色器
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // (此处教授提及实际开发中需进行代码报错检查，为简化排版暂略，确保源码无误即可)

    // 编译片元着色器
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // 链接着色器程序 (Program) 将顶点与片元组合在一起
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader); // 对应视频：attach 顶点着色器
    glAttachShader(shaderProgram, fragmentShader); // 对应视频：attach 片元着色器
    glLinkProgram(shaderProgram);                  // 对应视频：link 整个程序

    // 链接完成后，单独的着色器对象就可以释放了
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // ==========================================
    // 第三步：设置顶点数据与缓冲区 (对应视频 49:10 的 VBO/VAO 绑定状态机逻辑)
    // ==========================================

    // 逆时针定义的 3 个顶点坐标 (X, Y, Z)，在规范化设备坐标系 (NDC) 中
    float vertices[] = {
         -0.5f, -0.5f, 0.0f, // 左下角
          0.5f, -0.5f, 0.0f, // 右下角
          0.0f,  0.5f, 0.0f  // 顶角
    };

    unsigned int VAO, VBO;

    // 🌟 必须最先创建并绑定 VAO！(对应视频 1:04:35 处强调的：现代 OpenGL 中 VAO 不是可选的，不绑定必崩溃)
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO); // 激活 VAO 状态，后续的 VBO 关系及属性指针都会被它“记录”下来

    // 创建并绑定 VBO (对应视频 50:11)
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO); // 绑定到状态机插槽
    // 将 CPU 中的内存数据复制到 GPU 优化的显存中，GL_STATIC_DRAW 表示数据不常修改 (对应视频 52:01)
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 告诉 OpenGL 如何解析顶点数据 (对应视频 57:18)
    // 参数含义：属性位置0 | 每次读取3个值 | 数据类型Float | 不归一化 | 步长0(紧密排列) | 起始偏移量0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0); // 激活 location = 0 的顶点属性 (对应视频 56:35)

    // 解绑（非必须，为了保持状态机干净）
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    // 正确的代码
    glBindVertexArray(0);

    // ==========================================
    // 第四步：渲染主循环 (对应视频 59:35)
    // ==========================================
    while (!glfwWindowShouldClose(window)) {
        // 输入状态轮询（如按下 ESC 键退出）
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        // 清空屏幕背景 (用暗灰色填充)
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // 渲染三角形
        glUseProgram(shaderProgram);            // 使用刚才链接的着色器程序
        glBindVertexArray(VAO);                 // 绑定 VAO，自动恢复之前配置的所有 VBO 映射
        glDrawArrays(GL_TRIANGLES, 0, 3);       // 绘制图元：类型为三角形，从第0个顶点开始，绘制3个顶点 (对应视频 1:00:16)

        // 双缓冲区交换与事件触发
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // ==========================================
    // 第五步：清理资源
    // ==========================================
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}