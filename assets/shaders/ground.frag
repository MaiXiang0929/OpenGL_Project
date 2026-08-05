// 反射地面片段着色器
// 使用 render-to-texture 的反射纹理 + cubemap 实现地面反射

#version 330 core

in vec3 worldPos;

uniform samplerCube cubemap;
uniform sampler2D reflectionTex;
uniform mat4 reflectionVP;
uniform vec3 cameraWorldPos;

layout(location = 0) out vec4 color;

void main()
{
    // 将世界空间位置投影到反射纹理坐标
    vec4 clipPos = reflectionVP * vec4(worldPos, 1.0);
    vec3 ndc = clipPos.xyz / clipPos.w;
    vec2 uv = ndc.xy * 0.5 + 0.5;

    vec3 result;

    if (ndc.z > 0.0 && ndc.z < 1.0 &&
        uv.x >= 0.0 && uv.x <= 1.0 &&
        uv.y >= 0.0 && uv.y <= 1.0)
    {
        // 反射纹理中已有天空盒 + 物体的反射画面
        result = texture(reflectionTex, uv).rgb;
    }
    else
    {
        // 超出反射纹理范围时，回退到 cubemap 采样
        vec3 N = vec3(0.0, 1.0, 0.0);
        vec3 V = normalize(cameraWorldPos - worldPos);
        vec3 R = reflect(-V, N);
        result = texture(cubemap, R).rgb;
    }

    color = vec4(result, 1.0);
}
