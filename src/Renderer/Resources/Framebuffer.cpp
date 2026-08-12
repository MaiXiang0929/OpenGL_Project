// SPDX-License-Identifier: MIT
/// @file Framebuffer.cpp
/// @brief 帧缓冲区管理类的实现文件
/// @details 封装 OpenGL Framebuffer Object (FBO)，提供离屏渲染与纹理附着能力。
/// @author MaiX
/// @date 2026-08-04

#include "Framebuffer.h"
#include <cstring>
#include <iostream>

// 匿名命名空间中的名称只在当前 Framebuffer.cpp 文件内可见，其他 .cpp 文件无法访问
namespace {
    // GLAD 仅生成核心 API，因此在本文件中补充各向异性过滤扩展枚举值。
    constexpr GLenum TextureMaxAnisotropyExt = 0x84FE;          // 设置某张纹理的各向异性等级
    constexpr GLenum MaxTextureMaxAnisotropyExt = 0x84FF;       // 查询显卡支持的最大等级

    // 各向异性过滤不是 OpenGL 4.0 核心功能，设置参数前必须检查驱动扩展列表。
    bool SupportsAnisotropicFiltering() {
        GLint extensionCount = 0;
        glGetIntegerv(GL_NUM_EXTENSIONS, &extensionCount);
        for (GLint i = 0; i < extensionCount; ++i) {
            const char* extension = reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, i));
            if (extension != nullptr &&
                std::strcmp(extension, "GL_EXT_texture_filter_anisotropic") == 0) {
                return true;
            }
        }
        return false;
    }
}

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

    // 先创建完整的 MipMap 链，保证使用 MipMap 过滤时纹理仍然完整。
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // 驱动支持时使用硬件允许的最大各向异性等级，改善倾斜平面的缩小采样质量。
    if (SupportsAnisotropicFiltering()) {
        GLfloat maxAnisotropy = 1.0f;
        glGetFloatv(MaxTextureMaxAnisotropyExt, &maxAnisotropy);
        glTexParameterf(GL_TEXTURE_2D, TextureMaxAnisotropyExt, maxAnisotropy);
    }
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

void Framebuffer::GenerateMipmaps() const {
    // 颜色附件每帧都会被重绘，旧的低分辨率层级必须随之更新。
    glBindTexture(GL_TEXTURE_2D, m_ColorTexture);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
}
