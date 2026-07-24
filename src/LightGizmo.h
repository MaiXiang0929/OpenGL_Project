#pragma once

#include <glad/glad.h>
#include "cyMatrix.h"
#include <string>

class LightGizmo {
public:
    LightGizmo();
    ~LightGizmo();

    // 初始化：加载着色器并构建面片 VAO/VBO
    void Init(const std::string& vertPath, const std::string& fragPath);

    // 渲染光源 Gizmo
    void Draw(const cy::Matrix4f& proj, const cy::Matrix4f& view, const cy::Vec3f& lightWorldPos, float scale);

private:
    GLuint VAO = 0;
    GLuint VBO = 0;
    GLuint shaderProgram = 0;

    std::string ReadFile(const std::string& filePath);
    void CompileShaders(const std::string& vertPath, const std::string& fragPath);
};