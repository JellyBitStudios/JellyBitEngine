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
uniform mat4 model_matrix;
uniform mat4 projectorMatrix;
uniform vec2 screenSize;

void main()
{
    // fragment's screen pos
    vec2 screenPos = gl_FragCoord.xy;
    screenPos.x /= screenSize.x;
    screenPos.y /= screenSize.y;
    
    // gBuffer fragment's world pos
    vec4 worldPos = texture(gBufferPosition, screenPos);
    // gBuffer fragment's object pos
    vec4 objectPos = inverse(model_matrix) * worldPos;
    
    // Bounds check
    if (abs(objectPos.xyz) > 0.5)
    discard;
    
    
    
    
    //vec4 projectorTexCoord = projectorMatrix * modelPosition;
    
    vec4 color = vec4(1.0, 0.0, 0.0, 1.0);
    //color = textureProj(projectorTex, projectorTexCoord);

	gPosition.rgb = fs_in.gPosition;
	gNormal.rgb = normalize(fs_in.gNormal);
	gAlbedoSpec.rgb = color.rgb;
	gPosition.a = 1;
	gNormal.a = 1;
	gAlbedoSpec.a = 1;
}