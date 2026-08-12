#version 400 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in vec4 tangent;

uniform mat4 mvp;
uniform mat4 mv;
uniform mat4 lightMvp;

out vec3 fragPos;
out vec3 fragNormal;
out vec3 fragTangent;
out float fragTangentSign;
out vec2 fragTexCoord;
out vec4 fragLightSpacePos;

void main()
{
    gl_Position = mvp * vec4(pos, 1.0);
    fragPos = vec3(mv * vec4(pos, 1.0));

    mat3 normalMatrix = transpose(inverse(mat3(mv)));
    fragNormal = normalize(normalMatrix * normal);
    fragTangent = normalize(mat3(mv) * tangent.xyz);
    fragTangentSign = tangent.w;
    fragTexCoord = texCoord;
    fragLightSpacePos = lightMvp * vec4(pos, 1.0);
}
