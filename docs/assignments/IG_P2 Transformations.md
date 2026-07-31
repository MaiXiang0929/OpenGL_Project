## Requirements
[项目要求](https://graphics.cs.utah.edu/courses/cs6610/spring2021/?prj=2)

---
## 准备工作
### 准备依赖库 (libs)
课程强烈建议使用 `cyCodeBase`。你需要下载[cyCodeBase](https://github.com/cemyuksel/cyCodeBase)，并放入你的 `libs/` 文件夹中

### 准备obj文件
下载[teapot.obj](https://graphics.cs.utah.edu/courses/cs6610/spring2021/prj02/teapot.obj)
新建txt文件，将上述链接内容复制到txt中，并将文件名及扩展名改为teapot.obj

## 代码
### 修改shader
在：

```text
shaders/triangle.vert
```

写：

```glsl
#version 330 core

layout (location = 0) in vec3 pos;

uniform mat4 mvp;

void main()
{
	gl_Position = mvp * vec4(pos, 1.0);
}
```

在：

```text
shaders/triangle.frag
```

写：

```glsl
#version 330 core

layout(location = 0) out vec4 color;

void main()
{
	color = vec4(1.0, 1.0, 1.0, 1.0);
}
```

在：
```text
src/main.cpp
```

写：

```cpp
    #include <iostream>
    // 注意：GLAD 必须在 GLFW 之前包含，它会引入所需的 OpenGL 头文件
    #include <glad/glad.h> 
    #include <GLFW/glfw3.h>
    #include <fstream>      // 用于读取着色器文件
    #include <sstream>      // 用于将文件内容转换成字符串
    #include <vector>

	// cyCodeBase 头文件
    #include "cyMatrix.h"
    #include "cyTriMesh.h"

    // 屏幕宽高
    const unsigned int SCR_WIDTH = 1920;
    const unsigned int SCR_HEIGHT = 1080;

    // 全局变量
	float rotX = 0.0f, rotY = 0.0f; // 旋转角度
	float cameraDistance = 50.0f; // 相机距离
	double lastX, lastY; // 上一次鼠标位置
	bool leftDown = false, rightDown = false; // 鼠标按键状态
	bool isPerspective = true; // 是否使用透视投影
	GLuint shaderProgram = 0; // 着色器程序对象


    /// <summary>
    /// 窗口大小改变时的回调函数，用于动态更新 OpenGL 视口
    /// </summary>
    /// <param name="window">触发该事件的 GLFW 窗口句柄</param>
    /// <param name="width">新的窗口宽度</param>
    /// <param name="height">新的窗口高度</param>
    void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
        glViewport(0, 0, width, height);
    }

    /// <summary>
    /// 读取文件内容的函数
    /// </summary>
    /// <param name="filePath">着色器文件的相对或绝对路径</param>
    /// <returns>以字符串形式返回的文件源码，若读取失败返回空字符串</returns>
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

    /// <summary>
	/// 着色器编译函数，负责读取、编译顶点和片元着色器，并链接成一个着色器程序
    /// </summary>
    void CompileShaders() {
		if (shaderProgram != 0) glDeleteProgram(shaderProgram); // 删除旧的着色器程序

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
        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader); // attach 顶点着色器
        glAttachShader(shaderProgram, fragmentShader); // attach 片元着色器
        glLinkProgram(shaderProgram);                  // link 整个程序

        // 链接完成后，单独的着色器对象就可以释放了
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        std::cout << "Shaders compiled successfully!" << std::endl;
    }

    /// <summary>
	/// 鼠标按键回调函数，用于处理鼠标点击事件
    /// </summary>
    /// <param name="window">触发该事件的GLFW窗口句柄</param>
    /// <param name="button">按下的鼠标按键</param>
    /// <param name="action">按键动作</param>
    /// <param name="mods">修饰键状态</param>
    void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
			if (action == GLFW_PRESS) leftDown = true;
			else if (action == GLFW_RELEASE) leftDown = false;
        }
        if (button == GLFW_MOUSE_BUTTON_RIGHT) {
            if (action == GLFW_PRESS) rightDown = true;
            else if (action == GLFW_RELEASE) rightDown = false;
        }
    }

    /// <summary>
	/// 鼠标移动回调函数，用于处理鼠标拖拽事件，左键旋转，右键缩放
    /// </summary>
    /// <param name="window">触发该事件的GLFW窗口句柄</param>
    /// <param name="xpos">当前光标的X坐标</param>
    /// <param name="ypos">当前光标的Y坐标</param>
    void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
		double dx = xpos - lastX;
		double dy = ypos - lastY;
		lastX = xpos;
        lastY = ypos;

		// 左键旋转
        if (leftDown) {
            rotX += (float)dy * 0.01f;
			rotY += (float)dx * 0.01f;
        }

		// 右键缩放
        if (rightDown) {
			cameraDistance += (float)dy * 0.1f; // 鼠标垂直移动控制缩放
			if (cameraDistance < 0.1f) cameraDistance = 0.1f; // 防止相机距离过近
        }
    }

    /// <summary>
    /// 键盘输入回调函数，用于处理着色器重载（F6）、投影模式切换（P）、退出（ESC）
    /// </summary>
    /// <param name="window">触发该事件的GLFW窗口句柄</param>
    /// <param name="key">被按下的键盘按键</param>
    /// <param name="scancode">平台相关的按键扫描码</param>
    /// <param name="action">按键动作</param>
    /// <param name="mods">修饰键状态</param>
    void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        // 着色器重载（F6）
        if (key == GLFW_KEY_F6 && action == GLFW_PRESS) {
            CompileShaders();
        }

		// 投影模式切换（P）
        if (key == GLFW_KEY_P && action == GLFW_PRESS) {
			isPerspective = !isPerspective;
        }

		// 退出（ESC）
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
			glfwSetWindowShouldClose(window, true); 
        }
    }

    /// <summary>
    /// 程序主入口，负责解析参数、初始化环境、加载模型、配置渲染管线并启动主循环。
    /// </summary>
    /// <param name="argc">命令行参数数量</param>
    /// <param name="argv">命令行参数数组，argv[1] 必须是待加载的 .obj 模型的路径</param>
    /// <returns>int 正常退出返回 0，初始化失败或参数错误返回 -1</returns>
    int main(int argc, char** argv) {
		// 检查命令行参数
        if (argc < 2) {
			std::cerr << "Usage: " << argv[0] << " <path_to_obj_file>" << std::endl;
            return -1;
        }

        std::string objPath = argv[1];

        // 模型数据加载、包围盒计算
        cy::TriMesh mesh;
        if (!mesh.LoadFromFileObj(objPath.c_str())) {
            std::cerr << "Failed to load obj: " << objPath << std::endl;
            return -1;
        }
        mesh.ComputeBoundingBox();
        cy::Vec3f objCenter = (mesh.GetBoundMax() + mesh.GetBoundMin()) * 0.5f;

        // 初始化 GLFW 窗口并配置 OpenGL 上下文
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

        // 注册输入回调
        glfwSetMouseButtonCallback(window, mouse_button_callback);
        glfwSetCursorPosCallback(window, cursor_position_callback);
        glfwSetKeyCallback(window, key_callback);
        glfwGetCursorPos(window, &lastX, &lastY);

        // 编译着色器
        CompileShaders();

        // 配置顶点数组 (VAO) 与顶点缓冲 (VBO)
        GLuint VAO, VBO;

        // 必须最先创建并绑定 VAO
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO); // 激活 VAO 状态，后续的 VBO 关系及属性指针都会被它“记录”下来

        // 创建并绑定 VBO
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO); // 绑定到状态机插槽
        //  将 cyTriMesh 的顶点数据传给 GPU (NV 表示顶点数量)
        glBufferData(GL_ARRAY_BUFFER, sizeof(cy::Vec3f) * mesh.NV(), &mesh.V(0), GL_STATIC_DRAW);


        // 动态获取顶点属性在着色器中的入口位置
        GLint posLoc = glGetAttribLocation(shaderProgram, "pos");
        //GLint clrLoc = glGetAttribLocation(shaderProgram, "clr");

        // 安全检查：如果名字拼错或者着色器里没用到该变量，OpenGL 会返回 -1
        if (posLoc == -1) {
		    std::cerr << "[Error] Vertex attribute 'pos' not found in shader." << std::endl; 
        }
    //    if (clrLoc == -1) {
		  //  std::cerr << "[Error] Vertex attribute 'clr' not found in shader." << std::endl;
    //    }

        // 告诉 OpenGL 如何解析顶点数据
        // 位置信息 
        if (posLoc != -1) { // 确保位置有效再配置
            // 参数含义：属性位置0 | 每次读取3个值 | 数据类型Float | 不归一化 | 步长 | 起始偏移量0
            glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, sizeof(cy::Vec3f), (GLvoid*)0);
            glEnableVertexAttribArray(posLoc); // 激活 location = 0 的顶点属性 
        }

	   // // 颜色信息
    //    if (clrLoc != -1) {
    //        // 参数含义：属性位置1 | 每次读取4个值 | 数据类型Float | 不归一化 | 步长7*float | 起始偏移量3 * float（跳过前面的位置数据）
    //        glVertexAttribPointer(clrLoc, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (GLvoid*)(3 * sizeof(float)));
    //        glEnableVertexAttribArray(clrLoc);
    //    }
   
        // 解绑（非必须，为了保持状态机干净）
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        // 开启深度测试，防止 3D 模型渲染出现前后遮挡错误
        glEnable(GL_DEPTH_TEST);

        // =========
        // 渲染主循环
        // =========
        while (!glfwWindowShouldClose(window)) {
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 清除颜色与深度缓冲

            glUseProgram(shaderProgram);            // 使用刚才链接的着色器程序

			// 计算 MVP 矩阵
			float aspect = (float)SCR_WIDTH / (float)SCR_HEIGHT;
			cy::Matrix4f projMatrix;

            if (isPerspective) {
                projMatrix.SetPerspective(45.0f * (3.14159f / 180.0f), aspect, 0.1f, 1000.0f);
            } else {
				float scale = 1.0f / cameraDistance;
                projMatrix = cy::Matrix4f::Scale(cy::Vec3f(scale / aspect, scale, 0.1f));
            }

			// View 矩阵，沿Z轴负方向平移相机
            cy::Matrix4f viewMatrix = cy::Matrix4f::Translation(cy::Vec3f(0, 0, -cameraDistance));

			// Model 矩阵，旋转 + 居中
            cy::Matrix4f modelMatrix = cy::Matrix4f::RotationX(rotX) * cy::Matrix4f::RotationY(rotY);
            modelMatrix *= cy::Matrix4f::Translation(-objCenter); // 减去包围盒中心，使物体居于原点

            cy::Matrix4f mvp = projMatrix * viewMatrix * modelMatrix;

            // mvp传入shader
            int mvpLocation = glGetUniformLocation(shaderProgram, "mvp");
		    glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, &mvp.cell[0]); // 设置 mvp uniform 变量

            // 执行绘制
            glBindVertexArray(VAO);                 // 绑定 VAO，自动恢复之前配置的所有 VBO 映射
            glDrawArrays(GL_POINTS, 0, mesh.NV());       // 绘制图元：类型为三角形，从第0个顶点开始，绘制3个顶点

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

```

在：

```text
CMakeLists.txt
```

写：

```cmake
cmake_minimum_required(VERSION 3.15)

project(OpenGL_Project)

set(CMAKE_CXX_STANDARD 17)

# 强制 MSVC 编译器使用 UTF-8 编码读取源文件
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

include_directories(
    libs/glfw/include
    libs/glad/include
    libs/cyCodeBase
)

add_executable(OpenGL_Project
    src/main.cpp
    libs/glad/src/glad.c
 "src/shader.h")

# GLFW库路径（注意版本可能不同）
set(GLFW_LIB ${CMAKE_SOURCE_DIR}/libs/glfw/lib-vc2022/glfw3.lib)

target_link_libraries(OpenGL_Project
    ${GLFW_LIB}
    opengl32
)

# 编译后自动将源码目录下的 assets 文件夹拷贝到可执行文件输出目录
add_custom_command(TARGET OpenGL_Project POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:OpenGL_Project>/assets
)
```

## 运行
### step1：打开命令行

1. 打开 Windows 资源管理器，进入你的输出目录： `~\OpenGL_Project\out\build\x64-Debug\`
    
2. 在上方地址栏输入 `cmd`，然后按回车。
    
3. 这将直接在该目录下打开一个命令行窗口。
    

### step2：运行程序

在弹出的黑框（命令行）中，输入以下命令
```
OpenGL_Project.exe assets/models/teapot.obj
```