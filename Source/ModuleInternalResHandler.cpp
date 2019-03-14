#include "ModuleInternalResHandler.h"

#include "Application.h"
#include "ModuleResourceManager.h"
#include "MaterialImporter.h"

#include "ResourceMesh.h"
#include "ResourceTexture.h"
#include "ResourceMaterial.h"
#include "ResourceShaderObject.h"
#include "ResourceShaderProgram.h"

#include "Shaders.h"

bool ModuleInternalResHandler::Start()
{
	// Mesh resources
	CreatePlane();
	CreateCube();

	// Texture resources
	CreateCheckers();
	CreateDefaultTexture();
	CreateLightIcon();

	// Shader resources
	CreateDefaultShaderProgram(vShaderTemplate, fShaderTemplate, ShaderProgramTypes::Standard);
	CreateDefaultShaderProgram(Particle_vShaderTemplate, Particle_fShaderTemplate, ShaderProgramTypes::Particles);
	CreateDeferredShaderProgram();
	CreateBillboardShaderProgram();
	CreateUIShaderProgram();
	cartoonShaderProgram = CreateCartoonShaderProgram();

	// Material resources
	CreateDefaultMaterial();

	return true;
}

bool ModuleInternalResHandler::CleanUp()
{
	App->res->SetAsUnused(plane);
	App->res->SetAsUnused(lightIcon);

	return true;
}

void ModuleInternalResHandler::CreatePlane()
{
	ResourceMeshData specificData;

	specificData.verticesSize = 4;
	specificData.vertices = new Vertex[specificData.verticesSize];

	float verticesPosition[12]
	{
		-1.0f,  1.0f, 0.0f, // a
		-1.0f, -1.0f, 0.0f, // b
		 1.0f,  1.0f, 0.0f, // c
		 1.0f, -1.0f, 0.0f, // d
	};

	uint textCordsSize = 8;
	float textCordsPosition[8]
	{
		 0.0f,  1.0f, // a
		 0.0f,  0.0f, // b
		 1.0f,  1.0f, // c
		 1.0f,  0.0f, // d
	};

	specificData.indicesSize = 6;
	specificData.indices = new uint[specificData.indicesSize]
	{
		// Front
		0, 1, 2, // ABC
		2, 1, 3, // BDC
	};

	// Vertices
	/// Position
	float* cursor = verticesPosition;
	for (uint i = 0; i < specificData.verticesSize; ++i)
	{
		memcpy(specificData.vertices[i].position, cursor, sizeof(float) * 3);
		cursor += 3;
	}

	///TextCords
	cursor = textCordsPosition;
	for (uint i = 0; i < textCordsSize / 2; ++i)
	{
		memcpy(specificData.vertices[i].texCoord, cursor, sizeof(float) * 2);
		cursor += 2;
	}

	ResourceData data;
	data.name = "Default Plane";

	plane = App->res->CreateResource(ResourceTypes::MeshResource, data, &specificData, PLANE_UUID)->GetUuid();

	// need this for deferred shading
	App->res->SetAsUsed(plane);
}

void ModuleInternalResHandler::CreateCube()
{
	ResourceMeshData specificData;

	specificData.verticesSize = 8;
	specificData.vertices = new Vertex[specificData.verticesSize];

	float verticesPosition[24]
	{
		-1, -1, -1,
		 1, -1, -1,
		 1,  1, -1,
		-1,  1, -1,
		-1, -1,  1,
		 1, -1,  1,
		 1,  1,  1,
		-1,  1,  1
	};

	specificData.indicesSize = 36;
	specificData.indices = new uint[specificData.indicesSize]
	{
		0, 1, 3, 3, 1, 2,
		1, 5, 2, 2, 5, 6,
		5, 4, 6, 6, 4, 7,
		4, 0, 7, 7, 0, 3,
		3, 2, 7, 7, 2, 6,
		4, 5, 0, 0, 5, 1
	};

	// Vertices
	/// Position
	float* cursor = verticesPosition;
	for (uint i = 0; i < specificData.verticesSize; ++i)
	{
		memcpy(specificData.vertices[i].position, cursor, sizeof(float) * 3);
		cursor += 3;
	}

	ResourceData data;
	data.name = "Default Cube";
	data.internal = true;
	
	cube = App->res->CreateResource(ResourceTypes::MeshResource, data, &specificData, CUBE_UUID)->GetUuid();
}

void ModuleInternalResHandler::CreateCheckers()
{
	ResourceData data;
	ResourceTextureData textureData;

	data.name = "Checkers";

	uchar checkImage[CHECKERS_HEIGHT][CHECKERS_WIDTH][4];

	for (uint i = 0; i < CHECKERS_HEIGHT; i++) {
		for (uint j = 0; j < CHECKERS_WIDTH; j++) {
			int c = ((((i & 0x8) == 0) ^ (((j & 0x8)) == 0))) * 255;
			checkImage[i][j][0] = (uchar)c;
			checkImage[i][j][1] = (uchar)c;
			checkImage[i][j][2] = (uchar)c;
			checkImage[i][j][3] = (uchar)255;
		}
	}

	uint size = CHECKERS_HEIGHT * CHECKERS_WIDTH * 4;
	textureData.data = new uchar[size];
	memcpy(textureData.data, checkImage, size);

	textureData.width = CHECKERS_WIDTH;
	textureData.height = CHECKERS_HEIGHT;
	textureData.bpp = 4;

	textureData.textureImportSettings.wrapS = ResourceTextureImportSettings::TextureWrapMode::REPEAT;
	textureData.textureImportSettings.wrapT = ResourceTextureImportSettings::TextureWrapMode::REPEAT;
	textureData.textureImportSettings.minFilter = ResourceTextureImportSettings::TextureFilterMode::NEAREST;
	textureData.textureImportSettings.magFilter = ResourceTextureImportSettings::TextureFilterMode::NEAREST;

	checkers = (App->res->CreateResource(ResourceTypes::TextureResource, data, &textureData, CHECKERS_TEXTURE_UUID))->GetUuid();
}

void ModuleInternalResHandler::CreateDefaultTexture()
{
	ResourceData data;
	ResourceTextureData textureData;

	data.name = "Replace me!";

	uchar replaceMeTexture[REPLACE_ME_WIDTH][REPLACE_ME_HEIGHT][4]; // REPLACE ME!

	for (uint i = 0; i < 2; i++) {
		for (uint j = 0; j < 2; j++) {
			replaceMeTexture[i][j][0] = (uchar)190;
			replaceMeTexture[i][j][1] = (uchar)178;
			replaceMeTexture[i][j][2] = (uchar)137;
			replaceMeTexture[i][j][3] = (uchar)255;
		}
	}

	uint size = REPLACE_ME_HEIGHT * REPLACE_ME_WIDTH * 4;
	textureData.data = new uchar[size];
	memcpy(textureData.data, replaceMeTexture, size);

	textureData.width = REPLACE_ME_WIDTH;
	textureData.height = REPLACE_ME_HEIGHT;
	textureData.bpp = 4;

	textureData.textureImportSettings.wrapS = ResourceTextureImportSettings::TextureWrapMode::REPEAT;
	textureData.textureImportSettings.wrapT = ResourceTextureImportSettings::TextureWrapMode::REPEAT;
	textureData.textureImportSettings.minFilter = ResourceTextureImportSettings::TextureFilterMode::NEAREST;
	textureData.textureImportSettings.magFilter = ResourceTextureImportSettings::TextureFilterMode::NEAREST;

	defaultTexture = (App->res->CreateResource(ResourceTypes::TextureResource, data, &textureData, REPLACE_ME_TEXTURE_UUID))->GetUuid();
}

void ModuleInternalResHandler::CreateLightIcon()
{
	lightIcon = App->res->ImportFile("Settings/Light_Icon.png")->GetUuid();
	App->res->SetAsUsed(lightIcon); // used in lights;
}

void ModuleInternalResHandler::CreateDefaultShaderProgram(const char* vShader, const char* fShader, ShaderProgramTypes type)
{
	ResourceData vertexData;
	ResourceShaderObjectData vertexShaderData;
	vertexData.name = "Default vertex object";
	vertexData.internal = true;
	vertexShaderData.shaderObjectType = ShaderObjectTypes::VertexType;
	vertexShaderData.SetSource(vShader, strlen(vShader));
	ResourceShaderObject* vObj = (ResourceShaderObject*)App->res->CreateResource(ResourceTypes::ShaderObjectResource, vertexData, &vertexShaderData);
	if (!vObj->Compile())
		vObj->isValid = false;

	ResourceData fragmentData;
	ResourceShaderObjectData fragmentShaderData;
	fragmentData.name = "Default fragment object";
	fragmentData.internal = true;
	fragmentShaderData.shaderObjectType = ShaderObjectTypes::FragmentType;
	fragmentShaderData.SetSource(fShader, strlen(fShader));
	ResourceShaderObject* fObj = (ResourceShaderObject*)App->res->CreateResource(ResourceTypes::ShaderObjectResource, fragmentData, &fragmentShaderData);
	if (!fObj->Compile())
		fObj->isValid = false;

	ResourceData shaderData;
	ResourceShaderProgramData programShaderData;
	shaderData.name = "Default shader program";
	shaderData.internal = true;
	programShaderData.shaderObjectsUuids.push_back(vObj->GetUuid());
	programShaderData.shaderObjectsUuids.push_back(fObj->GetUuid());
	programShaderData.shaderProgramType = type;
	ResourceShaderProgram* prog = nullptr;
	if (type == ShaderProgramTypes::Standard)
		prog = (ResourceShaderProgram*)App->res->CreateResource(ResourceTypes::ShaderProgramResource, shaderData, &programShaderData, DEFAULT_SHADER_PROGRAM_UUID);
	else if (type == ShaderProgramTypes::Particles)
		prog = (ResourceShaderProgram*)App->res->CreateResource(ResourceTypes::ShaderProgramResource, shaderData, &programShaderData, DEFAULT_SHADER_PROGRAM_PARTICLE_UUID);

	if (!prog->Link())
		prog->isValid = false;
	if (type == ShaderProgramTypes::Standard)
		defaultShaderProgram = prog->GetUuid();
	else if (type == ShaderProgramTypes::Particles)
		particleShaderProgram = prog->GetUuid();
}

void ModuleInternalResHandler::CreateDeferredShaderProgram()
{
	ResourceData vertexData;
	ResourceShaderObjectData vertexShaderData;
	vertexData.name = "Deferred vertex object";
	vertexData.internal = true;
	vertexShaderData.shaderObjectType = ShaderObjectTypes::VertexType;
	vertexShaderData.SetSource(vDEFERREDSHADING, strlen(vDEFERREDSHADING));
	ResourceShaderObject* vObj = (ResourceShaderObject*)App->res->CreateResource(ResourceTypes::ShaderObjectResource, vertexData, &vertexShaderData);
	if (!vObj->Compile())
		vObj->isValid = false;

	ResourceData fragmentData;
	ResourceShaderObjectData fragmentShaderData;
	fragmentData.name = "Deferred fragment object";
	fragmentData.internal = true;
	fragmentShaderData.shaderObjectType = ShaderObjectTypes::FragmentType;
	fragmentShaderData.SetSource(fDEFERREDSHADING, strlen(fDEFERREDSHADING));
	ResourceShaderObject* fObj = (ResourceShaderObject*)App->res->CreateResource(ResourceTypes::ShaderObjectResource, fragmentData, &fragmentShaderData);
	if (!fObj->Compile())
		fObj->isValid = false;

	ResourceData shaderData;
	ResourceShaderProgramData programShaderData;
	shaderData.name = "Deferred shader program";
	shaderData.internal = true;
	programShaderData.shaderObjectsUuids.push_back(vObj->GetUuid());
	programShaderData.shaderObjectsUuids.push_back(fObj->GetUuid());
	programShaderData.shaderProgramType = ShaderProgramTypes::Source;
	ResourceShaderProgram* pShader = (ResourceShaderProgram*)App->res->CreateResource(ResourceTypes::ShaderProgramResource, shaderData, &programShaderData, DEFERRED_SHADER_PROGRAM_UUID);
	if (!pShader->Link())
		pShader->isValid = false;
	deferredShaderProgram = pShader->GetUuid();
}

void ModuleInternalResHandler::CreateBillboardShaderProgram()
{
	ResourceData vertexData;
	ResourceShaderObjectData vertexShaderData;
	vertexData.name = "Billboard vertex object";
	vertexData.internal = true;
	vertexShaderData.shaderObjectType = ShaderObjectTypes::VertexType;
	vertexShaderData.SetSource(vShaderBillboard, strlen(vShaderBillboard));
	ResourceShaderObject* vObj = (ResourceShaderObject*)App->res->CreateResource(ResourceTypes::ShaderObjectResource, vertexData, &vertexShaderData);
	if (!vObj->Compile())
		vObj->isValid = false;

	ResourceData fragmentData;
	ResourceShaderObjectData fragmentShaderData;
	fragmentData.name = "Billboard fragment object";
	fragmentData.internal = true;
	fragmentShaderData.shaderObjectType = ShaderObjectTypes::FragmentType;
	fragmentShaderData.SetSource(fShaderBillboard, strlen(fShaderBillboard));
	ResourceShaderObject* fObj = (ResourceShaderObject*)App->res->CreateResource(ResourceTypes::ShaderObjectResource, fragmentData, &fragmentShaderData);
	if (!fObj->Compile())
		fObj->isValid = false;

	ResourceData shaderData;
	ResourceShaderProgramData programShaderData;
	shaderData.name = "Billboard";
	shaderData.internal = true;
	programShaderData.shaderObjectsUuids.push_back(vObj->GetUuid());
	programShaderData.shaderObjectsUuids.push_back(fObj->GetUuid());
	programShaderData.shaderProgramType = ShaderProgramTypes::Standard;
	ResourceShaderProgram* pShader = (ResourceShaderProgram*)App->res->CreateResource(ResourceTypes::ShaderProgramResource, shaderData, &programShaderData, BILLBOARD_SHADER_PROGRAM_UUID);
	if (!pShader->Link())
		pShader->isValid = false;
	billboardShaderProgram = pShader->GetUuid();
}

uint ModuleInternalResHandler::CreateCartoonShaderProgram() const
{
	ResourceData vertexData;
	ResourceShaderObjectData vertexShaderData;
	vertexData.name = "Cartoon Vertex";
	vertexData.internal = true;
	vertexShaderData.shaderObjectType = ShaderObjectTypes::VertexType;
	vertexShaderData.SetSource(vShaderTemplate, strlen(vShaderTemplate));
	ResourceShaderObject* vObj = (ResourceShaderObject*)App->res->CreateResource(ResourceTypes::ShaderObjectResource, vertexData, &vertexShaderData);
	if (!vObj->Compile())
		vObj->isValid = false;

	ResourceData geometryData;
	ResourceShaderObjectData geometryShaderData;
	geometryData.name = "Cartoon Geometry";
	geometryData.internal = true;
	geometryShaderData.shaderObjectType = ShaderObjectTypes::GeometryType;
	geometryShaderData.SetSource(CartoonGeometry, strlen(CartoonGeometry));
	ResourceShaderObject* gObj = (ResourceShaderObject*)App->res->CreateResource(ResourceTypes::ShaderObjectResource, geometryData, &geometryShaderData);
	if (!gObj->Compile())
		gObj->isValid = false;

	ResourceData fragmentData;
	ResourceShaderObjectData fragmentShaderData;
	fragmentData.name = "Cartoon Fragment";
	fragmentData.internal = true;
	fragmentShaderData.shaderObjectType = ShaderObjectTypes::FragmentType;
	fragmentShaderData.SetSource(CartoonFragment, strlen(CartoonFragment));
	ResourceShaderObject* fObj = (ResourceShaderObject*)App->res->CreateResource(ResourceTypes::ShaderObjectResource, fragmentData, &fragmentShaderData);
	if (!fObj->Compile())
		fObj->isValid = false;

	ResourceData shaderData;
	ResourceShaderProgramData programShaderData;
	shaderData.name = "Cartoon Shader";
	shaderData.internal = true;
	programShaderData.shaderObjectsUuids.push_back(vObj->GetUuid());
	programShaderData.shaderObjectsUuids.push_back(gObj->GetUuid());
	programShaderData.shaderObjectsUuids.push_back(fObj->GetUuid());
	programShaderData.shaderProgramType = ShaderProgramTypes::Standard;
	ResourceShaderProgram* prog = (ResourceShaderProgram*)App->res->CreateResource(ResourceTypes::ShaderProgramResource, shaderData, &programShaderData, CARTOON_SHADER_PROGRAM_UUID);
	if (!prog->Link())
		prog->isValid = false;

	return prog->GetUuid();
}

void ModuleInternalResHandler::CreateUIShaderProgram()
{
	ResourceData vertexData;
	ResourceShaderObjectData vertexShaderData;
	vertexData.name = "UI vertex object";
	vertexData.internal = true;
	vertexShaderData.shaderObjectType = ShaderObjectTypes::VertexType;
	vertexShaderData.SetSource(uivShader, strlen(uivShader));
	ResourceShaderObject* vObj = (ResourceShaderObject*)App->res->CreateResource(ResourceTypes::ShaderObjectResource, vertexData, &vertexShaderData);
	if (!vObj->Compile())
		vObj->isValid = false;
	UIVertexShaderObject = vObj->shaderObject;

	ResourceData fragmentData;
	ResourceShaderObjectData fragmentShaderData;
	fragmentData.name = "UI fragment object";
	fragmentData.internal = true;
	fragmentShaderData.shaderObjectType = ShaderObjectTypes::FragmentType;
	fragmentShaderData.SetSource(uifShader, strlen(uifShader));
	ResourceShaderObject* fObj = (ResourceShaderObject*)App->res->CreateResource(ResourceTypes::ShaderObjectResource, fragmentData, &fragmentShaderData);
	if (!fObj->Compile())
		fObj->isValid = false;
	UIFragmentShaderObject = fObj->shaderObject;

	ResourceData shaderData;
	ResourceShaderProgramData programShaderData;
	shaderData.name = "UI shader program";
	shaderData.internal = true;
	programShaderData.shaderObjectsUuids.push_back(vObj->GetUuid());
	programShaderData.shaderObjectsUuids.push_back(fObj->GetUuid());
	programShaderData.shaderProgramType = ShaderProgramTypes::UI;
	ResourceShaderProgram* pShader = (ResourceShaderProgram*)App->res->CreateResource(ResourceTypes::ShaderProgramResource, shaderData, &programShaderData, DEFAULT_SHADER_PROGRAM_UI_UUID);
	if (!pShader->Link())
		pShader->isValid = false;
	UIShaderProgram = pShader->shaderProgram;
}

void ModuleInternalResHandler::CreateDefaultMaterial()
{
	ResourceData data;
	ResourceMaterialData materialData;
	data.name = "Default material";
	data.internal = true;
	materialData.shaderUuid = DEFAULT_SHADER_PROGRAM_UUID;
	((ResourceShaderProgram*)App->res->GetResource(materialData.shaderUuid))->GetUniforms(materialData.uniforms);
	for (uint i = 0; i < materialData.uniforms.size(); ++i)
	{
		Uniform& uniform = materialData.uniforms[i];
		switch (uniform.common.type)
		{
		case Uniforms_Values::Sampler2U_value:
		{
			if (strcmp(uniform.common.name, "diffuse") == 0)
			{
				uniform.sampler2DU.value.uuid = defaultTexture;
				uniform.sampler2DU.value.id = ((ResourceTexture*)App->res->GetResource(defaultTexture))->GetId();
			}
		}
		break;
		}
	}

	defaultMaterial = App->res->CreateResource(ResourceTypes::MaterialResource, data, &materialData, DEFAULT_MATERIAL_UUID)->GetUuid();
}