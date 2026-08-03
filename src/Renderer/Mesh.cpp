// SPDX-License-Identifier: MIT
/// @file Mesh.cpp
/// @brief 
/// @details
/// @author MaiX
/// @date 


#include "Mesh.h"

#include <cstddef>
#include <limits>
#include <stdexcept>

Mesh::Mesh()
{
    CreateBuffers();
}

Mesh::~Mesh()
{
    if (m_VBO != 0) glDeleteBuffers(1, &m_VBO);
    if (m_VAO != 0) glDeleteVertexArrays(1, &m_VAO);
}

void Mesh::Upload(const std::vector<Vertex>& vertices)
{
    if (vertices.size() > static_cast<size_t>(std::numeric_limits<int>::max())) {
        throw std::length_error("Mesh vertex count exceeds OpenGL draw range");
    }

    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex)),
        vertices.empty() ? nullptr : vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    m_VertexCount = static_cast<int>(vertices.size());
}

void Mesh::Draw() const
{
    if (m_VertexCount == 0) return;

    glBindVertexArray(m_VAO);
    glDrawArrays(GL_TRIANGLES, 0, m_VertexCount);
    glBindVertexArray(0);
}

int Mesh::GetVertexCount() const
{
    return m_VertexCount;
}

void Mesh::CreateBuffers()
{
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);

    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    SetupVertexAttributes();
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Mesh::SetupVertexAttributes()
{
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        reinterpret_cast<void*>(offsetof(Vertex, Position)));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        reinterpret_cast<void*>(offsetof(Vertex, Normal)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        reinterpret_cast<void*>(offsetof(Vertex, TexCoord)));
    glEnableVertexAttribArray(2);
}
