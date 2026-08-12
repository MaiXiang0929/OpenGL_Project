// SPDX-License-Identifier: MIT
/// @file Material.cpp
/// @brief Material 类的实现文件
/// @details 该文件实现了 Material 类的成员函数，用于绑定材质属性和纹理到着色器。
/// @author MaiX
/// @date 2026-08-11


#include "Material.h"

#include <utility>

#include "Shader.h"

void Material::Bind(Shader& shader, unsigned int firstTextureUnit) const
{
    shader.SetVec3(
        "material.baseColor",
        m_Properties.baseColor.x,
        m_Properties.baseColor.y,
        m_Properties.baseColor.z);
    shader.SetVec3(
        "material.specularColor",
        m_Properties.specularColor.x,
        m_Properties.specularColor.y,
        m_Properties.specularColor.z);
    shader.SetFloat("material.shininess", m_Properties.shininess);
    shader.SetFloat(
        "material.environmentReflectivity",
        m_Properties.environmentReflectivity);
    shader.SetFloat("material.metallic", m_Properties.metallic);
    shader.SetFloat("material.roughness", m_Properties.roughness);
    shader.SetFloat("material.ambientOcclusion", m_Properties.ambientOcclusion);

    const unsigned int albedoUnit = firstTextureUnit;
    const unsigned int specularUnit = firstTextureUnit + 1;
    const unsigned int normalUnit = firstTextureUnit + 2;
    const unsigned int displacementUnit = firstTextureUnit + 3;

    shader.SetInt("material.hasAlbedoMap", m_AlbedoMap ? 1 : 0);
    if (m_AlbedoMap)
    {
        m_AlbedoMap->Bind(albedoUnit);
        shader.SetInt("material.albedoMap", static_cast<int>(albedoUnit));
    }

    shader.SetInt("material.hasSpecularMap", m_SpecularMap ? 1 : 0);
    if (m_SpecularMap)
    {
        m_SpecularMap->Bind(specularUnit);
        shader.SetInt("material.specularMap", static_cast<int>(specularUnit));
    }

    shader.SetInt("material.hasNormalMap", m_NormalMap ? 1 : 0);
    if (m_NormalMap)
    {
        m_NormalMap->Bind(normalUnit);
        shader.SetInt("material.normalMap", static_cast<int>(normalUnit));
    }

    BindDisplacement(shader, displacementUnit);
}

void Material::SetAlbedoMap(std::shared_ptr<Texture2D> texture)
{
    m_AlbedoMap = std::move(texture);
}

void Material::SetSpecularMap(std::shared_ptr<Texture2D> texture)
{
    m_SpecularMap = std::move(texture);
}

void Material::SetNormalMap(std::shared_ptr<Texture2D> texture)
{
    m_NormalMap = std::move(texture);
}

void Material::SetDisplacementMap(std::shared_ptr<Texture2D> texture)
{
    m_DisplacementMap = std::move(texture);
}

void Material::BindDisplacement(Shader& shader, unsigned int textureUnit) const
{
    const int hasDisplacement = m_DisplacementMap ? 1 : 0;
    shader.SetInt("material.hasDisplacementMap", hasDisplacement);
    shader.SetInt("hasDisplacementMap", hasDisplacement);
    if (!m_DisplacementMap)
        return;

    m_DisplacementMap->Bind(textureUnit);
    shader.SetInt("material.displacementMap", static_cast<int>(textureUnit));
    shader.SetInt("displacementMap", static_cast<int>(textureUnit));
}
