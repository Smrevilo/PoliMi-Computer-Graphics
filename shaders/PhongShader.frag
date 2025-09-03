#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragPos; // Input fragment position in world space
layout(location = 1) in vec3 fragNorm; // Input fragment normal in world space
layout(location = 2) in vec2 fragTexCoord; // Input texture coordinates

layout(location = 0) out vec4 outColor; // Output color of the fragment

layout(binding = 0) uniform UniformBufferObject { // Uniform buffer for object-specific data
	mat4 mvpMat; // Model-View-Projection matrix
	mat4 mMat; // Model matrix
	mat4 nMat; // Normal matrix
	float specularStrength;
} ubo;

layout(binding = 1) uniform sampler2D texSampler; // Sampler for the texture

layout(binding = 2) uniform GlobalUniformBufferObject { // Uniform buffer for global data
	vec3 lightDir; // Direction of the light source
	vec4 lightColor; // Color of the light source
	vec3 eyePos; // Position of the camera/eye
} gubo;

void main() {
	vec3 Norm = normalize(-fragNorm);

	vec3 EyeDir = normalize(gubo.eyePos - fragPos); // Compute the direction from the fragment to the eye
	//Norm = dot(Norm, gubo.lightDir) > 0 ? Norm : -Norm; // Uncomment to flip normal if needed
	
	vec3 Albedo =  texture(texSampler, fragTexCoord).rgb;
	vec3 Diffuse = Albedo * max(dot(Norm, -gubo.lightDir),0.0f);
	vec3 Specular = vec3(pow(max(dot(EyeDir, -reflect(gubo.lightDir, Norm)),0.0f), 160.0f));

	const vec3 cxp = vec3(1.0,1.0,1.0) * 0.2;
	const vec3 cxn = vec3(1.0,1.0,1.0) * 0.2;
	const vec3 cyp = vec3(0.3,1.0,1.0) * 0.2;
	const vec3 cyn = vec3(0.5,0.5,0.5) * 0.2;
	const vec3 czn = vec3(1.0,1.0,1.0) * 0.2;
	const vec3 czp = vec3(1.0,1.0,1.0) * 0.2;
	
	vec3 Ambient =((Norm.x > 0 ? cxp : cxn) * (Norm.x * Norm.x) +
				   (Norm.y > 0 ? cyp : cyn) * (Norm.y * Norm.y) +
				   (Norm.z > 0 ? czp : czn) * (Norm.z * Norm.z))* Albedo;

	outColor = vec4((Diffuse + Specular * ubo.specularStrength) * gubo.lightColor.rgb + Ambient, 1.0f);
//	outColor = vec4(Ambient, 1.0f);

}