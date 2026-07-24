#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;

uniform mat4 mvp;
uniform mat4 mv;

out vec3 fragPos;
out vec3 fragNormal;

void main()
{
	gl_Position = mvp * vec4(pos, 1.0);

	// 顶点位置和法线传递给片段着色器
	fragPos = vec3(mv *  vec4(pos, 1.0));
	mat3 normalMatrix = transpose(inverse(mat3(mv))); // 相机变换矩阵的逆转置矩阵处理法线
	fragNormal = normalize(normalMatrix * normal);
}