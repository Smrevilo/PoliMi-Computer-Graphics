#version 450

layout(binding = 0, std140) uniform UniformBufferObject
{
    mat4 mvpMat;
    mat4 mMat;
    mat4 nMat;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 fragNorm;
layout(location = 2) in vec3 inNormal;
layout(location = 2) out vec2 fragTexCoord;
layout(location = 1) in vec2 inTexCoord;

void main()
{
    gl_Position = ubo.mvpMat * vec4(inPosition, 1.0);
    fragPos = (ubo.mMat * vec4(inPosition, 1.0)).xyz;
    fragNorm = mat3(ubo.nMat[0].xyz, ubo.nMat[1].xyz, ubo.nMat[2].xyz) * inNormal;
    fragTexCoord = inTexCoord;
}
