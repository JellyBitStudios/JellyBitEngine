#version 330 core
layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in VS_OUT
{
	vec3 gPosition;
	vec3 gNormal;
	vec4 gColor;
	vec2 gTexCoord;
} fs_in;

uniform sampler2D diffuse;

void main()
{
	gPosition.rgb = fs_in.gPosition;
	gNormal.rgb = normalize(fs_in.gNormal);
	gAlbedoSpec.rgb = texture(diffuse, fs_in.gTexCoord).rgb;
	gPosition.a = 1;
	gNormal.a = 1;
	gAlbedoSpec.a = 1;
}