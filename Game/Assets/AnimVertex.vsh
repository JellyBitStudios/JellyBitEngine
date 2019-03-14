#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec4 color;
layout(location = 3) in vec2 texCoord;
layout(location = 6) in vec4 weights;
layout(location = 7) in ivec4 ids;

uniform mat4 model_matrix;
uniform mat4 mvp_matrix;
uniform mat3 normal_matrix;

const int MAX_BONES = 100;
uniform mat4 bones[MAX_BONES];

out VS_OUT
{
  vec3 gPosition;
  vec3 gNormal;
  vec4 gColor;
  vec2 gTexCoord;
} vs_out;

void main()
{
	mat4 boneTransform = bones[ids[0]] * weights[0];
	boneTransform += bones[ids[1]] * weights[1];
	boneTransform += bones[ids[2]] * weights[2];
	boneTransform += bones[ids[3]] * weights[3];

	vec4 pos = boneTransform * vec4(position, 1.0);
	pos.x *= 100;
	pos.y *= 100;
	pos.z *= 100;
	vec4 norm = boneTransform * vec4(normal, 0.0);

	vs_out.gPosition = vec3(model_matrix * pos);
	vs_out.gNormal = normalize(normal_matrix * vec3(norm));
	vs_out.gColor = color;
	vs_out.gTexCoord = texCoord;

	gl_Position = mvp_matrix * pos;
}