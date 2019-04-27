#version 330 core
layout(location = 0) out vec4 gPosition;
layout(location = 1) out vec4 gNormal;
layout(location = 2) out vec4 gAlbedoSpec;
layout(location = 3) out uvec4 gInfo;
in VS_OUT
{
	vec3 gPosition;
	vec3 gNormal;
	vec4 gColor;
	vec2 gTexCoord;
} fs_in;

uniform sampler2D diffuse;

uniform uint layer;
uniform float alpha;

void main()
{
	gPosition.rgb = fs_in.gPosition;
	gNormal.rgb = normalize(fs_in.gNormal);
	gAlbedoSpec.rgb = texture(diffuse, fs_in.gTexCoord).rgb;
	gPosition.a = 1;
	gNormal.a = 1;
	gAlbedoSpec.a = alpha;
	gInfo.r = layer;
	gInfo.g = 0u;
	gInfo.b = 0u;
	gInfo.a = 0u;
}