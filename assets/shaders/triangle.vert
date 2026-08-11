// 顶点着色器（Vertex Shader）

#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;

uniform mat4 mvp;
uniform mat4 mv;
uniform mat4 lightMvp;

out vec3 fragPos;
out vec3 fragNormal;
out vec2 fragTexCoord;
out vec4 fragLightSpacePos;

void main()
{
	gl_Position = mvp * vec4(pos, 1.0);

	// 顶点位置和法线传递给片段着色器
	fragPos = vec3(mv *  vec4(pos, 1.0));
	mat3 normalMatrix = transpose(inverse(mat3(mv))); // 相机变换矩阵的逆转置矩阵处理法线
	fragNormal = normalize(normalMatrix * normal);
	fragTexCoord = texCoord;

	// 同一局部空间顶点额外投影到聚光灯裁剪空间，供片元阶段查询阴影贴图。
	fragLightSpacePos = lightMvp * vec4(pos, 1.0);
}
