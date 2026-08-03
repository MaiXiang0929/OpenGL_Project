#pragma once

#include <vector>
#include <glad/glad.h>
#include "cyVector.h"

/// @brief 顶点结构体，包含位置、法线和纹理坐标
struct Vertex {
	cy::Vec3f Position;
	cy::Vec3f Normal;
	cy::Vec2f TexCoord;
};

class Mesh {

public:

    Mesh();

    ~Mesh();

	// 禁止复制
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    void Upload(
        const std::vector<Vertex>& vertices
    );

    void Draw() const;

    int GetVertexCount() const;
 

private:

    GLuint m_VAO = 0;

    GLuint m_VBO = 0;

    int m_VertexCount = 0;

private:

    /// @brief 创建 OpenGL Buffer
    void CreateBuffers();

    /// @brief 设置 Vertex Attribute Layout
    void SetupVertexAttributes();
};