// SPDX-License-Identifier: MIT
/// @file Renderer.cpp
/// @brief 渲染器核心类的实现文件
/// @details 该文件实现了 Renderer 类的核心功能，包括初始化、渲染循环、资源管理等。
/// @author MaiX
/// @date 2026-08-01

#include "Renderer.h"

#include <iostream>
#include <fstream>
#include <sstream>

#include "lodepng.h"

/*
* @brief 构造函数，初始化成员变量默认值
*/
Renderer::Renderer()
{
    
}

/*
* @brief 析构函数，释放 OpenGL 资源
*/
Renderer::~Renderer()
{
	// 删除纹理对象
    if (m_DiffuseTexture) glDeleteTextures(1, &m_DiffuseTexture);
    if (m_SpecularTexture) glDeleteTextures(1, &m_SpecularTexture);

	// 删除顶点缓冲对象和顶点数组对象
    glDeleteBuffers(3, m_VBO);
    glDeleteVertexArrays(1, &m_VAO);

	// 删除着色器程序
    glDeleteProgram(m_ShaderProgram);
}

/*
* @brief 初始化 Renderer
*/
void Renderer::Init()
{
    // 开启深度测试
    glEnable(GL_DEPTH_TEST);

	// 编译着色器程序
	CompileShaders();

    // --- 新增：初始化 Gizmo ---
    // 注意：请确保 "assets/shaders/billboard.vert/frag" 路径下有你对应的着色器文件！
    m_LightGizmo.Init("assets/shaders/billboard.vert", "assets/shaders/billboard.frag");

    
}

/*
* @brief 开始一帧渲染
*/
void Renderer::BeginFrame()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 清除颜色与深度缓冲
}

/*
* @brief 渲染场景
*/
void Renderer::RenderScene(const cy::Matrix4f& mvp, const cy::Matrix4f& mv, const cy::Vec3f& lightPosView)
{
	// 使用着色器程序
    glUseProgram(m_ShaderProgram);

    // 1. 传递矩阵与光照 Uniform
    int mvpLoc = glGetUniformLocation(m_ShaderProgram, "mvp");
    int mvLoc = glGetUniformLocation(m_ShaderProgram, "mv");
    int lightPosLoc = glGetUniformLocation(m_ShaderProgram, "lightPos");

    glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, &mvp.cell[0]);
    glUniformMatrix4fv(mvLoc, 1, GL_FALSE, &mv.cell[0]);
    glUniform3f(lightPosLoc, lightPosView.x, lightPosView.y, lightPosView.z);

    // 2. 绑定纹理并传递标志位
    if (m_DiffuseTexture != 0) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_DiffuseTexture);
        glUniform1i(glGetUniformLocation(m_ShaderProgram, "texDiffuse"), 0);
        glUniform1i(glGetUniformLocation(m_ShaderProgram, "hasDiffuseMap"), 1);
    }
    else {
        glUniform1i(glGetUniformLocation(m_ShaderProgram, "hasDiffuseMap"), 0);
    }

    if (m_SpecularTexture != 0) {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_SpecularTexture);
        glUniform1i(glGetUniformLocation(m_ShaderProgram, "texSpecular"), 1);
        glUniform1i(glGetUniformLocation(m_ShaderProgram, "hasSpecularMap"), 1);
    }
    else {
        glUniform1i(glGetUniformLocation(m_ShaderProgram, "hasSpecularMap"), 0);
    }

	// 绑定 VAO
    glBindVertexArray(m_VAO);

	// 绘制三角形
    glDrawArrays(
        GL_TRIANGLES,
        0,
        m_VertexCount
    );
}

/*
* @brief 结束一帧渲染
*/
void Renderer::EndFrame()
{

}

/*
* @brief 上传模型数据到GPU
*/
void Renderer::SetMesh(
    const std::vector<cy::Vec3f>& vertices,
    const std::vector<cy::Vec3f>& normals,
    const std::vector<cy::Vec2f>& texCoords
)
{
	// 创建并绑定 VAO
    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);

	// 创建 VBO
    glGenBuffers( 3, m_VBO);

    // 数据上传GPU
    // position
    glBindBuffer(
        GL_ARRAY_BUFFER,
        m_VBO[0]
    );
    glBufferData(
        GL_ARRAY_BUFFER,
        vertices.size() * sizeof(cy::Vec3f),
        vertices.data(),
        GL_STATIC_DRAW
    );
    // normal
    glBindBuffer(
        GL_ARRAY_BUFFER,
        m_VBO[1]
    );
    glBufferData(
        GL_ARRAY_BUFFER,
        normals.size() * sizeof(cy::Vec3f),
        normals.data(),
        GL_STATIC_DRAW
    );
    // texcoord
    glBindBuffer(
        GL_ARRAY_BUFFER,
        m_VBO[2]
    );
    glBufferData(
        GL_ARRAY_BUFFER,
        texCoords.size() * sizeof(cy::Vec2f),
        texCoords.data(),
        GL_STATIC_DRAW
    );

    // 动态获取顶点属性在着色器中的入口位置
    GLint posLoc =
        glGetAttribLocation(
            m_ShaderProgram,
            "pos"
        );
    GLint normalLoc =
        glGetAttribLocation(
            m_ShaderProgram,
            "normal"
        );
    GLint texLoc =
        glGetAttribLocation(
            m_ShaderProgram,
            "texCoord"
        );

    // =======================================
    // 安全检查：如果名字拼错或者着色器里没用到该变量，OpenGL 会返回 -1
    // =======================================
    if (posLoc == -1) {
        std::cerr << "[Error] Vertex attribute 'pos' not found in shader." << std::endl;
    }
    if (normalLoc == -1) {
        std::cerr << "[Error] Vertex attribute 'normal' not found in shader." << std::endl;
    }
    if (texLoc == -1) {
        std::cerr << "[Error] Vertex attribute 'texCoord' not found in shader." << std::endl;
    }

    // 告诉 OpenGL 如何解析顶点数据
	// position
    if (posLoc != -1)
    {
        glBindBuffer(
            GL_ARRAY_BUFFER,
            m_VBO[0]
        );

        glVertexAttribPointer(
            posLoc,                     // 属性位置
			3,                          // 属性大小 (vec3)
			GL_FLOAT,                   // 数据类型
			GL_FALSE,                   // 是否归一化
			0,                          // 步长 (0 = 紧密排列)
            (GLvoid*)0                  // 偏移量
        );
		// 启用顶点属性数组
        glEnableVertexAttribArray(posLoc);
    }
	// normal
    if (normalLoc != -1)
    {
        glBindBuffer(
            GL_ARRAY_BUFFER,
            m_VBO[1]
        );

        glVertexAttribPointer(
            normalLoc,
            3,
            GL_FLOAT,
            GL_FALSE,
            0,
            (GLvoid*)0
        );

        glEnableVertexAttribArray(normalLoc);
    }
	// texcoord
    if (texLoc != -1)
    {
        glBindBuffer(
            GL_ARRAY_BUFFER,
            m_VBO[2]
        );

        glVertexAttribPointer(
            texLoc,
            2,
            GL_FLOAT,
            GL_FALSE,
            0,
            (GLvoid*)0
        );

        glEnableVertexAttribArray(texLoc);
    }

    // 解绑（非必须，为了保持状态机干净）
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    m_VertexCount =
        static_cast<int>(
            vertices.size()
            );
}

/*
* @brief 从文件中读取内容
*/
std::string Renderer::ReadFile(
    const std::string& path
)
{
    std::ifstream file(path);

    if (!file.is_open())
    {
        std::cerr
            << "[Shader File Error] Cannot read: "
            << path
            << std::endl;

        return "";
    }

    std::stringstream ss;
    ss << file.rdbuf();

    return ss.str();
}

void Renderer::CompileShaders()
{
    if (m_ShaderProgram != 0) glDeleteProgram(m_ShaderProgram); // 删除旧的着色器程序

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
    m_ShaderProgram = glCreateProgram();
    glAttachShader(m_ShaderProgram, vertexShader); // attach 顶点着色器
    glAttachShader(m_ShaderProgram, fragmentShader); // attach 片元着色器
    glLinkProgram(m_ShaderProgram);                  // link 整个程序

    // 链接完成后，单独的着色器对象就可以释放了
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    std::cout << "Shaders compiled successfully!" << std::endl;
}

// 内部 PNG 加载逻辑
GLuint Renderer::LoadTexturePNG(const std::string& filePath) {
    std::vector<unsigned char> image;
    unsigned width, height;
    unsigned error = lodepng::decode(image, width, height, filePath);

    if (error) {
        std::cerr << "[LodePNG Error] " << lodepng_error_text(error) << " File: " << filePath << std::endl;
        return 0;
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data());
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    return textureID;
}

// --- 新增：暴露给外部的纹理加载接口 ---
void Renderer::LoadTextures(const std::string& diffusePath, const std::string& specularPath) {
    m_DiffuseTexture = LoadTexturePNG(diffusePath);
    m_SpecularTexture = LoadTexturePNG(specularPath);
}

// --- 新增：重新加载着色器的接口 ---
void Renderer::ReloadShaders() {
    CompileShaders();
    std::cout << "[Renderer] Shaders reloaded successfully!" << std::endl;
}

// --- 新增：调用 LightGizmo 的 Draw 方法 ---
void Renderer::DrawLightGizmo(const cy::Matrix4f& proj, const cy::Matrix4f& view, const cy::Vec3f& lightWorldPos, float scale) {
    // 你的 LightGizmo::Draw 已经处理了禁用深度测试和开启混合，所以直接调用即可
    m_LightGizmo.Draw(proj, view, lightWorldPos, scale);
}
