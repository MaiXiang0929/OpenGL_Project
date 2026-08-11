#version 400 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;
out VS_OUT { vec3 position; vec3 normal; vec2 uv; } vsOut;
void main() { vsOut.position = position; vsOut.normal = normal; vsOut.uv = texCoord; gl_Position = vec4(position, 1.0); }
