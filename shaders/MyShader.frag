#version 450#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNorm;layout(location = 2) in vec2 fragTexCoord;
layout(location = 0) out vec4 outColor;
layout(binding = 0) uniform UniformBufferObject {	mat4 mvpMat;	mat4 mMat;	mat4 nMat;	vec4 color;} ubo;layout(binding = 1) uniform sampler2D texSampler;
layout(binding = 2) uniform GlobalUniformBufferObject {
	vec3 lightDir;
	vec4 lightColor;
	vec3 eyePos;
} gubo;

void main() {
//	outColor = vec4(1.0f, 0.0f, 0.0f, 1.0f); // 0 - all red//	outColor = vec4(0.0f, 0.5f, 1.0f, 1.0f); // 1 - all azure//	outColor = vec4(fragPos, 1.0f); // 2 - first shade//	outColor = vec4((fragPos + vec3(1,1,1)) / 2.0f, 1.0f); // 3 - extend the gamma//	outColor = vec4((fragPos + vec3(10,10,10)) / 20.0f, 1.0f); // 4 - extend the gamma even more	outColor = vec4((fragPos + vec3(1+sin(20.0f * fragPos.x)*0.1,1+cos(33.0f * fragPos.y)*0.08,1)) / 2.0f, 1.0f); // 3 - extend the gamma}