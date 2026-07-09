    #include <iostream>
    // 注意：GLAD 必须在 GLFW 之前包含，它会引入所需的 OpenGL 头文件
    #include <glad/glad.h> 
    #include <GLFW/glfw3.h>
    #include <fstream>      // 用于读取着色器文件
    #include <sstream>      // 用于将文件内容转换成字符串    

    // 屏幕宽高
    const unsigned int SCR_WIDTH = 1920;
    const unsigned int SCR_HEIGHT = 1080;

    // 窗口大小改变的回调函数
    void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
        glViewport(0, 0, width, height);
    }

    // 读取文件内容的函数
    std::string ReadFile(const std::string& filePath)
    {
        std::ifstream file;
	    // 设置异常掩码，确保在文件打开失败时抛出异常
        file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        try
        {
            file.open(filePath);

            std::stringstream ss;
            ss << file.rdbuf();

            file.close();

            return ss.str();
        }
        catch (std::ifstream::failure& e)
        {
            std::cerr << "[Shader File Error] Cannot read: " << filePath << std::endl;

            std::cerr << "Reason: " << e.what() << std::endl;

            return "";
        }
    }

    int main() {
        // =================================
        // 初始化 GLFW 窗口并配置 OpenGL 上下文
        // =================================
        if (!glfwInit()) {
            std::cerr << "Failed to initialize GLFW" << std::endl;
            return -1;
        }

        // 配置 GLFW：指定使用 OpenGL 3.3 Core Profile
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    #ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Mac 必须加上这行
    #endif

        // 创建窗口对象
	    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL", NULL, NULL); // 创建一个 1920x1080 的窗口，标题为 "OpenGL"
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

        // ===================
        // 运行时编译和链接着色器
        // ===================

        // 编译顶点着色器
        std::string vertexCodeStr = ReadFile("assets/shaders/triangle.vert");
	    const char* vsSource = vertexCodeStr.c_str();
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vsSource, nullptr);
        glCompileShader(vertexShader);

        // 编译片元着色器
        std::string fragmentCodeStr = ReadFile("assets/shaders/triangle.frag");
        const char* fsSource = fragmentCodeStr.c_str();
        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fsSource, nullptr);
        glCompileShader(fragmentShader);

        // 链接着色器程序 (Program) 将顶点与片元组合在一起
        GLuint shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader); // attach 顶点着色器
        glAttachShader(shaderProgram, fragmentShader); // attach 片元着色器
        glLinkProgram(shaderProgram);                  // link 整个程序

        // 链接完成后，单独的着色器对象就可以释放了
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        // =================
        // 设置顶点数据与缓冲区
        // =================

        // 位置 + 颜色
        float vertices[] =
        {
            // position           // color
		    -0.5f, -0.5f, 0.0f,   1.0f, 0.0f, 0.0f, 1.0f, // 左下角，红色
		     0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f, 1.0f, // 右下角，绿色
		     0.0f,  0.5f, 0.0f,   0.0f, 0.0f, 1.0f, 1.0f  // 顶部，蓝色
        };

        GLuint VAO, VBO;

        // 必须最先创建并绑定 VAO
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO); // 激活 VAO 状态，后续的 VBO 关系及属性指针都会被它“记录”下来

        // 创建并绑定 VBO
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO); // 绑定到状态机插槽
        // 将 CPU 中的内存数据复制到 GPU 优化的显存中，GL_STATIC_DRAW 表示数据不常修改
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);


        // 动态获取顶点属性在着色器中的入口位置
        GLint posLoc = glGetAttribLocation(shaderProgram, "pos");
        GLint clrLoc = glGetAttribLocation(shaderProgram, "clr");

        // 安全检查：如果名字拼错或者着色器里没用到该变量，OpenGL 会返回 -1
        if (posLoc == -1) {
		    std::cerr << "[Error] Vertex attribute 'pos' not found in shader." << std::endl; 
        }
        if (clrLoc == -1) {
		    std::cerr << "[Error] Vertex attribute 'clr' not found in shader." << std::endl;
        }

        // 告诉 OpenGL 如何解析顶点数据
        // 位置信息 
        if (posLoc != -1) { // 确保位置有效再配置
            // 参数含义：属性位置0 | 每次读取3个值 | 数据类型Float | 不归一化 | 步长7 * float | 起始偏移量0
            glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (GLvoid*)0);
            glEnableVertexAttribArray(posLoc); // 激活 location = 0 的顶点属性 
        }

	    // 颜色信息
        if (clrLoc != -1) {
            // 参数含义：属性位置1 | 每次读取4个值 | 数据类型Float | 不归一化 | 步长7*float | 起始偏移量3 * float（跳过前面的位置数据）
            glVertexAttribPointer(clrLoc, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (GLvoid*)(3 * sizeof(float)));
            glEnableVertexAttribArray(clrLoc);
        }
   
        // 解绑（非必须，为了保持状态机干净）
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // 正确的代码
        glBindVertexArray(0);

	    // 获取uniform变量的位置
	    int mvpLocation = glGetUniformLocation(shaderProgram, "mvp");
	    int alphaLocation = glGetUniformLocation(shaderProgram, "alpha");

        // 定义一个单位矩阵（Identity Matrix），让三角形保持原本大小，不进行任何旋转平移
        float identityMatrix[16] = {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };

        // =========
        // 渲染主循环
        // =========
        while (!glfwWindowShouldClose(window)) {
            // ESC退出
            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
                glfwSetWindowShouldClose(window, true);

		    float time = (float)glfwGetTime(); // 获取时间，单位秒

		    float redValue = (sin(time) * 0.5f) + 0.5f; 
		    float greenValue = (sin(time + 1.0f) * 0.5f) + 0.5f;
		    float blueValue = (sin(time + 2.0f) * 0.5f) + 0.5f;

            glClearColor(redValue, greenValue, blueValue, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            // 渲染三角形
            glUseProgram(shaderProgram);            // 使用刚才链接的着色器程序

		    glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, identityMatrix); // 设置 mvp uniform 变量
		    glUniform1f(alphaLocation, 1.0f); // 设置 alpha uniform 变量

            glBindVertexArray(VAO);                 // 绑定 VAO，自动恢复之前配置的所有 VBO 映射
            glDrawArrays(GL_TRIANGLES, 0, 3);       // 绘制图元：类型为三角形，从第0个顶点开始，绘制3个顶点

            // 双缓冲区交换与事件触发
            glfwSwapBuffers(window);
            glfwPollEvents();
        }

        // =======
        // 清理资源
        // =======
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteProgram(shaderProgram);

        glfwTerminate();
        return 0;
    }
