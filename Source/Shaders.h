#ifndef __SHADERS_H__
#define __SHADERS_H__

#pragma region ShaderDefault

#define vShaderTemplate											\
"#version 330 core\n"											\
"layout(location = 0) in vec3 position;\n"						\
"layout(location = 1) in vec3 normal;\n"						\
"layout(location = 2) in vec4 color;\n"							\
"layout(location = 3) in vec2 texCoord;\n"						\
"uniform mat4 model_matrix;\n"									\
"uniform mat4 mvp_matrix;\n"									\
"uniform mat3 normal_matrix;\n"									\
"out vec3 fPosition;\n"											\
"out vec3 fNormal;\n"											\
"out vec2 fTexCoord;\n"											\
"void main()\n"													\
"{\n"															\
"	//fPosition = vec3(model_matrix * vec4(position, 1.0));\n"	\
"	fNormal = normalize(normal_matrix * normal);\n"				\
"	fTexCoord = texCoord;\n"									\
"	gl_Position = mvp_matrix * vec4(position, 1.0);\n"			\
"}"

#define fShaderTemplate															\
"#version 330 core\n"															\
"layout (location = 0) out vec4 gPosition;\n"									\
"layout (location = 1) out vec4 gNormal;\n"										\
"layout (location = 2) out vec4 gAlbedoSpec;\n"									\
"in vec2 fTexCoord;\n"															\
"in vec3 fPosition;\n"															\
"in vec3 fNormal;\n"															\
"uniform sampler2D diffuse;\n"													\
"void main()\n"																	\
"{\n"																			\
"	gPosition.rgb = fPosition;\n"												\
"	gNormal.rgb = normalize(fNormal);\n"										\
"	gAlbedoSpec.rgb = texture(diffuse,fTexCoord).rgb;\n"						\
"	gPosition.a = 1;\n"															\
"	gNormal.a = 1;\n"															\
"	gAlbedoSpec.a = 1;\n"														\
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

#define fDEFERREDSHADING																									\
"#version 330 core\n"																										\
"out vec4 FragColor;\n"																										\
"in vec2 TexCoords;\n"																										\
"uniform sampler2D gPosition;\n"																							\
"uniform sampler2D gNormal;\n"																								\
"uniform sampler2D gAlbedoSpec;\n"																							\
"struct Light {\n"																											\
"int type;\n"																												\
"vec3 dir;\n"																												\
"vec3 position;\n"																											\
"vec3 color;\n"																												\
"float linear;\n"																											\
"float quadratic;\n"																										\
"};\n"																														\
"const int NR_LIGHTS = 32;\n"																								\
"uniform Light lights[NR_LIGHTS];\n"																						\
"void main()\n"																												\
"{\n"																														\
"	// retrieve data from gbuffer\n"																						\
"	vec3 FragPos = texture(gPosition, TexCoords).rgb;\n"																	\
"	vec3 Normal = texture(gNormal, TexCoords).rgb;\n"																		\
"	vec3 Albedo = texture(gAlbedoSpec, TexCoords).rgb;\n"																	\
"	vec3 lighting = Albedo * 0.3; // hard-coded ambient component\n"														\
"	for (int i = 0; i < NR_LIGHTS; ++i) \n"																					\
"	{\n"																													\
"		vec3 diffuse = vec3(0.0,0.0,0.0);\n"																				\
"		if (lights[i].type == 1)\n"																							\
"			diffuse = max(dot(Normal, lights[i].dir), 0.0) * Albedo * lights[i].color;\n"									\
"		else if (lights[i].type == 2)\n"																					\
"		{\n"																												\
"			 vec3 lightDir = normalize(lights[i].position - FragPos);\n"													\
"			 diffuse = max(dot(Normal, lightDir), 0.0) * Albedo * lights[i].color;\n"										\
"			 float distance = length(lights[i].position - FragPos);\n"														\
"			 float attenuation = 1.0 / (1.0 + lights[i].linear * distance + lights[i].quadratic * distance * distance);\n"	\
"			 diffuse *= attenuation;\n"																						\
"		}\n"																												\
"		lighting += diffuse;\n"																								\
"	}\n"																													\
"	FragColor = vec4(lighting, 1.0);\n"																						\
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
"	FragColor = texture(diffuse, fTexCoord);\n"		\
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
"uniform vec3 viewPos;\n" \
"uniform Light light;\n" \
"uniform Material material;\n" \
"uniform float averageColor;\n" \
"\n" \
"vec3 phong(vec3 ambient, vec3 diffuse, vec3 specular, float shininess, bool blinn)\n" \
"{\n" \
"	// Ambient\n" \
"	vec3 a = light.ambient * ambient;\n" \
"\n" \
"	// Diffuse\n" \
"	vec3 lightDir = normalize(-light.direction);\n" \
"	float diff = max(dot(fNormal, lightDir), 0.0);\n" \
"	vec3 d = light.diffuse * (diff * diffuse);\n" \
"\n" \
"	// Specular\n" \
"	vec3 viewDir = normalize(viewPos - fPosition);\n" \
"	float spec = 0.0;\n" \
"	if (blinn)\n" \
"	{\n" \
"		vec3 halfwayDir = normalize(lightDir + viewDir);\n" \
"		spec = pow(max(dot(fNormal, halfwayDir), 0.0), shininess);\n" \
"	}\n" \
"	else\n" \
"	{\n" \
"		vec3 reflectDir = reflect(-lightDir, fNormal);\n" \
"		spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);\n" \
"	}\n" \
"	vec3 s = light.specular * (spec * specular);\n" \
"\n" \
"	return a + d + s;\n" \
"}\n" \
"\n" \
"void main()\n" \
"{\n" \
"	vec4 albedo = texture(material.albedo, fTexCoord);\n" \
"	if (albedo.a < 0.1)\n" \
"		discard;\n" \
"\n" \
"	vec3 a = vec3(albedo);\n" \
"	vec3 s = vec3(texture(material.specular, fTexCoord));\n" \
"	vec3 phong = phong(a, a, s, 32.0, true);\n" \
"   vec4 textColor = vec4(phong, albedo.a);\n" \
"	FragColor = mix(textColor, fColor, averageColor);" \
"}"
#pragma endregion

#pragma region ShaderUI
//UI
#define uivShader \
"#version 330 core\n" \
"layout(location = 0) in vec2 vertex; // <vec2 position, vec2 texCoords>\n" \
"layout(location = 1) in vec2 texture_coords; // <vec2 position, vec2 texCoords>\n" \
"out vec2 TexCoords;\n" \
"uniform int isScreen;\n" \
"uniform mat4 mvp_matrix;\n" \
"uniform vec3 topRight;\n" \
"uniform vec3 topLeft;\n" \
"uniform vec3 bottomLeft;\n" \
"uniform vec3 bottomRight;\n" \
"void main()\n" \
"{\n" \
"	vec3 position = topRight;\n" \
"	if (vertex.x > 0.0 && vertex.y > 0.0)\n" \
"	{\n" \
"		position = topRight;\n" \
"		if (isScreen == 0)\n" \
"			TexCoords = vec2(0.0, 1.0);\n" \
"	}"\
"	else if (vertex.x > 0.0 && vertex.y < 0.0)\n" \
"	{\n" \
"		position = bottomRight;\n" \
"		if (isScreen == 0)\n" \
"			TexCoords = vec2(0.0,0.0);\n" \
"	}"\
"	else if (vertex.x < 0.0 && vertex.y > 0.0)\n" \
"	{\n" \
"		position = topLeft;\n" \
"		if (isScreen == 0)\n" \
"			TexCoords = vec2(1.0,1.0);\n" \
"	}"\
"	else if (vertex.x < 0.0 && vertex.y < 0.0)\n" \
"	{\n" \
"		position = bottomLeft;\n" \
"		if (isScreen == 0)\n" \
"			TexCoords = vec2(1.0,0.0);\n" \
"	}"\
"	if(isScreen == 1)\n"\
"	{\n" \
"		gl_Position = vec4(position, 1.0);\n" \
"		TexCoords = texture_coords;\n" \
"	}"\
"	else\n"\
"		gl_Position = mvp_matrix * vec4(position, 1.0);\n" \
"}"

#define uifShader \
"#version 330 core\n" \
"in vec2 TexCoords;\n" \
"out vec4 FragColor;\n" \
"uniform int use_color;\n"\
"uniform sampler2D image;\n" \
"uniform vec4 spriteColor;\n" \
"void main()\n" \
"{\n" \
"	if(use_color == 1)\n"\
"		FragColor = spriteColor;\n" \
"	else\n"\
"		FragColor = texture(image, TexCoords);\n" \
"}"

#pragma endregion

#endif