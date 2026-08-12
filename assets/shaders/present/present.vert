#version 330 core

// 方形平面只需要位置与纹理坐标；法线属性由通用 Mesh 布局保留但不参与计算。
layout (location = 0) in vec3 pos;
layout (location = 2) in vec2 texCoord;

uniform mat4 mvp;

out vec2 fragTexCoord;

void main()
{
    // 平面使用独立相机，因此可以在按住 Alt 时单独旋转和缩放。
    gl_Position = mvp * vec4(pos, 1.0);
    fragTexCoord = texCoord;
}
