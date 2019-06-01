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
	vec4 pos;
	float time;
} fs_in;

uniform mat4 view_matrix;

uniform sampler2D diffuse;
uniform sampler2D wavesTexture;
uniform sampler2D foamTexture;

uniform float speedWaves;
uniform float speedFoam;

struct Wave 
{
float speed;
float amplitude;
float frequency;
};

uniform Wave wave;

void main()
{
	vec2 wavesTexCoords = fs_in.gTexCoord;
	vec2 foamTexCoords = fs_in.gTexCoord;
    wavesTexCoords += sin((fs_in.time * speedWaves) + (fs_in.pos.x * fs_in.pos.z * 0.0001)) * speedWaves;
	foamTexCoords += cos((fs_in.time * speedFoam) + (fs_in.pos.x * fs_in.pos.z * 0.0001)) * speedFoam;

	gPosition.rgb = fs_in.gPosition;
	gNormal.rgb = normalize(fs_in.gNormal);
	vec4 wavesTex = texture(wavesTexture, wavesTexCoords);
	vec4 foamTex = texture(foamTexture, fs_in.gTexCoord);
	vec4 diffuseTex = texture(diffuse, foamTexCoords);
	vec3 mixed = mix(foamTex.rgb, diffuseTex.rgb, diffuseTex.a);
	mixed = mix(mixed.rgb, wavesTex.rgb, wavesTex.a);
	gAlbedoSpec.rgb = mixed.rgb;
	gPosition.a = 1;
	gNormal.a = 1;
	gAlbedoSpec.a = 1;
}