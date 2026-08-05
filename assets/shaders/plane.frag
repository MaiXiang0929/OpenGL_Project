#version 330 core

in vec2 fragTexCoord;

layout(location = 0) out vec4 color;

uniform sampler2D renderedTexture;

void main()
{
    vec3 renderedColor = texture(renderedTexture, fragTexCoord).rgb;

    // 作业要求给平面颜色增加小常量，使纹理中的黑色区域仍能与黑色背景区分。
    color = vec4(min(renderedColor + vec3(0.03), vec3(1.0)), 1.0);
}
