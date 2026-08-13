// SPDX-License-Identifier: MIT
#include "OpenGLStateCache.h"

OpenGLStateCache& OpenGLStateCache::Get()
{
    static OpenGLStateCache instance;
    return instance;
}

void OpenGLStateCache::Invalidate()
{
    m_VertexArrayKnown = false;
}

void OpenGLStateCache::BindVertexArray(GLuint vao)
{
    if (m_VertexArrayKnown && m_VertexArray == vao)
        return;

    glBindVertexArray(vao);
    m_VertexArray = vao;
    m_VertexArrayKnown = true;
}
