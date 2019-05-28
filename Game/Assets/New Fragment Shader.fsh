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

struct Fog
{
	vec3 color;
	//float minDist;
	//float maxDist;
	float density;
};

uniform Fog fog;
uniform mat4 view_matrix;

uniform int animate;

uniform sampler2D diffuse;
uniform sampler2D specular;
uniform sampler2D gInfoTexture;

uniform int dot;
uniform vec2 screenSize;

void main()
{
	gPosition.rgb = fs_in.gPosition;
	gNormal.rgb = normalize(fs_in.gNormal);
	gAlbedoSpec.rgb = mix(texture(diffuse, fs_in.gTexCoord).rgb, texture(specular, fs_in.gTexCoord).rgb, 0.1);
	gPosition.a = 1;
	gNormal.a = 1;
	gAlbedoSpec.a = 1;

	if (animate != 1)
	{
	vec4 modelViewPos = view_matrix * vec4(fs_in.gPosition, 1.0);
	float dist = length(modelViewPos.xyz);
	//float fogFactor = (fog.maxDist - dist) / (fog.maxDist - fog.minDist); // Linear
	//float fogFactor = exp(-fog.density * dist); // Exponential
	float fogFactor = exp(-pow(fog.density * dist, 2.0)); // Exponential Squared
	fogFactor = clamp(fogFactor, 0.0, 1.0);
	gAlbedoSpec = mix(vec4(fog.color, 1.0), gAlbedoSpec, fogFactor);
	}

	vec2 screenPos = gl_FragCoord.xy;
	screenPos.x /= screenSize.x;
	screenPos.y /= screenSize.y;
	gInfo.r = dot;
	if (dot != 1)
	gInfo.g = texture(gInfoTexture,screenPos).g;
	else
	gInfo.g = 1;
	gInfo.b = 0;
	gInfo.a = 0;
}