#version 400 core
layout(triangles, equal_spacing, ccw) in;
in TC_OUT { vec3 position; vec3 normal; vec2 uv; } teIn[];
out TE_OUT { vec3 position; vec3 normal; vec2 uv; } teOut;
uniform mat4 mvp;
uniform sampler2D displacementMap;
uniform int hasDisplacement;
uniform float displacementScale;
void main() {
    vec3 p = gl_TessCoord.x * teIn[0].position + gl_TessCoord.y * teIn[1].position + gl_TessCoord.z * teIn[2].position;
    vec3 n = normalize(gl_TessCoord.x * teIn[0].normal + gl_TessCoord.y * teIn[1].normal + gl_TessCoord.z * teIn[2].normal);
    vec2 uv = gl_TessCoord.x * teIn[0].uv + gl_TessCoord.y * teIn[1].uv + gl_TessCoord.z * teIn[2].uv;
    if (hasDisplacement != 0) p += n * (texture(displacementMap, uv).r * displacementScale);
    teOut.position = p; teOut.normal = n; teOut.uv = uv; gl_Position = mvp * vec4(p, 1.0);
}
