#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec4 color;
layout(location = 3) in vec2 texCoord;
layout(location = 4) in vec3 tangents;
layout(location = 5) in vec3 bitangents;
layout(location = 6) in vec4 weights;
layout(location = 7) in ivec4 ids;

out VS_OUT
{
	vec3 gPosition;
	vec3 gNormal;
	vec4 gColor;
	vec2 gTexCoord;
	vec4 pos;
	float time;
} vs_out;

uniform mat4 model_matrix;
uniform mat4 mvp_matrix;
uniform mat3 normal_matrix;

uniform int repeat;

struct Wave 
{
float speed;
float amplitude;
float frequency;
};

uniform float Time;
uniform Wave wave;

void main()
{
	vec4 pos = vec4(position, 1.0);
	vec4 norm = vec4(normal, 0.0);
	
	
	vs_out.gTexCoord = texCoord * repeat;
	
	// Wave
    pos.y += sin((Time * wave.speed) + (pos.x * pos.z * wave.frequency)) * wave.amplitude;

	vs_out.gPosition = vec3(model_matrix * pos);
	vs_out.gNormal = normalize(normal_matrix * norm.xyz);
	vs_out.gColor = color;
    vs_out.pos = pos;
    vs_out.time = Time;
	gl_Position = mvp_matrix * pos;
}