// 聚光灯深度 pass 顶点着色器
#version 330 core

layout (location = 0) in vec3 pos;

// OBJ 局部空间直接变换到光源裁剪空间。
uniform mat4 lightMvp;

void main()
{
    gl_Position = lightMvp * vec4(pos, 1.0);
}
