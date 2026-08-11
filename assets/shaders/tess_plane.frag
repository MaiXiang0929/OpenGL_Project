#version 400 core
in TE_OUT { vec3 position; vec3 normal; vec2 uv; } fsIn;
layout(location = 0) out vec4 color;
uniform sampler2D normalMap;
uniform vec3 lightPosition;
uniform vec3 cameraPosition;
uniform mat4 lightMvp;
uniform sampler2DShadow shadowMap;
void main() {
    vec3 nTex = texture(normalMap, fsIn.uv).xyz * 2.0 - 1.0;
    vec3 N = normalize(vec3(nTex.xy, max(nTex.z, 0.15)));
    vec3 L = normalize(lightPosition - fsIn.position);
    vec3 V = normalize(cameraPosition - fsIn.position);
    vec3 H = normalize(L + V);
    float diffuse = max(dot(N, L), 0.0);
    float specular = pow(max(dot(N, H), 0.0), 32.0);
    vec4 lightClip = lightMvp * vec4(fsIn.position, 1.0);
    vec3 shadowCoord = lightClip.xyz / lightClip.w * 0.5 + 0.5;
    float visibility = shadowCoord.z <= 1.0
        ? texture(shadowMap, vec3(shadowCoord.xy, shadowCoord.z - 0.002)) : 1.0;
    color = vec4(vec3(0.12) + visibility * (diffuse * vec3(0.65, 0.72, 0.9) + specular * vec3(1.0)), 1.0);
}
