// SPDX-License-Identifier: MIT
#pragma once

#include <glad/glad.h>

#include "cyMatrix.h"
#include "cyVector.h"

class CubemapTexture;
class Material;
class Mesh;
struct TessellationSettings;

enum class RenderPassType
{
    Shadow,
    Reflection,
    Forward,
    Present
};

/// @brief Application 每帧提交给渲染器的纯场景数据。
/// @details 这里只保存矩阵、光源和视口等 CPU 数据，不持有任何 OpenGL 资源。
struct RenderFrameData
{
    unsigned int viewportWidth = 0;
    unsigned int viewportHeight = 0;

    cy::Matrix4f projection;
    cy::Matrix4f view;
    cy::Matrix4f model;
    cy::Matrix4f mvp;
    cy::Matrix4f mv;

    cy::Matrix4f lightVP;
    cy::Matrix4f lightMvp;
    cy::Vec3f lightWorldPosition;
    cy::Vec3f lightPositionView;

    cy::Matrix4f reflectionView;
    cy::Matrix4f reflectionMvp;
    cy::Matrix4f reflectionMv;
    cy::Vec3f reflectionLightPositionView;

    cy::Matrix4f groundMvp;
    cy::Matrix4f groundModel;
    cy::Matrix4f reflectionVP;
    cy::Vec3f cameraWorldPosition;

    cy::Matrix4f lightObjectMvp;
    cy::Matrix4f presentMvp;

    bool shadowsEnabled = true;
};

/// @brief RenderPipeline 在各 Pass 之间共享的执行上下文。
/// @details Renderer 负责填入资源引用；Pass 只读取资源并提交对应的 GPU 命令。
struct RenderPassContext
{
    RenderFrameData& frame;

    Mesh& sceneMesh;
    Mesh& lightMesh;
    Mesh& presentMesh;
    Mesh& skyboxMesh;
    Mesh& groundMesh;
    Material& material;
    CubemapTexture& cubemap;
    TessellationSettings& tessellation;

    // 前序 Pass 生成、后序 Pass 消费的 GPU 纹理句柄。
    GLuint shadowTexture = 0;
    GLuint reflectionTexture = 0;
};

/// @brief 所有真实渲染阶段的统一接口。
class RenderPass
{
public:
    virtual ~RenderPass() = default;
    virtual RenderPassType GetType() const = 0;
    virtual void Execute(RenderPassContext& context) = 0;
};
