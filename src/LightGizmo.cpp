#include "LightGizmo.h"
#include <fstream>
#include <sstream>
#include <iostream>

LightGizmo::LightGizmo() {}

LightGizmo::~LightGizmo() {
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
    if (shaderProgram) glDeleteProgram(shaderProgram);
}

std::string LightGizmo::ReadFile(const std::string& filePath) {
    std::ifstream file;
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        file.open(filePath);
        std::stringstream ss;
        ss << file.rdbuf();
        file.close();
        return ss.str();
    }
    catch (std::ifstream::failure& e) {
        std::cerr << "[LightGizmo Shader Error] Cannot read: " << filePath << std::endl;
        return "";
    }
}

void LightGizmo::CompileShaders(const std::string& vertPath, const std::string& fragPath) {
    std::string vCodeStr = ReadFile(vertPath);
    const char* vCode = vCodeStr.c_str();
    GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertShader, 1, &vCode, nullptr);
    glCompileShader(vertShader);

    // 检查顶点着色器编译错误
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vertShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertShader, 512, NULL, infoLog);
        std::cerr << "[LightGizmo] Vertex Shader Compilation Failed:\n" << infoLog << std::endl;
    }

    std::string fCodeStr = ReadFile(fragPath);
    const char* fCode = fCodeStr.c_str();
    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader, 1, &fCode, nullptr);
    glCompileShader(fragShader);

    // 检查片元着色器编译错误
    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragShader, 512, NULL, infoLog);
        std::cerr << "[LightGizmo] Fragment Shader Compilation Failed:\n" << infoLog << std::endl;
    }

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertShader);
    glAttachShader(shaderProgram, fragShader);
    glLinkProgram(shaderProgram);

    // 检查程序链接错误
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "[LightGizmo] Shader Program Linking Failed:\n" << infoLog << std::endl;
    }

    glDeleteShader(vertShader);
    glDeleteShader(fragShader);
}

void LightGizmo::Init(const std::string& vertPath, const std::string& fragPath) {
    CompileShaders(vertPath, fragPath);

    // 准备一个 2D 矩形面片顶点数据 (位置 + UV)
    float billboardVertices[] = {
        -0.5f,  0.5f, 0.0f,    0.0f, 1.0f,
        -0.5f, -0.5f, 0.0f,    0.0f, 0.0f,
         0.5f, -0.5f, 0.0f,    1.0f, 0.0f,

        -0.5f,  0.5f, 0.0f,    0.0f, 1.0f,
         0.5f, -0.5f, 0.0f,    1.0f, 0.0f,
         0.5f,  0.5f, 0.0f,    1.0f, 1.0f
    };

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(billboardVertices), billboardVertices, GL_STATIC_DRAW);

    // 顶点位置属性 (Location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // 纹理坐标属性 (Location = 1)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void LightGizmo::Draw(const cy::Matrix4f& proj, const cy::Matrix4f& view, const cy::Vec3f& lightWorldPos, float scale) {
    // 关闭深度测试、让光源图标永远显示在最上层
    glDisable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUseProgram(shaderProgram);

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "proj"), 1, GL_FALSE, &proj.cell[0]);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, &view.cell[0]);
    glUniform3f(glGetUniformLocation(shaderProgram, "lightWorldPos"), lightWorldPos.x, lightWorldPos.y, lightWorldPos.z);
    glUniform1f(glGetUniformLocation(shaderProgram, "scale"), scale);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    // 恢复 OpenGL 状态
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}