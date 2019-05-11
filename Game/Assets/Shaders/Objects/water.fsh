#version 330 core
layout(location = 0) out vec4 gPosition;
layout(location = 1) out vec4 gNormal;
layout(location = 2) out vec4 gAlbedoSpec;
layout(location = 3) out vec4 gInfo;
in VS_OUT
{
	vec3 gPosition;
	vec3 gNormal;
	vec4 gColor;
	vec2 gTexCoord;
} fs_in;

uniform mat4 view_matrix;

uniform sampler2D diffuse;

struct Wave 
{
float speed;
float amplitude;
float frequency;
};

uniform Wave wave;

void main()
{
	gPosition.rgb = fs_in.gPosition;
	gNormal.rgb = normalize(fs_in.gNormal);
	gAlbedoSpec.rgb = vec3(1.0,0.0,0.0).rgb;
	gPosition.a = 1;
	gNormal.a = 1;
	gAlbedoSpec.a = 1;
}