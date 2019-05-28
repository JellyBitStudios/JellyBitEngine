#ifndef __RESOURCE_MATERIAL_H__
#define __RESOURCE_MATERIAL_H__

#include "Resource.h"

#include "Uniforms.h"

#include "MathGeoLib\include\Math\float4.h"
#include "MathGeoLib\include\Math\float4x4.h"

#include <vector>

struct ResourceMaterialData
{
	uint shaderUuid = 0;
	uint shader = 0;
	std::vector<Uniform> uniforms;

	//math::float4x4 matrix = math::float4x4::identity;
};

class ResourceMaterial : public Resource
{
public:

	ResourceMaterial(ResourceTypes type, uint uuid, ResourceData data, ResourceMaterialData materialData, bool internalRes);
	~ResourceMaterial();

	void OnPanelAssets();

	// ----------------------------------------------------------------------------------------------------

	static bool ImportFile(const char* file, std::string& name, std::string& outputFile);
	static bool ExportFile(ResourceData& data, ResourceMaterialData& materialData, std::string& outputFile, bool overwrite = false);
	static uint SaveFile(ResourceData& data, ResourceMaterialData& materialData, std::string& outputFile, bool overwrite = false);
	static bool LoadFile(const char* file, ResourceMaterialData& outputMaterialData);
	
	static uint CreateMeta(const char* file, uint materialUuid, std::string& name, std::string& outputMetaFile);
	static bool ReadMeta(const char* metaFile, int64_t& lastModTime, uint& materialUuid, std::string& name);
	static uint SetNameToMeta(const char* metaFile, const std::string& name);

	bool GenerateLibraryFiles() const;

	// ----------------------------------------------------------------------------------------------------
	
	inline ResourceMaterialData& GetSpecificData() { return materialData; }

	void SetResourceShader(uint shaderUuid);
	bool UpdateResourceShader();

	uint GetShaderUuid() const;

	void SetResourceTexture(uint textureUuid, uint& textureUuidUniform, uint& textureIdUniform);

	std::vector<Uniform>& GetUniforms();
	void FillUniforms();
	void ClearUniforms();
	void GetIgnoreUniforms(std::vector<const char*>& ignoreUniforms);

private:

	void InitResources();
	void DeinitResources();

	void SetUniformsAsUsed();
	void SetUniformsAsUnused() const;

	bool UpdateUniformsLocations();
	bool UpdateUniforms();

	void EditTextureMatrix(uint textureUuid);

	bool LoadInMemory();
	bool UnloadFromMemory();

public:

	ResourceMaterialData materialData;
};

#endif