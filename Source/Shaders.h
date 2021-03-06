#ifndef __SHADERS_H__
#define __SHADERS_H__

#pragma region ShaderDefault

#define vShaderTemplate																		\
"#version 330 core\n"																		\
"layout(location = 0) in vec3 position;\n"													\
"layout(location = 1) in vec3 normal;\n"													\
"layout(location = 2) in vec4 color;\n"														\
"layout(location = 3) in vec2 texCoord;\n"													\
"layout(location = 4) in vec3 tangents;\n"													\
"layout(location = 5) in vec3 bitangents;\n"												\
"layout(location = 6) in vec4 weights;\n"													\
"layout(location = 7) in ivec4 ids;\n"														\
"\n"																						\
"out VS_OUT\n"																				\
"{\n"																						\
"	vec3 gPosition;\n"																		\
"	vec3 gNormal;\n"																		\
"	vec4 gColor;\n"																			\
"	vec2 gTexCoord;\n"																		\
"} vs_out;\n"																				\
"\n"																						\
"const int MAX_BONES = 160;\n"																\
"uniform mat4 bones[MAX_BONES];\n"															\
"uniform int animate;\n"																	\
"\n"																						\
"uniform mat4 model_matrix;\n"																\
"uniform mat4 mvp_matrix;\n"																\
"uniform mat3 normal_matrix;\n"																\
"\n"																						\
"void main()\n"																				\
"{\n"																						\
"	vec4 pos = vec4(position, 1.0);\n"														\
"	vec4 norm = vec4(normal, 0.0);\n"														\
"\n"																						\
"	if (animate == 1)\n"																	\
"	{\n"																					\
"		mat4 boneTransform = bones[ids[0]] * weights[0];\n"									\
"		boneTransform += bones[ids[1]] * weights[1];\n"										\
"		boneTransform += bones[ids[2]] * weights[2];\n"										\
"		boneTransform += bones[ids[3]] * weights[3];\n"										\
"\n"																						\
"		pos = boneTransform * pos;\n"														\
"		norm = boneTransform * norm;\n"														\
"	}\n"																					\
"\n"																						\
"	vs_out.gPosition = vec3(model_matrix * pos);\n"											\
"	vs_out.gNormal = normalize(normal_matrix * norm.xyz);\n"								\
"	vs_out.gColor = color;\n"																\
"	vs_out.gTexCoord = texCoord;\n"															\
"\n"																						\
"	gl_Position = mvp_matrix * pos;\n"														\
"}"

#define fShaderTemplate	\
"#version 330 core\n" \
"layout(location = 0) out vec4 gPosition;\n" \
"layout(location = 1) out vec4 gNormal;\n" \
"layout(location = 2) out vec4 gAlbedoSpec;\n" \
"layout(location = 3) out vec4 gInfo;\n" \
"in VS_OUT\n" \
"{\n" \
"	vec3 gPosition;\n" \
"	vec3 gNormal;\n" \
"	vec4 gColor;\n" \
"	vec2 gTexCoord;\n" \
"} fs_in;\n" \
"\n" \
"struct Fog\n" \
"{\n" \
"	vec3 color;\n" \
"	//float minDist;\n" \
"	//float maxDist;\n" \
"	float density;\n" \
"};\n" \
"\n" \
"uniform Fog fog;\n" \
"uniform mat4 view_matrix;\n" \
"\n" \
"uniform int animate;\n" \
"\n" \
"uniform sampler2D diffuse;\n" \
"uniform sampler2D gInfoTexture;\n"	\
"\n" \
"uniform int dot;\n" \
"uniform vec2 screenSize;\n" \
"\n" \
"void main()\n" \
"{\n" \
"	gPosition.rgb = fs_in.gPosition;\n" \
"	gNormal.rgb = normalize(fs_in.gNormal);\n" \
"	gAlbedoSpec.rgb = texture(diffuse, fs_in.gTexCoord).rgb;\n" \
"	gPosition.a = 1;\n" \
"	gNormal.a = 1;\n" \
"	gAlbedoSpec.a = 1;\n" \
"\n" \
"	if (animate != 1)\n" \
"	{\n" \
"	vec4 modelViewPos = view_matrix * vec4(fs_in.gPosition, 1.0);\n" \
"	float dist = length(modelViewPos.xyz);\n" \
"	//float fogFactor = (fog.maxDist - dist) / (fog.maxDist - fog.minDist); // Linear\n" \
"	//float fogFactor = exp(-fog.density * dist); // Exponential\n" \
"	float fogFactor = exp(-pow(fog.density * dist, 2.0)); // Exponential Squared\n" \
"	fogFactor = clamp(fogFactor, 0.0, 1.0);\n" \
"	gAlbedoSpec = mix(vec4(fog.color, 1.0), gAlbedoSpec, fogFactor);\n" \
"	}\n" \
"\n" \
"	vec2 screenPos = gl_FragCoord.xy;\n" \
"	screenPos.x /= screenSize.x;\n" \
"	screenPos.y /= screenSize.y;\n" \
"	gInfo.r = dot;\n" \
"	if (dot != 1)\n" \
"	gInfo.g = texture(gInfoTexture,screenPos).g;\n" \
"	else\n" \
"	gInfo.g = 1;\n"	\
"	gInfo.b = 0;\n"	\
"	gInfo.a = 0;\n"	\
"}"
#pragma endregion

#pragma region ShaderDeferred

#define vDEFERREDSHADING						\
"#version 330 core\n"							\
"layout(location = 0) in vec3 position;\n"		\
"layout(location = 1) in vec3 normal;\n"		\
"layout(location = 2) in vec4 color;\n"			\
"layout(location = 3) in vec2 texCoord;\n"		\
"out vec2 TexCoords;\n"							\
"void main()\n"									\
"{\n"											\
"	TexCoords = texCoord;\n"					\
"	gl_Position = vec4(position, 1.0);\n"		\
"}"

/*
0: Blinn-Phong light
1: cartoon light
2: no light
3: outline (no light)
*/

#define fDEFERREDSHADING \
"#version 330 core\n" \
"out vec4 FragColor;\n" \
"in vec2 TexCoords;\n" \
"\n" \
"uniform sampler2D gPosition;\n" \
"uniform sampler2D gNormal;\n" \
"uniform sampler2D gAlbedoSpec;\n" \
"uniform sampler2D gInfo;\n" \
"\n" \
"struct Light\n" \
"{\n" \
"	int type;\n" \
"	vec3 dir;\n" \
"	vec3 position;\n" \
"	vec3 color;\n" \
"	float linear;\n" \
"	float quadratic;\n" \
"};\n" \
"\n" \
"const int NR_LIGHTS = 50;\n" \
"\n" \
"uniform float ambient;\n" \
"uniform Light lights[NR_LIGHTS];\n" \
"uniform vec3 colorDot;\n" \
"\n" \
"void main()\n" \
"{\n" \
"	// retrieve data from gbuffer\n" \
"	vec4 FragPosTexture = texture(gPosition, TexCoords);\n" \
"	vec3 FragPos = FragPosTexture.rgb;\n" \
"	float FragPosA = FragPosTexture.a; // type of shader\n" \
"	vec4 NormalTexture = texture(gNormal, TexCoords);\n" \
"	vec3 Normal = NormalTexture.rgb;\n" \
"	float NormalA = NormalTexture.a; // levels \n" \
"	vec4 AlbedoTexture = texture(gAlbedoSpec, TexCoords);\n" \
"	vec3 Albedo = AlbedoTexture.rgb;\n" \
"	float AlbedoA = AlbedoTexture.a;\n" \
"	vec4 InfoTexture = texture(gInfo, TexCoords);\n" \
"	if (InfoTexture.r != 1 && InfoTexture.g != 0) {\n" \
"	FragColor = vec4(colorDot, 1.0);\n" \
"	return;\n" \
"	}\n" \
"\n" \
"	vec3 lighting = Albedo;\n" \
"\n" \
"	if (FragPosA == 0 || FragPosA == 1) // light\n" \
"	{\n" \
"		lighting *= ambient; // hard-coded ambient component\n" \
"\n" \
"	for (int i = 0; i < NR_LIGHTS; ++i)\n" \
"	{\n" \
"		vec3 diffuse = vec3(0.0, 0.0, 0.0);\n" \
"		if (lights[i].type == 1)\n" \
"		{\n" \
"			if (FragPosA == 1) // cartoon\n" \
"			{\n" \
"				float cosine = max(0.0, dot(Normal, lights[i].dir));\n" \
"				float scaleFactor = 1.0 / NormalA;\n" \
"				diffuse = Albedo * lights[i].color * floor(cosine * NormalA) * scaleFactor;\n" \
"			}\n" \
"			else\n" \
"				diffuse = max(dot(Normal, lights[i].dir), 0.0) * Albedo * lights[i].color;\n" \
"		}\n" \
"		else if (lights[i].type == 2)\n" \
"		{\n" \
"			vec3 lightDir = normalize(lights[i].position - FragPos);\n" \
"			if (FragPosA == 1) // cartoon\n" \
"			{\n" \
"				float cosine = max(0.0, dot(Normal, lightDir));\n" \
"				float scaleFactor = 1.0 / NormalA;\n" \
"				diffuse = Albedo * lights[i].color * floor(cosine * NormalA) * scaleFactor;\n" \
"			}\n" \
"			else\n" \
"				diffuse = max(dot(Normal, lightDir), 0.0) * Albedo * lights[i].color;\n" \
"			float distance = length(lights[i].position - FragPos);\n" \
"			float attenuation = 1.0 / (1.0 + lights[i].linear * distance + lights[i].quadratic * distance * distance);\n" \
"			diffuse *= attenuation;\n" \
"		}\n" \
"		lighting += diffuse;\n" \
"	}\n" \
"	}\n" \
"\n" \
"	FragColor = vec4(lighting, AlbedoA);\n" \
"}"
#pragma endregion

#pragma region ShaderBillboard

#define vShaderBillboard										\
"#version 330 core\n"											\
"layout(location = 0) in vec3 position;\n"						\
"layout(location = 1) in vec3 normal;\n"						\
"layout(location = 2) in vec4 color;\n"							\
"layout(location = 3) in vec2 texCoord;\n"						\
"uniform mat4 mvp_matrix;\n"									\
"out vec2 fTexCoord;\n"											\
"void main()\n"													\
"{\n"															\
"	fTexCoord = texCoord;\n"									\
"	gl_Position = mvp_matrix * vec4(position, 1.0);\n"			\
"}"

#define fShaderBillboard							\
"#version 330 core\n"								\
"out vec4 FragColor;\n"								\
"in vec2 fTexCoord;\n"								\
"uniform sampler2D diffuse;\n"						\
"void main()\n"										\
"{\n"												\
"	vec4 color = texture(diffuse, fTexCoord);\n"	\
"	if (color.a < 0.1)\n"							\
"		discard;\n"									\
"	FragColor = color;\n"							\
"}"
#pragma endregion

#pragma region ShaderParticles

#define Particle_vShaderTemplate \
"#version 330 core\n" \
"\n" \
"layout(location = 0) in vec3 position;\n" \
"layout(location = 1) in vec3 normal;\n" \
"layout(location = 2) in vec4 color;\n" \
"layout(location = 3) in vec2 texCoord;\n" \
"\n" \
"uniform mat4 model_matrix;\n" \
"uniform mat4 mvp_matrix;\n" \
"uniform mat3 normal_matrix;\n" \
"uniform vec4 currColor;\n" \
"uniform float rowUVNorm;\n" \
"uniform float columUVNorm;\n" \
"uniform vec2 currMinCoord;\n" \
"uniform int isAnimated;\n" \
"\n" \
"out vec3 fPosition;\n" \
"out vec3 fNormal;\n" \
"out vec4 fColor;\n" \
"out vec2 fTexCoord;\n" \
"\n" \
"void main()\n" \
"{\n" \
"	fPosition = vec3(model_matrix * vec4(position, 1.0));\n" \
"	fNormal = normalize(normal_matrix * normal);\n" \
"	fColor = currColor;\n" \
"	if(isAnimated == 0)\n" \
"		fTexCoord = texCoord;\n" \
"	else\n" \
"	{\n" \
"		fTexCoord = currMinCoord;\n" \
"		if(texCoord.x == 1)\n" \
"			fTexCoord.x += columUVNorm;\n" \
"		if(texCoord.y == 1)\n" \
"			fTexCoord.y += rowUVNorm;\n" \
"	}\n" \
"\n" \
"	gl_Position = mvp_matrix * vec4(position, 1.0);\n" \
"}"

#define Particle_fShaderTemplate \
"#version 330 core\n" \
"\n" \
"in vec3 fPosition;\n" \
"in vec3 fNormal;\n" \
"in vec4 fColor;\n" \
"in vec2 fTexCoord;\n" \
"\n" \
"out vec4 FragColor;\n" \
"\n" \
"struct Material\n" \
"{\n" \
"	sampler2D albedo;\n" \
"	sampler2D specular;\n" \
"	float shininess;\n" \
"};\n" \
"\n" \
"struct Light\n" \
"{\n" \
"	vec3 direction;\n" \
"\n" \
"	vec3 ambient;\n" \
"	vec3 diffuse;\n" \
"	vec3 specular;\n" \
"};\n" \
"\n" \
"uniform Material material;\n" \
"uniform float averageColor;\n" \
"\n" \
"\n" \
"void main()\n" \
"{\n" \
"	vec4 albedo = texture(material.albedo, fTexCoord);\n" \
"	if (albedo.a < 0.1)\n" \
"		discard;\n" \
"\n" \
"	FragColor = albedo * fColor;\n;" \
"}\n"
#pragma endregion

#pragma region ShaderTrails

#define vShaderTrail \
"#version 330 core\n" \
"\n" \
"layout(location = 0) in vec3 position;\n" \
"layout(location = 1) in vec3 normal;\n" \
"layout(location = 2) in vec4 color;\n" \
"layout(location = 3) in vec2 texCoord;\n" \
"\n" \
"uniform mat4 model_matrix;\n" \
"uniform mat4 mvp_matrix;\n" \
"uniform mat3 normal_matrix;\n" \
"uniform float currUV;\n" \
"uniform float nextUV;\n" \
"uniform vec4 realColor;\n" \
"uniform vec3 vertex1;\n" \
"uniform vec3 vertex2;\n" \
"uniform vec3 vertex3;\n" \
"uniform vec3 vertex4;\n" \
"\n" \
"out vec3 fPosition;\n" \
"out vec3 fNormal;\n" \
"out vec4 fColor;\n" \
"out vec2 fTexCoord;\n" \
"\n" \
"void main()\n" \
"{\n" \
"	fNormal = vec3(0, 1, 0);\n" \
"	vec2 realCoord = texCoord;\n" \
"	vec3 realPos = vec3(0);\n" \
"	if (texCoord.x == 0)\n" \
"	{\n" \
"		realCoord.x = currUV;\n" \
"		\n" \
"		if (texCoord.y == 1)\n" \
"			realPos = vertex1;\n" \
"		else\n" \
"			realPos = vertex2;\n" \
"	}\n" \
"	\n" \
"	else\n" \
"	{\n" \
"		realCoord.x = nextUV;\n" \
"		\n" \
"		if (texCoord.y == 1)\n" \
"			realPos = vertex3;\n" \
"		else\n" \
"			realPos = vertex4;\n" \
"	}\n" \
"	\n" \
"	fColor = realColor;\n" \
"	fPosition = vec3(model_matrix * vec4(realPos, 1.0));\n" \
"	fTexCoord = realCoord;\n" \
"	gl_Position = mvp_matrix * vec4(realPos, 1.0);\n" \
"}\n"

#define fShaderTrail \
"#version 330 core\n" \
"\n" \
"in vec3 fPosition;\n" \
"in vec3 fNormal;\n" \
"in vec4 fColor;\n" \
"in vec2 fTexCoord;\n" \
"\n" \
"out vec4 FragColor;\n" \
"\n" \
"struct Material\n" \
"{\n" \
"	sampler2D albedo;\n" \
"	sampler2D specular;\n" \
"	float shininess;\n" \
"};\n" \
"\n" \
"struct Light\n" \
"{\n" \
"	vec3 direction;\n" \
"\n" \
"	vec3 ambient;\n" \
"	vec3 diffuse;\n" \
"	vec3 specular;\n" \
"};\n" \
"\n" \
"uniform Material material;\n" \
"uniform float averageColor;\n" \
"\n" \
"\n" \
"void main()\n" \
"{\n" \
"	vec4 albedo = texture(material.albedo, fTexCoord);\n" \
"	if (albedo.a < 0.1)\n" \
"		discard;\n" \
"\n" \
"	FragColor = albedo * fColor;\n;" \
"}\n"
#pragma endregion

#pragma region ShaderUI
//UI
#define uivShaderDYNAMIC 																\
"#version 330 core\n" 																	\
"layout(location = 0) in vec2 vertex; // <vec2 position, vec2 texCoords>\n" 			\
"layout(location = 1) in vec2 texture_coords; // <vec2 position, vec2 texCoords>\n" 	\
"out vec2 TexCoords;"																	\
"uniform int isScreen;" 																\
"uniform int isLabel;" 																	\
"uniform int useMask;" 																	\
"uniform vec2 coordsMask;" 																\
"uniform mat4 mvp_matrix;" 																\
"\n"																					\
"uniform vec4 topLeft;\n"																\
"uniform vec4 topRight;\n"																\
"uniform vec4 bottomLeft;\n"															\
"uniform vec4 bottomRight;\n"															\
"\n"																					\
"void VertexImage()\n" 																	\
"{\n"																					\
"	vec4 position = topRight;" 															\
"	if (vertex.x > 0.0 && vertex.y > 0.0)\n"	 										\
"	{\n" 																				\
"		position = topRight;" 															\
"		if (isScreen == 0)\n" 															\
"			TexCoords = vec2(0.0, 1.0);" 												\
"		else\n"																			\
"			if (useMask == 1)\n" 														\
"				TexCoords = vec2(coordsMask.x,1.0);" 									\
"	}\n"																				\
"	else if (vertex.x > 0.0 && vertex.y < 0.0)\n" 										\
"	{\n" 																				\
"		position = bottomRight;" 														\
"		if (isScreen == 0)\n" 															\
"		{\n" 																			\
"			if (useMask == 0)\n" 														\
"				TexCoords = vec2(0.0,0.0);" 											\
"			else\n"																		\
"				TexCoords = vec2(0.0,coordsMask.y);" 									\
"		}\n"																			\
"		else\n"																			\
"			if (useMask == 1)\n" 														\
"				TexCoords = vec2(coordsMask.x,coordsMask.y);"							\
"	}\n"																				\
"	else if (vertex.x < 0.0 && vertex.y > 0.0)\n" 										\
"	{\n" 																				\
"		position = topLeft;" 															\
"		if (isScreen == 0)\n" 															\
"		{\n" 																			\
"			if (useMask == 0)\n" 														\
"				TexCoords = vec2(1.0,1.0);" 											\
"			else\n"																		\
"				TexCoords = vec2(coordsMask.x,1.0);" 									\
"		}\n"																			\
"		else\n"																			\
"			if (useMask == 1)\n" 														\
"				TexCoords = vec2(0.0,1.0);" 											\
"	}\n"																				\
"	else if (vertex.x < 0.0 && vertex.y < 0.0)\n" 										\
"	{\n" 																				\
"		position = bottomLeft;" 														\
"		if (isScreen == 0)\n" 															\
"		{\n" 																			\
"			if (useMask == 0)\n" 														\
"				TexCoords = vec2(1.0,0.0);" 											\
"			else\n"																		\
"				TexCoords = vec2(coordsMask.x,coordsMask.y);" 							\
"		}\n"																			\
"		else\n"																			\
"			if (useMask == 1)\n" 														\
"				TexCoords = vec2(0.0,coordsMask.y);" 									\
"	}\n"																				\
"	if(isScreen == 1)\n"																\
"	{\n" 																				\
"		gl_Position = position;" 														\
"		if (useMask == 0)" 																\
"			TexCoords = texture_coords;" 												\
"	}\n"																				\
"	else\n"																				\
"		gl_Position = mvp_matrix *  position;" 											\
"}\n"																					\
"\n"																					\
"void VertexLabel()\n" 																	\
"{\n"																					\
"	vec4 position = topRight;" 															\
"	if (vertex.x > 0.0 && vertex.y > 0.0)\n" 											\
"	{\n" 																				\
"		position = topRight;" 															\
"		if (isScreen == 0)\n" 															\
"			TexCoords = vec2(0.0, 0.0);" 												\
"	}\n"																				\
"	else if (vertex.x > 0.0 && vertex.y < 0.0)\n" 										\
"	{\n" 																				\
"		position = bottomRight;" 														\
"		if (isScreen == 0)\n" 															\
"			TexCoords = vec2(0.0,1.0);" 												\
"	}\n"																				\
"	else if (vertex.x < 0.0 && vertex.y > 0.0\n)" 										\
"	{\n" 																				\
"		position = topLeft;" 															\
"		if (isScreen == 0)\n" 															\
"			TexCoords = vec2(1.0,0.0);" 												\
"	}\n"																				\
"	else if (vertex.x < 0.0 && vertex.y < 0.0)\n" 										\
"	{\n" 																				\
"		position = bottomLeft;" 														\
"		if (isScreen == 0)" 															\
"			TexCoords = vec2(1.0,1.0);" 												\
"	}\n"																				\
"	if(isScreen == 1)\n"																\
"	{\n" 																				\
"		gl_Position = position;" 														\
"		TexCoords = texture_coords;"	 												\
"	}\n"																				\
"	else\n"																				\
"		gl_Position = mvp_matrix *  position;" 											\
"}\n"																					\
"\n"																					\
"void main()\n" 																		\
"{\n" 																					\
"	if(isLabel == 1)\n" 																\
"		VertexLabel();" 																\
"	else\n" 																			\
"		VertexImage();" 																\
"}\n"																						

#define uivShaderSTATIC 																\
"#version 430 core\n" 																	\
"#extension GL_ARB_shader_storage_buffer_object : require\n" 							\
"layout(location = 0) in vec2 vertex; // <vec2 position, vec2 texCoords>\n" 			\
"layout(location = 1) in vec2 texture_coords; // <vec2 position, vec2 texCoords>\n" 	\
"out vec2 TexCoords;"																	\
"uniform int isScreen;" 																\
"uniform int isLabel;" 																	\
"uniform int useMask;" 																	\
"uniform vec2 coordsMask;" 																\
"uniform float indexCorner;" 															\
"uniform mat4 mvp_matrix;" 																\
"\n"																					\
"layout (std430, binding = 1) buffer UICorners {\n"										\
"	vec4 corners[];"																	\
"	//0 vec4 topLeft;\n"																\
"	//1 vec4 topRight;\n"																\
"	//2 vec4 bottomLeft;\n"																\
"	//3 vec4 bottomRight;\n"															\
"	//indexCorner + indexarray;\n"														\
"};\n"																					\
"\n"																					\
"void VertexImage(int indexI)\n" 														\
"{\n"																					\
"	vec4 position = corners[indexI + 1];" 												\
"	if (vertex.x > 0.0 && vertex.y > 0.0)\n"	 										\
"	{\n" 																				\
"		position = corners[indexI + 1];" 												\
"		if (isScreen == 0)\n" 															\
"			TexCoords = vec2(0.0, 1.0);" 												\
"		else\n"																			\
"			if (useMask == 1)\n" 														\
"				TexCoords = vec2(coordsMask.x,1.0);" 									\
"	}\n"																				\
"	else if (vertex.x > 0.0 && vertex.y < 0.0)\n" 										\
"	{\n" 																				\
"		position = corners[indexI + 3];" 												\
"		if (isScreen == 0)\n" 															\
"		{\n" 																			\
"			if (useMask == 0)\n" 														\
"				TexCoords = vec2(0.0,0.0);" 											\
"			else\n"																		\
"				TexCoords = vec2(0.0,coordsMask.y);" 									\
"		}\n"																			\
"		else\n"																			\
"			if (useMask == 1)\n" 														\
"				TexCoords = vec2(coordsMask.x,coordsMask.y);"							\
"	}\n"																				\
"	else if (vertex.x < 0.0 && vertex.y > 0.0)\n" 										\
"	{\n" 																				\
"		position = corners[indexI];" 													\
"		if (isScreen == 0)\n" 															\
"		{\n" 																			\
"			if (useMask == 0)\n" 														\
"				TexCoords = vec2(1.0,1.0);" 											\
"			else\n"																		\
"				TexCoords = vec2(coordsMask.x,1.0);" 									\
"		}\n"																			\
"		else\n"																			\
"			if (useMask == 1)\n" 														\
"				TexCoords = vec2(0.0,1.0);" 											\
"	}\n"																				\
"	else if (vertex.x < 0.0 && vertex.y < 0.0)\n" 										\
"	{\n" 																				\
"		position = corners[indexI + 2];" 												\
"		if (isScreen == 0)\n" 															\
"		{\n" 																			\
"			if (useMask == 0)\n" 														\
"				TexCoords = vec2(1.0,0.0);" 											\
"			else\n"																		\
"				TexCoords = vec2(coordsMask.x,coordsMask.y);" 							\
"		}\n"																			\
"		else\n"																			\
"			if (useMask == 1)\n" 														\
"				TexCoords = vec2(0.0,coordsMask.y);" 									\
"	}\n"																				\
"	if(isScreen == 1)\n"																\
"	{\n" 																				\
"		gl_Position = position;" 														\
"		if (useMask == 0)" 																\
"			TexCoords = texture_coords;" 												\
"	}\n"																				\
"	else\n"																				\
"		gl_Position = mvp_matrix *  position;" 											\
"}\n"																					\
"\n"																					\
"void VertexLabel(int indexL)\n" 														\
"{\n"																					\
"	vec4 position = corners[indexL + 1];" 												\
"	if (vertex.x > 0.0 && vertex.y > 0.0)\n" 											\
"	{\n" 																				\
"		position = corners[indexL + 1];" 												\
"		if (isScreen == 0)\n" 															\
"			TexCoords = vec2(0.0, 0.0);" 												\
"	}\n"																				\
"	else if (vertex.x > 0.0 && vertex.y < 0.0)\n" 										\
"	{\n" 																				\
"		position = corners[indexL + 3];" 												\
"		if (isScreen == 0)\n" 															\
"			TexCoords = vec2(0.0,1.0);" 												\
"	}\n"																				\
"	else if (vertex.x < 0.0 && vertex.y > 0.0\n)" 										\
"	{\n" 																				\
"		position = corners[indexL];" 													\
"		if (isScreen == 0)\n" 															\
"			TexCoords = vec2(1.0,0.0);" 												\
"	}\n"																				\
"	else if (vertex.x < 0.0 && vertex.y < 0.0)\n" 										\
"	{\n" 																				\
"		position = corners[indexL + 2];" 												\
"		if (isScreen == 0)" 															\
"			TexCoords = vec2(1.0,1.0);" 												\
"	}\n"																				\
"	if(isScreen == 1)\n"																\
"	{\n" 																				\
"		gl_Position = position;" 														\
"		TexCoords = texture_coords;"	 												\
"	}\n"																				\
"	else\n"																				\
"		gl_Position = mvp_matrix *  position;" 											\
"}\n"																					\
"\n"																					\
"void main()\n" 																		\
"{\n" 																					\
"	highp int index = int(indexCorner);" 												\
"	if(isLabel == 1)\n" 																\
"		VertexLabel(index);" 															\
"	else\n" 																			\
"		VertexImage(index);" 															\
"}\n"																						

#define uifShader 																		\
"#version 330 core\n" 																	\
"in vec2 TexCoords;\n" 																	\
"out vec4 FragColor;\n" 																\
"uniform int using_texture;\n"															\
"uniform sampler2D image;\n" 															\
"uniform vec4 spriteColor;\n" 															\
"uniform int isLabel;\n" 																\
"void main()\n" 																		\
"{\n" 																					\
"	if(isLabel == 0)\n"																	\
"	{\n"																				\
"	  if(using_texture == 1)\n"															\
"		 FragColor = texture(image, TexCoords) * spriteColor;\n" 						\
"	  else\n"																			\
"	  	FragColor = spriteColor;\n" 													\
"	}\n"																				\
"	else\n"																				\
"	{\n" 																				\
"	  vec4 sampled = vec4(1.0, 1.0, 1.0, texture(image, TexCoords).r);\n" 				\
"		FragColor = spriteColor * sampled; \n" 											\
"	}\n"																				\
"}\n"
#pragma endregion

#pragma region CartoonShader

#define CartoonGeometry \
"#version 330 core\n" \
"\n" \
"layout(triangles_adjacency) in;\n" \
"layout(triangle_strip, max_vertices = 15) out;\n" \
"\n" \
"uniform mat4 model_matrix;\n" \
"\n" \
"in VS_OUT\n" \
"{\n" \
"  vec3 gPosition;\n" \
"  vec3 gNormal;\n" \
"  vec4 gColor;\n" \
"  vec2 gTexCoord;\n" \
"} gs_in[];\n" \
"\n" \
"out GS_OUT\n" \
"{\n" \
"  vec3 fPosition;\n" \
"  vec3 fNormal;\n" \
"  vec4 fColor;\n" \
"  vec2 fTexCoord;\n" \
"} gs_out;\n" \
"\n" \
"flat out int fIsEdge; // which output primitives are silhouette edges\n" \
"\n" \
"uniform float edgeWidth; // width of silhouette edge in clip\n" \
"//uniform float pctExtend; // percentage to extend quad\n" \
"\n" \
"bool isFrontFacing(vec3 a, vec3 b, vec3 c) // is a triangle front facing?\n" \
"{\n" \
"	// Compute the triangle's z coordinate of the normal vector (cross product)\n" \
"	return ((a.x * b.y - b.x * a.y) + (b.x * c.y - c.x * b.y) + (c.x * a.y - a.x * c.y)) > 0.0;\n" \
"}\n" \
"\n" \
"void emitEdgeQuad(vec3 e0, vec3 e1)\n" \
"{\n" \
"	float pctExtend = 0.0;\n" \
"\n" \
"	vec2 ext = pctExtend * (e1.xy - e0.xy);\n" \
"	vec2 v = normalize(e1.xy - e0.xy);\n" \
"	vec2 n = vec2(-v.y, v.x) * edgeWidth;\n" \
"\n" \
"	// Emit the quad\n" \
"	fIsEdge = 1; // this is part of an edge\n" \
"\n" \
"	vec4 a = vec4(e0.xy - ext, e0.z, 1.0);\n" \
"	gs_out.fPosition = vec3(model_matrix * a);\n" \
"	gl_Position = a;\n" \
"	EmitVertex();\n" \
"	vec4 b = vec4(e0.xy - n - ext, e0.z, 1.0);\n" \
"	gs_out.fPosition = vec3(model_matrix * b);\n" \
"	gl_Position = b;\n" \
"	EmitVertex();\n" \
"	vec4 c = vec4(e1.xy + ext, e1.z, 1.0);\n" \
"	gs_out.fPosition = vec3(model_matrix * c);\n" \
"	gl_Position = c;\n" \
"	EmitVertex();\n" \
"	vec4 d = vec4(e1.xy - n + ext, e1.z, 1.0);\n" \
"	gs_out.fPosition = vec3(model_matrix * d);\n" \
"	gl_Position = d;\n" \
"	EmitVertex();\n" \
"\n" \
"	EndPrimitive();\n" \
"}\n" \
"\n" \
"void main()\n" \
"{\n" \
"	vec3 p0 = gl_in[0].gl_Position.xyz / gl_in[0].gl_Position.w;\n" \
"	vec3 p1 = gl_in[1].gl_Position.xyz / gl_in[1].gl_Position.w;\n" \
"	vec3 p2 = gl_in[2].gl_Position.xyz / gl_in[2].gl_Position.w;\n" \
"	vec3 p3 = gl_in[3].gl_Position.xyz / gl_in[3].gl_Position.w;\n" \
"	vec3 p4 = gl_in[4].gl_Position.xyz / gl_in[4].gl_Position.w;\n" \
"	vec3 p5 = gl_in[5].gl_Position.xyz / gl_in[5].gl_Position.w;\n" \
"\n" \
"	if (gl_in[0].gl_Position.w > 0.0 && gl_in[1].gl_Position.w > 0.0 && gl_in[2].gl_Position.w > 0.0\n" \
"   && gl_in[3].gl_Position.w > 0.0 && gl_in[4].gl_Position.w > 0.0 && gl_in[5].gl_Position.w > 0.0 && isFrontFacing(p0, p2, p4))\n" \
"	{\n" \
"		if (!isFrontFacing(p0, p1, p2))\n" \
"			emitEdgeQuad(p0, p2);\n" \
"		if (!isFrontFacing(p2, p3, p4))\n" \
"			emitEdgeQuad(p2, p4);\n" \
"		if (!isFrontFacing(p4, p5, p0))\n" \
"			emitEdgeQuad(p4, p0);\n" \
"	}\n" \
"\n" \
"	// Output the original triangle\n" \
"	fIsEdge = 0; // this is not part of an edge\n" \
"\n" \
"	gs_out.fPosition = gs_in[0].gPosition;\n" \
"	gs_out.fNormal = gs_in[0].gNormal;\n" \
"	gs_out.fColor = gs_in[0].gColor;\n" \
"	gs_out.fTexCoord = gs_in[0].gTexCoord;\n" \
"	gl_Position = gl_in[0].gl_Position;\n" \
"	EmitVertex();\n" \
"\n" \
"	gs_out.fPosition = gs_in[2].gPosition;\n" \
"	gs_out.fNormal = gs_in[2].gNormal;\n" \
"	gs_out.fColor = gs_in[2].gColor;\n" \
"	gs_out.fTexCoord = gs_in[2].gTexCoord;\n" \
"	gl_Position = gl_in[2].gl_Position;\n" \
"	EmitVertex();\n" \
"\n" \
"	gs_out.fPosition = gs_in[4].gPosition;\n" \
"	gs_out.fNormal = gs_in[4].gNormal;\n" \
"	gs_out.fColor = gs_in[4].gColor;\n" \
"	gs_out.fTexCoord = gs_in[4].gTexCoord;\n" \
"	gl_Position = gl_in[4].gl_Position;\n" \
"	EmitVertex();\n" \
"\n" \
"	EndPrimitive();\n" \
"}"

#define CartoonFragment \
"#version 330 core\n" \
"\n" \
"layout(location = 0) out vec4 gPosition;\n" \
"layout(location = 1) out vec4 gNormal;\n" \
"layout(location = 2) out vec4 gAlbedoSpec;\n" \
"layout(location = 3) out vec4 gInfo;\n" \
"\n" \
"in GS_OUT\n" \
"{\n" \
"  vec3 fPosition;\n" \
"  vec3 fNormal;\n" \
"  vec4 fColor;\n" \
"  vec2 fTexCoord;\n" \
"} fs_in;\n" \
"\n" \
"flat in int fIsEdge; // whether or not we're drawing an edge\n" \
"\n" \
"struct Material\n" \
"{\n" \
"	sampler2D albedo;\n" \
"};\n" \
"\n" \
"struct Fog\n" \
"{\n" \
"	vec3 color;\n" \
"	//float minDist;\n" \
"	//float maxDist;\n" \
"	float density;\n" \
"};\n" \
"\n" \
"uniform Fog fog;\n" \
"uniform mat4 view_matrix;\n" \
"\n" \
"uniform int animate;\n" \
"\n" \
"uniform sampler2D gInfoTexture;\n"	\
"\n" \
"uniform Material material;\n" \
"\n" \
"uniform vec4 color;\n" \
"uniform float pct;\n" \
"uniform int lightCartoon;\n" \
"\n" \
"uniform int dot;\n" \
"uniform vec2 screenSize;\n" \
"\n" \
"//uniform vec3 lineColor; // the silhouette edge color\n" \
"//uniform int levels;\n" \
"\n" \
"void main()\n"	\
"{\n" \
"	vec3 lineColor = vec3(0.0, 0.0, 0.0);\n" \
"	int levels = 2;\n" \
"\n" \
"	// If we're drawing an edge, use constant color\n" \
"	if (fIsEdge == 1)\n" \
"	{\n" \
"		gNormal.a = 1;\n" \
"		gPosition.a = 3;\n" \
"\n" \
"		gAlbedoSpec = vec4(lineColor, 1.0);\n" \
"	}\n" \
"	// Otherwise, shade the poly\n" \
"	else\n" \
"	{\n" \
"		gNormal.a = levels;\n" \
"		gPosition.a = lightCartoon;\n" \
"\n" \
"		gAlbedoSpec = mix(texture(material.albedo, fs_in.fTexCoord), color, pct);\n" \
"\n" \
"	if (animate != 1)\n" \
"	{\n" \
"	vec4 modelViewPos = view_matrix * vec4(fs_in.fPosition, 1.0);\n" \
"	float dist = length(modelViewPos.xyz);\n" \
"	//float fogFactor = (fog.maxDist - dist) / (fog.maxDist - fog.minDist); // Linear\n" \
"	//float fogFactor = exp(-fog.density * dist); // Exponential\n" \
"	float fogFactor = exp(-pow(fog.density * dist, 2.0)); // Exponential Squared\n" \
"	fogFactor = clamp(fogFactor, 0.0, 1.0);\n" \
"	gAlbedoSpec = mix(vec4(fog.color, 1.0), gAlbedoSpec, fogFactor);\n" \
"	}\n" \
"	}\n" \
"\n" \
"	gPosition.rgb = fs_in.fPosition;\n" \
"	gNormal.rgb = normalize(fs_in.fNormal);\n" \
"	vec2 screenPos = gl_FragCoord.xy;\n" \
"	screenPos.x /= screenSize.x;\n" \
"	screenPos.y /= screenSize.y;\n" \
"	gInfo.r = dot;\n" \
"	if (dot != 1)\n" \
"	gInfo.g = texture(gInfoTexture, screenPos).g;\n"\
"	else\n"	\
"	gInfo.g = 1;\n" \
"	gInfo.b = 0;\n" \
"	gInfo.a = 0;\n" \
"}"
#pragma endregion

#pragma region DecalShader

#define DecalVertex \
"#version 330 core\n" \
"layout(location = 0) in vec3 position;\n" \
"layout(location = 1) in vec3 normal;\n" \
"layout(location = 2) in vec4 color;\n" \
"layout(location = 3) in vec2 texCoord;\n" \
"layout(location = 4) in vec3 tangents;\n" \
"layout(location = 5) in vec3 bitangents;\n" \
"layout(location = 6) in vec4 weights;\n" \
"layout(location = 7) in ivec4 ids;\n" \
"\n" \
"out VS_OUT\n" \
"{\n" \
"	vec3 gPosition;\n" \
"	vec3 gNormal;\n" \
"	vec4 gColor;\n" \
"	vec2 gTexCoord;\n" \
"} vs_out;\n" \
"\n" \
"uniform mat4 model_matrix;\n" \
"uniform mat4 mvp_matrix;\n" \
"uniform mat3 normal_matrix;\n" \
"\n" \
"void main()\n" \
"{\n" \
"	vec4 pos = vec4(position, 1.0);\n" \
"	vec4 norm = vec4(normal, 0.0);\n" \
"\n" \
"	vs_out.gPosition = vec3(model_matrix * pos);\n" \
"	vs_out.gNormal = normalize(normal_matrix * norm.xyz);\n" \
"	vs_out.gColor = color;\n" \
"	vs_out.gTexCoord = texCoord;\n" \
"\n" \
"	gl_Position = mvp_matrix * pos;\n" \
"}"

#define DecalFragment \
"#version 330 core\n" \
"layout(location = 0) out vec4 gPosition;\n" \
"layout(location = 1) out vec4 gNormal;\n" \
"layout(location = 2) out vec4 gAlbedoSpec;\n" \
"layout(location = 3) out uvec4 gInfo;\n" \
"\n" \
"in VS_OUT\n" \
"{\n" \
"	vec3 gPosition;\n" \
"	vec3 gNormal;\n" \
"	vec4 gColor;\n" \
"	vec2 gTexCoord;\n" \
"} fs_in;\n" \
"\n" \
"uniform sampler2D gBufferPosition;\n" \
"uniform sampler2D gBufferNormal;\n" \
"uniform sampler2D gBufferInfo;\n" \
"\n" \
"uniform sampler2D projectorTex;\n" \
"\n" \
"uniform int lightCartoon;\n" \
"\n" \
"uniform mat4 model_matrix;\n" \
"uniform mat4 projectorMatrix;\n" \
"\n" \
"uniform vec2 screenSize;\n" \
"\n" \
"uniform float alphaMultiplier;\n" \
"\n" \
"struct Fog\n" \
"{\n" \
"	vec3 color;\n" \
"	//float minDist;\n" \
"	//float maxDist;\n" \
"	float density;\n" \
"};\n" \
"\n" \
"uniform Fog fog;\n" \
"uniform mat4 view_matrix;\n" \
"\n" \
"void main()\n" \
"{\n" \
"	// fragment's screen pos\n" \
"	vec2 screenPos = gl_FragCoord.xy;\n" \
"	screenPos.x /= screenSize.x;\n" \
"	screenPos.y /= screenSize.y;\n" \
"\n" \
"	// Dot shader\n" \
"	vec4 dotInfo = texture(gBufferInfo, screenPos);\n" \
"	if (dotInfo.g == 1)\n" \
"		discard;\n" \
"\n" \
"	// gBuffer fragment's world pos\n" \
"	vec4 worldPos = texture(gBufferPosition, screenPos);\n" \
"	if (worldPos.w == 3)\n" \
"		discard;\n" \
"	worldPos.w = 1.0;\n" \
"\n" \
"	if (worldPos.z == 0.0)\n" \
"		discard;\n" \
"	// gBuffer fragment's object pos\n" \
"	vec4 objectPos = inverse(model_matrix) * worldPos;\n" \
"\n" \
"	// Bounds check\n" \
"	float objX = 0.5 - abs(objectPos.x);\n" \
"	float objY = 0.5 - abs(objectPos.y);\n" \
"	float objZ = 0.5 - abs(objectPos.z);\n" \
"	if (objX < 0.0 || objY < 0.0 || objZ < 0.0)\n" \
"		discard;\n" \
"\n" \
"	// Calculate tex coord\n" \
"	//vec2 texCoord = objectPos.xy + 0.5;\n" \
"	//vec4 color = texture(projectorTex, texCoord);\n" \
"	vec4 texCoord = projectorMatrix * worldPos;\n" \
"	vec4 color = textureProj(projectorTex, texCoord);\n" \
"	if (color.a == 0.0)\n" \
"		discard;\n" \
"\n" \
"	//////////\n" \
"\n" \
"	gPosition.rgb = worldPos.rgb;\n" \
"	gNormal.rgb = texture(gBufferNormal, screenPos).xyz;\n" \
"	gAlbedoSpec = vec4(color.rgb, color.a * alphaMultiplier);\n" \
"\n" \
"	vec4 modelViewPos = view_matrix * vec4(fs_in.gPosition, 1.0);\n" \
"	float dist = length(modelViewPos.xyz);\n" \
"	//float fogFactor = (fog.maxDist - dist) / (fog.maxDist - fog.minDist); // Linear\n" \
"	//float fogFactor = exp(-fog.density * dist); // Exponential\n" \
"	float fogFactor = exp(-pow(fog.density * dist, 2.0)); // Exponential Squared\n" \
"	fogFactor = clamp(fogFactor, 0.0, 1.0);\n" \
"	gAlbedoSpec = mix(vec4(fog.color, 1.0), gAlbedoSpec, fogFactor);\n" \
"\n" \
"	gPosition.a = lightCartoon;\n" \
"	int levels = 2;\n" \
"	gNormal.a = levels;\n" \
"}"
#pragma endregion

#endif