#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 texCoord;

uniform mat4 proj;
uniform mat4 view;
uniform vec3 lightWorldPos; // 光源在世界空间中的中心点位置
uniform float scale;        // 图标的缩放大小

out vec2 TexCoord;

void main()
{
    // 世界坐标转换到 View Space，并在 X、Y 平面上直接偏移，实现永远朝向相机
    vec4 centerPosView = view * vec4(lightWorldPos, 1.0);
    vec4 vertexPosView = centerPosView + vec4(pos.x * scale, pos.y * scale, 0.0, 0.0);

    gl_Position = proj * vertexPosView;
    TexCoord = texCoord;
}