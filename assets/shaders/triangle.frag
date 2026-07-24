#version 330 core

in vec3 fragPos;
in vec3 fragNormal;

layout(location = 0) out vec4 color;

uniform vec3 lightPos;

void main()
{
	// 材质与光源参数（硬编码）
	vec3 ambientColor = vec3(0.1, 0.1, 0.15);  // 环境光底色
    vec3 diffuseColor = vec3(1.0, 0.0, 0.0);   // 物体表面漫反射颜色(红)
    vec3 specularColor = vec3(1.0, 1.0, 1.0);  // 高光颜色(纯白)
    float shininess = 64.0;                    // 高光系数大小
    vec3 lightIntensity = vec3(1.0, 1.0, 1.0); // 光源强度

    vec3 norm =  normalize(fragNormal);

    // Ambient
    vec3 ambient = ambientColor * lightIntensity;

    // Diffuse
    vec3 lightDir = normalize(lightPos - fragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * diffuseColor * lightIntensity;

    // Specular (镜面高光 - Blinn)
    vec3 viewDir = normalize(-fragPos); 
    vec3 halfwayDir = normalize(lightDir + viewDir); 
    float spec = pow(max(dot(norm, halfwayDir), 0.0), shininess);
    vec3 specular = spec * specularColor * lightIntensity;

    // fianalColor
    vec3 finalColor = ambient + diffuse + specular;

    color = vec4(finalColor, 1.0);
}