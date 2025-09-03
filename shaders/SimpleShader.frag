#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform UniformBufferObject {
    mat4 mvpMat;
    mat4 mMat;
    mat4 nMat;
} ubo;

layout(binding = 1) uniform sampler2D texSampler;

layout(binding = 2) uniform GlobalUniformBufferObject {
    vec3 lightDir;   // Direction TO light? In existing shaders it's used as -lightDir
    vec4 lightColor;
    vec3 eyePos;
    vec4 eyeDir;     // Unused but present in main.cpp
} gubo;

void main() {
    vec3 N = normalize(fragNorm);
    vec3 L = normalize(gubo.lightDir);
    vec3 V = normalize(gubo.eyePos - fragPos);
    vec3 albedo = texture(texSampler, fragTexCoord).rgb;

    // Simple lighting: ambient + lambert + small specular
    float diff = max(dot(N, L), 0.0);
    vec3 R = reflect(-L, N);
    float spec = pow(max(dot(V, R), 0.0), 32.0);

    vec3 ambient = 0.15 * albedo;
    vec3 color = ambient + (albedo * diff + vec3(0.2) * spec) * gubo.lightColor.rgb;
    outColor = vec4(color, 1.0);
}
