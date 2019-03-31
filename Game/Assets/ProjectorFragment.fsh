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

uniform sampler2D projectorTex;

uniform sampler2D gBufferPosition;
uniform mat4 projectorMatrix;
uniform vec2 viewportSize;

void main()
{
    vec2 screenPos = gl_FragCoord.xy;
    screenPos.x /= viewportSize.x;
    screenPos.y /= viewportSize.y;
    
    // gBuffer fragment's screen pos (2D)
    vec4 modelPosition = texture(gBufferPosition, screenPos);
    vec4 projectorTexCoord = projectorMatrix * modelPosition;
    
    vec4 color = vec4(0.0);
    color = textureProj(projectorTex, projectorTexCoord);

	gPosition.rgb = fs_in.gPosition;
	gNormal.rgb = normalize(fs_in.gNormal);
	gAlbedoSpec.rgb = color.rgb;
	gPosition.a = 1;
	gNormal.a = 1;
	gAlbedoSpec.a = 1;
}