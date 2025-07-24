#version 450

layout(binding = 2, std140) uniform GlobalUniformBufferObject
{
    vec3 lightDir;
    vec4 lightColor;
    vec3 eyePos;
    vec4 eyeDir;
} gubo;

layout(binding = 0, std140) uniform UniformBufferObject
{
    mat4 mvpMat;
    mat4 mMat;
    mat4 nMat;
} ubo;

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 1) in vec3 fragNorm;
layout(location = 0) in vec3 fragPos;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 0) out vec4 outColor;

void main()
{
    vec3 Norm = normalize(fragNorm);
    vec3 EyeDir = normalize(gubo.eyePos - fragPos);
    vec3 Albedo = texture(texSampler, fragTexCoord).xyz;
    vec3 Diffuse = Albedo * ((max(dot(Norm, gubo.lightDir), 0.0) * 0.89999997615814208984375) + 0.100000001490116119384765625);
    vec3 Specular = vec3(pow(max(dot(EyeDir, -reflect(gubo.lightDir, Norm)), 0.0), 64.0));
    vec3 Ambient = (((mix(vec3(0.180000007152557373046875, 0.119999997317790985107421875, 0.07999999821186065673828125), vec3(0.20000000298023223876953125, 0.100000001490116119384765625, 0.100000001490116119384765625), bvec3(Norm.x > 0.0)) * (Norm.x * Norm.x)) + (mix(vec3(0.100000001490116119384765625), vec3(0.0599999986588954925537109375, 0.20000000298023223876953125, 0.20000000298023223876953125), bvec3(Norm.y > 0.0)) * (Norm.y * Norm.y))) + (mix(vec3(0.0599999986588954925537109375, 0.119999997317790985107421875, 0.14000000059604644775390625), vec3(0.1599999964237213134765625, 0.039999999105930328369140625, 0.07999999821186065673828125), bvec3(Norm.z > 0.0)) * (Norm.z * Norm.z))) * Albedo;
    outColor = vec4(((Diffuse + (Specular * (1.0 - gubo.eyeDir.w))) * gubo.lightColor.xyz) + Ambient, 1.0);
}
