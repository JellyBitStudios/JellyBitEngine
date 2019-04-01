#version 330 core
layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;
layout (location = 3) out uvec4 gInfo;

in VS_OUT
{
	vec3 gPosition;
	vec3 gNormal;
	vec4 gColor;
	vec2 gTexCoord;
} fs_in;

uniform sampler2D gBufferPosition;
uniform sampler2D gBufferNormal;
uniform usampler2D gBufferInfo;

uniform sampler2D projectorTex;

uniform mat4 model_matrix;
uniform mat4 projectorMatrix;
uniform vec2 screenSize;
uniform uint filterMask;

void main()
{
    // fragment's screen pos
    vec2 screenPos = gl_FragCoord.xy;
    screenPos.x /= screenSize.x;
    screenPos.y /= screenSize.y;
    
    // gBuffer fragment's info
    uvec4 info = texture(gBufferInfo, screenPos);
    //uint layer = 1u << info.r;
    //if ((filterMask & info.r) == 1u)
    //discard;
    
    // gBuffer fragment's world pos
    vec4 worldPos = texture(gBufferPosition, screenPos);
    if (worldPos.z == 0.0)
    discard;
    // gBuffer fragment's object pos
    vec4 objectPos = inverse(model_matrix) * worldPos;
    
    // Bounds check
    float objX = 0.5 - abs(objectPos.x);
    float objY = 0.5 - abs(objectPos.y);
    float objZ = 0.5 - abs(objectPos.z);
    if (objX < 0.0 || objY < 0.0 || objZ < 0.0)
    discard;
    
    // Calculate tex coord
    //vec2 texCoord = objectPos.xy + 0.5;
    //vec4 color = texture(projectorTex, texCoord);
    vec4 texCoord = projectorMatrix * worldPos;
    vec4 color = textureProj(projectorTex, texCoord);
    if (color.a < 0.1)
    discard;
    
    //////////

	gPosition.rgb = worldPos.rgb;
	gNormal.rgb = texture(gBufferNormal, screenPos).xyz;
	gAlbedoSpec = color;
	//gPosition.a = 1;
	//gNormal.a = 1;
	
	// ***Cartoon***
	int levels = 2;
	gPosition.a = 2;
	gNormal.a = levels;
}