// SPDX-License-Identifier: MIT
/// @file Material.h
/// @brief Material 类的声明文件
/// @details 该文件声明了 Material 类及其相关结构体，用于管理材质属性和纹理。
/// @author MaiX
/// @date 2026-08-11


#pragma once

#include <memory>

#include "cyVector.h"

#include "Renderer/Resources/Texture2D.h"

class Shader;

struct MaterialProperties
{
    cy::Vec3f baseColor{ 1.0f, 1.0f, 1.0f };
    cy::Vec3f specularColor{ 1.0f, 1.0f, 1.0f };
    float shininess = 64.0f;
    float environmentReflectivity = 0.5f;

    // Cook-Torrance metallic-roughness inputs consumed by the forward shader.
    float metallic = 0.0f;
    float roughness = 0.5f;
    float ambientOcclusion = 1.0f;
    // 透明 Pass 使用的整体不透明度；管线分类仍由 Primitive 单独决定。
    float opacity = 1.0f;
};

class Material
{
public:
    void Bind(Shader& shader, unsigned int firstTextureUnit = 0) const;

    MaterialProperties& GetProperties() { return m_Properties; }
    const MaterialProperties& GetProperties() const { return m_Properties; }

    void SetAlbedoMap(std::shared_ptr<Texture2D> texture);
    void SetSpecularMap(std::shared_ptr<Texture2D> texture);
    void SetNormalMap(std::shared_ptr<Texture2D> texture);
    void SetDisplacementMap(std::shared_ptr<Texture2D> texture);
    void BindDisplacement(Shader& shader, unsigned int textureUnit) const;

private:
    MaterialProperties m_Properties;
    std::shared_ptr<Texture2D> m_AlbedoMap;
    std::shared_ptr<Texture2D> m_SpecularMap;
    std::shared_ptr<Texture2D> m_NormalMap;
    std::shared_ptr<Texture2D> m_DisplacementMap;
};
