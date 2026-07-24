#version 330 core
in vec2 TexCoord;
out vec4 color;

void main()
{
    float dist = distance(TexCoord, vec2(0.5, 0.5));
    if (dist > 0.5) discard; // 裁剪成一个圆
    
    color = vec4(1.0, 1.0, 0.0, 1.0); // 纯黄色
}