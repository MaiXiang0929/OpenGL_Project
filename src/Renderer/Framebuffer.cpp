// SPDX-License-Identifier: MIT
/// @file Framebuffer.cpp
/// @brief 帧缓冲区管理类的实现文件
/// @author MaiX
/// @date 2026-08-04

#include "Framebuffer.h"
#include <iostream>

Framebuffer::~Framebuffer() {
    Cleanup();
}

void Framebuffer::Init(int width, int height) {
    if (m_Width == width && m_Height == height && m_FBO != 0) return;

    Cleanup(); // 清理旧数据（防止 Resize 时内存泄漏）

    m_Width = width;
    m_Height = height;

    // 1. 创建 FBO
    glGenFramebuffers(1, &m_FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);

    // 2. 创建 Color Texture Attachment
    glGenTextures(1, &m_ColorTexture);
    glBindTexture(GL_TEXTURE_2D, m_ColorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_Width, m_Height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ColorTexture, 0);

    // 3. 创建 Depth & Stencil Renderbuffer Attachment (RBO)
    glGenRenderbuffers(1, &m_RBO);
    glBindRenderbuffer(GL_RENDERBUFFER, m_RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_Width, m_Height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_RBO);

    // 4. 检查 FBO 完整性
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "[Error] Framebuffer is not complete!" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::Cleanup() {
    if (m_RBO != 0) { glDeleteRenderbuffers(1, &m_RBO); m_RBO = 0; }
    if (m_ColorTexture != 0) { glDeleteTextures(1, &m_ColorTexture); m_ColorTexture = 0; }
    if (m_FBO != 0) { glDeleteFramebuffers(1, &m_FBO); m_FBO = 0; }
}

void Framebuffer::Bind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
    glViewport(0, 0, m_Width, m_Height);
}

void Framebuffer::Unbind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}