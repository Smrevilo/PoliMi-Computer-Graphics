#version 450
#extension GL_ARB_separate_shader_objects : enable

// Define a uniform buffer object (UBO) to pass data from the CPU to the GPU
layout(binding = 0) uniform UniformBufferObject {
	mat4 mvpMat; // Model-View-Projection matrix for transforming vertices to clip space
	mat4 mMat; // Model matrix for transforming vertices to world space
	mat4 nMat; // Normal matrix for transforming normals to world space 
	vec4 color; // Color information (not used in this shader)
} ubo;

// Define input attributes from the vertex buffer
layout(location = 0) in vec3 inPosition; // Vertex position in model space
layout(location = 1) in vec2 inUV; // Texture coordinates
layout(location = 2) in vec3 inNorm; // Vertex normal in model space

// Define output variables to pass data to the fragment shader
layout(location = 0) out vec3 fragPos; // Vertex position in world space
layout(location = 1) out vec3 fragNorm; // Normal vector in world space
layout(location = 2) out vec2 fragTexCoord; // Texture coordinates

void main() {
	// Transform the vertex position from model space to clip space using the MVP matrix
	gl_Position = ubo.mvpMat * vec4(inPosition, 1.0); // Compute the clip-space position of the vertex

	// Transform the vertex position from model space to world space using the model matrix
	fragPos = (ubo.mMat * vec4(inPosition, 1.0)).xyz; // Compute the world-space position of the vertex

	// Transform the vertex normal from model space to world space using the normal matrix
	fragNorm = mat3(ubo.nMat) * inNorm; // Compute the world-space normal vector

	// Pass the texture coordinates directly to the fragment shader
	fragTexCoord = inUV; // Pass the texture coordinates unchanged
}