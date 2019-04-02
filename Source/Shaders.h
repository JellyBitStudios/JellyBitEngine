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

#define fShaderTemplate															\
"#version 330 core\n"															\
"layout (location = 0) out vec4 gPosition;\n"									\
"layout (location = 1) out vec4 gNormal;\n"										\
"layout (location = 2) out vec4 gAlbedoSpec;\n"									\
"\n"																			\
"in VS_OUT\n"																	\
"{\n"																			\
"	vec3 gPosition;\n"															\
"	vec3 gNormal;\n"															\
"	vec4 gColor;\n"																\
"	vec2 gTexCoord;\n"															\
"} fs_in;\n"																	\
"\n"																			\
"uniform sampler2D diffuse;\n"													\
"\n"																			\
"void main()\n"																	\
"{\n"																			\
"	gPosition.rgb = fs_in.gPosition;\n"											\
"	gNormal.rgb = normalize(fs_in.gNormal);\n"									\
"	gAlbedoSpec.rgb = texture(diffuse, fs_in.gTexCoord).rgb;\n"					\
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

#define fDEFERREDSHADING \
"#version 330 core\n" \
"out vec4 FragColor;\n" \
"in vec2 TexCoords;\n" \
"uniform sampler2D gPosition;\n" \
"uniform sampler2D gNormal;\n" \
"uniform sampler2D gAlbedoSpec;\n" \
"uniform sampler2D gDepth;\n" \
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
"uniform Light lights[NR_LIGHTS];\n" \
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
"	vec3 lighting = Albedo * 0.3; // hard-coded ambient component\n" \
"	for (int i = 0; i < NR_LIGHTS; ++i)\n" \
"	{\n" \
"		vec3 diffuse = vec3(0.0, 0.0, 0.0);\n" \
"		if (lights[i].type == 1)\n" \
"		{\n" \
"			if (FragPosA == 2) // cartoon\n" \
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
"			if (FragPosA == 2) // cartoon\n" \
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
"	if (FragPosA == 3) // outline\n" \
"		lighting = Albedo;\n" \
"	FragColor = vec4(lighting, 1.0);\n" \
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
"uniform vec3 viewPos;\n" \
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
#define uivShader 																		\
"#version 330 core\n" 																	\
"#extension GL_ARB_enhanced_layouts : enable\n"											\
"layout(location = 0) in vec2 vertex; // <vec2 position, vec2 texCoords>\n" 			\
"layout(location = 1) in vec2 texture_coords; // <vec2 position, vec2 texCoords>\n" 	\
"out vec2 TexCoords;\n"																	\
"uniform int isScreen;\n" 																\
"uniform mat4 mvp_matrix;\n" 															\
"\n"																					\
"layout (std140) uniform UIBlock {\n"													\
"	layout (offset = 0) int useMask;\n" 												\
"	layout (offset = 8) vec2 coordsMask;\n" 											\
"	layout (offset = 16) vec3 topLeft;\n"												\
"	layout (offset = 32) vec3 topRight;\n"												\
"	layout (offset = 48) vec3 bottomLeft;\n"											\
"	layout (offset = 64) vec3 bottomRight;\n"											\
"};\n"																					\
"\n"																					\
"uniform int isLabel;\n" 																\
"void main()\n" 																		\
"{\n" 																					\
"	vec3 position = topRight;\n" 														\
"	if (vertex.x > 0.0 && vertex.y > 0.0)\n" 											\
"	{\n" 																				\
"		position = topRight;\n" 														\
"		if (isScreen == 0)\n" 															\
"		{\n" 																			\
"			if (isLabel == 0)\n" 														\
"				TexCoords = vec2(0.0, 1.0);\n" 											\
"			else\n"																		\
"				TexCoords = vec2(0.0, 0.0);\n" 											\
"		}"																				\
"		else\n"																			\
"			if (useMask == 1)\n" 														\
"				TexCoords = vec2(coordsMask.x,1.0);\n" 									\
"	}"																					\
"	else if (vertex.x > 0.0 && vertex.y < 0.0)\n" 										\
"	{\n" 																				\
"		position = bottomRight;\n" 														\
"		if (isScreen == 0)\n" 															\
"		{\n" 																			\
"			if (isLabel == 0)\n" 														\
"			{\n" 																		\
"				if (useMask == 0)\n" 												\
"					TexCoords = vec2(0.0,0.0);\n" 										\
"				else\n"																	\
"					TexCoords = vec2(0.0,coordsMask.y);\n" 								\
"			}"																			\
"			else\n"																		\
"				TexCoords = vec2(0.0,1.0);\n" 											\
"		}"																				\
"		else\n"																			\
"			if (useMask == 1)\n" 													\
"				TexCoords = vec2(coordsMask.x,coordsMask.y);\n" 						\
"	}"																					\
"	else if (vertex.x < 0.0 && vertex.y > 0.0)\n" 										\
"	{\n" 																				\
"		position = topLeft;\n" 															\
"		if (isScreen == 0)\n" 															\
"		{\n" 																			\
"			if (isLabel == 0)\n" 														\
"			{\n" 																		\
"				if (useMask == 0)\n" 												\
"					TexCoords = vec2(1.0,1.0);\n" 										\
"				else\n"																	\
"					TexCoords = vec2(coordsMask.x,1.0);\n" 								\
"			}"																			\
"			else\n"																		\
"				TexCoords = vec2(1.0,0.0);\n" 											\
"		}"																				\
"		else\n"																			\
"			if (useMask == 1)\n" 													\
"				TexCoords = vec2(0.0,1.0);\n" 											\
"	}"																					\
"	else if (vertex.x < 0.0 && vertex.y < 0.0)\n" 										\
"	{\n" 																				\
"		position = bottomLeft;\n" 														\
"		if (isScreen == 0)\n" 															\
"		{\n" 																			\
"			if (isLabel == 0)\n" 														\
"			{\n" 																		\
"				if (useMask == 0)\n" 												\
"					TexCoords = vec2(1.0,0.0);\n" 										\
"				else\n"																	\
"					TexCoords = vec2(coordsMask.x,coordsMask.y);\n" 					\
"			}"																			\
"			else\n"																		\
"				TexCoords = vec2(1.0,1.0);\n" 											\
"		}"																				\
"		else\n"																			\
"			if (useMask == 1)\n" 													\
"				TexCoords = vec2(0.0,coordsMask.y);\n" 									\
"	}"																					\
"	if(isScreen == 1)\n"																\
"	{\n" 																				\
"		gl_Position = vec4(position,1.0);\n" 														\
"		if (useMask == 0)\n" 														\
"			TexCoords = texture_coords;\n" 												\
"	}"																					\
"	else\n"																				\
"		gl_Position = mvp_matrix *  vec4(position, 1.0);\n" 									\
"}"

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
" if(isLabel == 0)\n"																	\
" {\n"																					\
"	  if(using_texture == 1)\n"															\
"		 FragColor = texture(image, TexCoords) * spriteColor;\n" 						\
"	  else\n"																			\
"	  	FragColor = spriteColor;\n" 													\
" }\n"																					\
"	else\n"																				\
"	{\n" 																				\
"	  vec4 sampled = vec4(1.0, 1.0, 1.0, texture(image, TexCoords).r);\n" 				\
"		FragColor = spriteColor * sampled; \n" 											\
"	}\n"																				\
"}\n"
#pragma endregion

#pragma region CartoonShader

#define CartoonGeometry																				\
"#version 330 core\n"																				\
"\n"																								\
"layout(triangles_adjacency) in;\n"																	\
"layout(triangle_strip, max_vertices = 255) out;\n"													\
"\n"																								\
"in VS_OUT\n"																						\
"{\n"																								\
"  vec3 gPosition;\n"																				\
"  vec3 gNormal;\n"																					\
"  vec4 gColor;\n"																					\
"  vec2 gTexCoord;\n"																				\
"} gs_in[];\n"																						\
"\n"																								\
"out GS_OUT\n"																						\
"{\n"																								\
"  vec3 fPosition;\n"																				\
"  vec3 fNormal;\n"																					\
"  vec4 fColor;\n"																					\
"  vec2 fTexCoord;\n"																				\
"} gs_out;\n"																						\
"\n"																								\
"flat out int fIsEdge; // which output primitives are silhouette edges\n"							\
"\n"																								\
"//uniform float edgeWidth; // width of silhouette edge in clip\n"									\
"//uniform float pctExtend; // percentage to extend quad\n"											\
"\n"																								\
"bool isFrontFacing(vec3 a, vec3 b, vec3 c) // is a triangle front facing?\n"						\
"{\n"																								\
"	// Compute the triangle's z coordinate of the normal vector (cross product)\n"					\
"	return ((a.x * b.y - b.x * a.y) + (b.x * c.y - c.x * b.y) + (c.x * a.y - a.x * c.y)) > 0;\n"	\
"}\n"																								\
"\n"																								\
"void emitEdgeQuad(vec3 e0, vec3 e1)\n"																\
"{\n"																								\
"	float edgeWidth = 0.002;\n"																		\
"	float pctExtend = 0.0;\n"																		\
"\n"																								\
"	vec2 ext = pctExtend * (e1.xy - e0.xy);\n"														\
"	vec2 v = normalize(e1.xy - e0.xy);\n"															\
"	vec2 n = vec2(-v.y, v.x) * edgeWidth;\n"														\
"\n"																								\
"	// Emit the quad\n"																				\
"	fIsEdge = 1; // this is part of an edge\n"														\
"\n"																								\
"	gl_Position = vec4(e0.xy - ext, e0.z, 1.0);\n"													\
"	EmitVertex();\n"																				\
"	gl_Position = vec4(e0.xy - n - ext, e0.z, 1.0);\n"												\
"	EmitVertex();\n"																				\
"	gl_Position = vec4(e1.xy + ext, e1.z, 1.0);\n"													\
"	EmitVertex();\n"																				\
"	gl_Position = vec4(e1.xy - n + ext, e1.z, 1.0);\n"												\
"	EmitVertex();\n"																				\
"\n"																								\
"	EndPrimitive();\n"																				\
"}\n"																								\
"\n"																								\
"void main()\n"																						\
"{\n"																								\
"	vec3 p0 = gl_in[0].gl_Position.xyz / gl_in[0].gl_Position.w;\n"									\
"	vec3 p1 = gl_in[1].gl_Position.xyz / gl_in[1].gl_Position.w;\n"									\
"	vec3 p2 = gl_in[2].gl_Position.xyz / gl_in[2].gl_Position.w;\n"									\
"	vec3 p3 = gl_in[3].gl_Position.xyz / gl_in[3].gl_Position.w;\n"									\
"	vec3 p4 = gl_in[4].gl_Position.xyz / gl_in[4].gl_Position.w;\n"									\
"	vec3 p5 = gl_in[5].gl_Position.xyz / gl_in[5].gl_Position.w;\n"									\
"\n"																								\
"	if (isFrontFacing(p0, p2, p4))\n"																\
"	{\n"																							\
"		if (!isFrontFacing(p0, p1, p2))\n"															\
"			emitEdgeQuad(p0, p2);\n"																\
"		if (!isFrontFacing(p2, p3, p4))\n"															\
"			emitEdgeQuad(p2, p4);\n"																\
"		if (!isFrontFacing(p4, p5, p0))\n"															\
"			emitEdgeQuad(p4, p0);\n"																\
"	}\n"																							\
"\n"																								\
"	// Output the original triangle\n"																\
"	fIsEdge = 0; // this is not part of an edge\n"													\
"\n"																								\
"	gs_out.fPosition = gs_in[0].gPosition;\n"														\
"	gs_out.fNormal = gs_in[0].gNormal;\n"															\
"	gs_out.fColor = gs_in[0].gColor;\n"																\
"	gs_out.fTexCoord = gs_in[0].gTexCoord;\n"														\
"	gl_Position = gl_in[0].gl_Position;\n"															\
"	EmitVertex();\n"																				\
"\n"																								\
"	gs_out.fPosition = gs_in[2].gPosition;\n"														\
"	gs_out.fNormal = gs_in[2].gNormal;\n"															\
"	gs_out.fColor = gs_in[2].gColor;\n"																\
"	gs_out.fTexCoord = gs_in[2].gTexCoord;\n"														\
"	gl_Position = gl_in[2].gl_Position;\n"															\
"	EmitVertex();\n"																				\
"\n"																								\
"	gs_out.fPosition = gs_in[4].gPosition;\n"														\
"	gs_out.fNormal = gs_in[4].gNormal;\n"															\
"	gs_out.fColor = gs_in[4].gColor;\n"																\
"	gs_out.fTexCoord = gs_in[4].gTexCoord;\n"														\
"	gl_Position = gl_in[4].gl_Position;\n"															\
"	EmitVertex();\n"																				\
"\n"																								\
"	EndPrimitive();\n"																				\
"}"

#define CartoonFragment																	\
"#version 330 core\n"																	\
"\n"																					\
"layout(location = 0) out vec4 gPosition;\n"											\
"layout(location = 1) out vec4 gNormal;\n"												\
"layout(location = 2) out vec4 gAlbedoSpec;\n"											\
"\n"																					\
"in GS_OUT\n"																			\
"{\n"																					\
"  vec3 fPosition;\n"																	\
"  vec3 fNormal;\n"																		\
"  vec4 fColor;\n"																		\
"  vec2 fTexCoord;\n"																	\
"} fs_in;\n"																			\
"\n"																					\
"flat in int fIsEdge; // whether or not we're drawing an edge\n"						\
"\n"																					\
"struct Material\n"																		\
"{\n"																					\
"	sampler2D albedo;\n"																\
"};\n"																					\
"\n"																					\
"uniform vec3 viewPos;\n"																\
"uniform Material material;\n"															\
"\n"																					\
"//uniform vec3 lineColor; // the silhouette edge color\n"								\
"//uniform int levels;\n"																\
"\n"																					\
"void main()\n"																			\
"{\n"																					\
"	vec3 lineColor = vec3(0.0, 0.0, 0.0);\n"											\
"	int levels = 2;\n"																	\
"\n"																					\
"	// If we're drawing an edge, use constant color\n"									\
"	if (fIsEdge == 1)\n"																\
"	{\n"																				\
"		gNormal.a = 1;\n"																\
"		gPosition.a = 3;\n"																\
"\n"																					\
"		gAlbedoSpec = vec4(lineColor, 1.0);\n"											\
"	}\n"																				\
"	// Otherwise, shade the poly\n"														\
"	else\n"																				\
"	{\n"																				\
"		gNormal.a = levels;\n"															\
"		gPosition.a = 2;\n"																\
"\n"																					\
"		vec4 albedo = texture(material.albedo, fs_in.fTexCoord);\n"						\
"		vec3 diffuse = vec3(albedo);\n"													\
"		gAlbedoSpec = vec4(diffuse, albedo.a);\n"										\
"	}\n"																				\
"\n"																					\
"	gPosition.rgb = fs_in.fPosition;\n"													\
"	gNormal.rgb = normalize(fs_in.fNormal);\n"											\
"}"

#define CartoonFloorFragment																\
"#version 330 core\n"																	\
"\n"																					\
"layout(location = 0) out vec4 gPosition;\n"											\
"layout(location = 1) out vec4 gNormal;\n"												\
"layout(location = 2) out vec4 gAlbedoSpec;\n"											\
"\n"																					\
"in GS_OUT\n"																			\
"{\n"																					\
"  vec3 fPosition;\n"																	\
"  vec3 fNormal;\n"																		\
"  vec4 fColor;\n"																		\
"  vec2 fTexCoord;\n"																	\
"} fs_in;\n"																			\
"\n"																					\
"flat in int fIsEdge; // whether or not we're drawing an edge\n"						\
"\n"																					\
"struct Material\n"																		\
"{\n"																					\
"	sampler2D albedo;\n"																\
"};\n"																					\
"\n"																					\
"uniform vec3 viewPos;\n"																\
"uniform Material material;\n"															\
"uniform vec2 repeat = vec2(2, 2);\n"															\
"\n"																					\
"//uniform vec3 lineColor; // the silhouette edge color\n"								\
"//uniform int levels;\n"																\
"\n"																					\
"void main()\n"																			\
"{\n"																					\
"	vec3 lineColor = vec3(0.0, 0.0, 0.0);\n"											\
"	int levels = 2;\n"																	\
"\n"																					\
"	// If we're drawing an edge, use constant color\n"									\
"	if (fIsEdge == 1)\n"																\
"	{\n"																				\
"		gNormal.a = 1;\n"																\
"		gPosition.a = 3;\n"																\
"\n"																					\
"		gAlbedoSpec = vec4(lineColor, 1.0);\n"											\
"	}\n"																				\
"	// Otherwise, shade the poly\n"														\
"	else\n"																				\
"	{\n"																				\
"		gNormal.a = levels;\n"															\
"		gPosition.a = 2;\n"																\
"\n"																					\
"		vec4 albedo = texture2D(material.albedo, vec2(mod(fs_in.fTexCoord.x * repeat.x, 1), mod(fs_in.fTexCoord.y * repeat.y, 1)));\n"						\
"		vec3 diffuse = vec3(albedo);\n"													\
"		gAlbedoSpec = vec4(diffuse, albedo.a);\n"										\
"	}\n"																				\
"\n"																					\
"	gPosition.rgb = fs_in.fPosition;\n"													\
"	gNormal.rgb = normalize(fs_in.fNormal);\n"											\
"}"

#pragma endregion

#endif
