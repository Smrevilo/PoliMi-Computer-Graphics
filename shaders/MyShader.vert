#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
	mat4 mvpMat;
	mat4 mMat;
	mat4 nMat;
	vec4 color;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNorm;

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 fragNorm;
layout(location = 2) out vec2 fragTexCoord;

void main() {
//	fragPos = vec3(inPosition.x, inPosition.z, 0.5f);
//	gl_Position = vec4(inPosition.x, inPosition.z, 0.5f, 1.0f); // 0 - Starting point: just a grid that fills the entire screen

//	gl_Position = vec4(inPosition.x, inPosition.z, 0.5f, 3.0f); // 1 - See what happens if i change w

//	gl_Position = vec4(cos(inPosition.x*3.14)* (1+inPosition.z)/2,  // 2 - Make a circle
//	                   sin(inPosition.x*3.14)* (1+inPosition.z)/2,
//	                   0.5f, 1.0f);

//	fragPos = vec3(inPosition.x, inPosition.z, 0.5f); // 3 - Use World View Projection
//	gl_Position = ubo.mvpMat * vec4(fragPos, 1.0f);

//	fragPos = vec3(inPosition.x, inPosition.y + sin(5*inPosition.x)*0.2, inPosition.z); // 4 - Wobbling effect
//	gl_Position = ubo.mvpMat * vec4(fragPos, 1.0f);

	fragPos = inPosition; 
	fragNorm = vec3(0.0,0.0,1.0);
	fragTexCoord = vec2(0.5,0.5);
	gl_Position = ubo.mvpMat * vec4(fragPos, 1.0f);

}