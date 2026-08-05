// 片段着色器（Fragment Shader）— 带环境反射的 Blinn-Phong 着色

#version 330 core

in vec3 fragPos;
in vec3 fragNormal;
in vec2 fragTexCoord;

layout(location = 0) out vec4 color;

uniform vec3 lightPos;
uniform sampler2D texDiffuse;
uniform sampler2D texSpecular;
uniform bool hasDiffuseMap;
uniform bool hasSpecularMap;

// --- 环境反射 ---
uniform samplerCube cubemap;
uniform mat3 viewToWorld;

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
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * diffuseColor * lightIntensity;

    // Specular (镜面高光 - Blinn)
    vec3 viewDir = normalize(-fragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), shininess);
    vec3 specular = spec * specularColor * lightIntensity;

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
