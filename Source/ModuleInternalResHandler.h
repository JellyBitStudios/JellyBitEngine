#ifndef __MODULE_INTERNAL_RES_HANDLER_H__
#define __MODULE_INTERNAL_RES_HANDLER_H__

#include "Module.h"
#include "Globals.h"

#include "ResourceShaderProgram.h"

#define PLANE_UUID 1428675893
#define CUBE_UUID 1451315056
#define DEFAULT_SHADER_PROGRAM_UUID 1608702687
#define DEFERRED_SHADER_PROGRAM_UUID 1708702688
#define BILLBOARD_SHADER_PROGRAM_UUID 1708712988
#define CARTOON_SHADER_PROGRAM_UUID 2628543447
#define CARTOON_FLOOR_SHADER_PROGRAM_UUID 3828343447
#define DEFAULT_SHADER_PROGRAM_PARTICLE_UUID 2628722347
#define DEFAULT_SHADER_PROGRAM_UI_UUID 1246832795 
#define CUBEMAP_SHADER_PROGRAM_UUID 1676961097
#define DEFAULT_MATERIAL_UUID 2168314292
#define REPLACE_ME_TEXTURE_UUID 3462814329
#define CHECKERS_TEXTURE_UUID 1162820329

enum ShaderProgramTypes;

class ModuleInternalResHandler : public Module
{
public:

	bool Start();
	bool CleanUp();

	// Mesh resources
	void CreatePlane();
	void CreateCube();

	// Texture resources
	void CreateCheckers();
	void CreateDefaultTexture();
	void CreateLightIcon();

	// Shader resources
	void CreateDefaultShaderProgram(const char* vShader, const char* fShader, ShaderProgramTypes type);
	void CreateDeferredShaderProgram();
	void CreateBillboardShaderProgram();
	uint CreateCartoonShaderProgram() const;
	uint CreateFloorCartoonShaderProgram() const;

	void CreateUIShaderProgram();

	// Material resources
	void CreateDefaultMaterial();

public:

	// Mesh resources
	uint plane;
	uint cube;

	// Texture resources
	uint checkers;
	uint defaultTexture;
	uint lightIcon;

	// Shader resources
	uint defaultShaderProgram;
	uint deferredShaderProgram; // Used at FBO
	uint billboardShaderProgram;
	uint particleShaderProgram;
	uint UIVertexShaderObject;
	uint UIFragmentShaderObject;
	uint UIShaderProgram;
	uint cartoonShaderProgram;
	uint cartoonFloorProgram;

	// Material resources
	uint defaultMaterial;
};

#endif