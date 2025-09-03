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
	vec3 X = dFdx(fragPos);
	vec3 Y = dFdy(fragPos);
	vec3 Norm = normalize(cross(X,Y));

	vec3 EyeDir = normalize(gubo.eyePos - fragPos);
	//Norm = dot(Norm, gubo.lightDir) > 0 ? Norm : -Norm;
		vec3 Albedo = ubo.color.rgb * ubo.color.a + texture(texSampler, fragTexCoord).rgb * (1 - ubo.color.a);
	vec3 Diffuse = Albedo * max(dot(Norm, -gubo.lightDir),0.0f);
	vec3 Specular = vec3(pow(max(dot(EyeDir, -reflect(gubo.lightDir, Norm)),0.0f), 160.0f));
	// A special type of non-uniform ambient color, invented for this course	const vec3 cxp = vec3(1.0,0.5,0.5) * 0.2;	const vec3 cxn = vec3(0.9,0.6,0.4) * 0.2;	const vec3 cyp = vec3(0.3,1.0,1.0) * 0.2;	const vec3 cyn = vec3(0.5,0.5,0.5) * 0.2;	const vec3 czn = vec3(0.8,0.2,0.4) * 0.2;	const vec3 czp = vec3(0.3,0.6,0.7) * 0.2;		vec3 Ambient =((Norm.x > 0 ? cxp : cxn) * (Norm.x * Norm.x) +				   (Norm.y > 0 ? cyp : cyn) * (Norm.y * Norm.y) +				   (Norm.z > 0 ? czp : czn) * (Norm.z * Norm.z))* Albedo;	
	outColor = vec4((Diffuse + Specular) * gubo.lightColor.rgb + Ambient, 1.0f);//	outColor = vec4(Ambient, 1.0f);

}