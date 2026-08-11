#version 400 core
layout(vertices = 3) out;
in VS_OUT { vec3 position; vec3 normal; vec2 uv; } tcIn[];
out TC_OUT { vec3 position; vec3 normal; vec2 uv; } tcOut[];
uniform float tessellationLevel;
void main() {
    tcOut[gl_InvocationID].position = tcIn[gl_InvocationID].position;
    tcOut[gl_InvocationID].normal = tcIn[gl_InvocationID].normal;
    tcOut[gl_InvocationID].uv = tcIn[gl_InvocationID].uv;
    gl_TessLevelInner[0] = tessellationLevel;
    gl_TessLevelOuter[0] = tessellationLevel;
    gl_TessLevelOuter[1] = tessellationLevel;
    gl_TessLevelOuter[2] = tessellationLevel;
}
