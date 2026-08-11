// 片段着色器（Fragment Shader）— 带环境反射的 Blinn-Phong 着色

#version 330 core

in vec3 fragPos;
in vec3 fragNormal;
in vec2 fragTexCoord;
in vec4 fragLightSpacePos;

layout(location = 0) out vec4 color;

uniform vec3 lightPos;
uniform sampler2D texDiffuse;
uniform sampler2D texSpecular;
uniform bool hasDiffuseMap;
uniform bool hasSpecularMap;

// --- 环境反射 ---
uniform samplerCube cubemap;
uniform mat3 viewToWorld;

// 深度纹理由 OpenGL 硬件执行“参考深度 <= 已记录深度”的比较。
uniform sampler2DShadow shadowMap;
uniform bool shadowsEnabled;

float CalculateShadowVisibility(vec4 lightSpacePos, vec3 normal, vec3 lightDir)
{
    // 关闭时不访问深度纹理，保持原有光照结果。
    if (!shadowsEnabled) {
        return 1.0;
    }

    if (lightSpacePos.w <= 0.0) {
        return 0.0;
    }

    // OpenGL 裁剪空间 [-1, 1] 映射到深度纹理空间 [0, 1]。
    vec3 projected = lightSpacePos.xyz / lightSpacePos.w;
    projected = projected * 0.5 + 0.5;

    // 透视投影范围即聚光灯光锥，范围外没有直射光。
    if (projected.x < 0.0 || projected.x > 1.0 ||
        projected.y < 0.0 || projected.y > 1.0 ||
        projected.z < 0.0 || projected.z > 1.0) {
        return 0.0;
    }

    // 斜向表面使用更大的 bias，兼顾自阴影与 shadow acne。
    float bias = max(0.0025 * (1.0 - dot(normal, lightDir)), 0.0005);
    vec2 texelSize = 1.0 / vec2(textureSize(shadowMap, 0));
    float visibility = 0.0;
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            vec2 offset = vec2(x, y) * texelSize;
            visibility += texture(
                shadowMap,
                vec3(projected.xy + offset, projected.z - bias));
        }
    }
    return visibility / 9.0;
}

void main()
{
    // 材质与光源参数（硬编码）
    vec3 ambientColor = vec3(0.1, 0.1, 0.15);  // 环境光底色
    vec3 diffuseColor = hasDiffuseMap ? texture(texDiffuse, fragTexCoord).rgb : vec3(1.0, 0.0, 0.0);
    vec3 specularColor = hasSpecularMap ? texture(texSpecular, fragTexCoord).rgb : vec3(1.0, 1.0, 1.0);
    float shininess = 64.0;                    // 高光系数大小
    vec3 lightIntensity = vec3(1.0, 1.0, 1.0); // 光源强度

    vec3 norm = normalize(fragNormal);

    // Ambient
    vec3 ambient = ambientColor * diffuseColor * lightIntensity;

    // Diffuse
    vec3 lightDir = normalize(lightPos - fragPos);
    float visibility = CalculateShadowVisibility(
        fragLightSpacePos, norm, lightDir);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = visibility * diff * diffuseColor * lightIntensity;

    // Specular (镜面高光 - Blinn)
    vec3 viewDir = normalize(-fragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), shininess);
    vec3 specular = visibility * spec * specularColor * lightIntensity;

    // --- 环境反射 ---
    // 计算 view space 中的反射方向，再变换到 world space 采样 cubemap
    vec3 reflectDirView = reflect(-viewDir, norm);
    vec3 reflectDirWorld = viewToWorld * reflectDirView;
    vec3 envReflection = texture(cubemap, reflectDirWorld).rgb;

    // 结合 Blinn-Phong 与环境反射：
    // 用高光贴图的亮度控制反射强度（模拟金属/光滑表面反射率）
    float reflectivity = hasSpecularMap ? texture(texSpecular, fragTexCoord).r : 0.5;
    vec3 finalColor = ambient + diffuse + reflectivity * envReflection;

    color = vec4(finalColor, 1.0);
}
