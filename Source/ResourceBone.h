#ifndef __RESOURCE_BONE_H__
#define __RESOURCE_BONE_H__

#include "Resource.h"

#include "MathGeoLib\include\Math\float4x4.h"

#define MAX_WEIGHTS 4

struct ResourceBoneData
{
	std::string name;
	math::float4x4 offsetMatrix = math::float4x4::identity; // matrix that transform from mesh space to bone space in bind pose
	
	float weights[MAX_WEIGHTS] = { 0.0f, 0.0f, 0.0f, 0.0f };
	uint indices[MAX_WEIGHTS] = { 0,0,0,0 };
};

class ResourceBone : public Resource
{
public:

	ResourceBone(ResourceTypes type, uint uuid, ResourceData data, ResourceBoneData boneData);
	~ResourceBone();

	void OnPanelAssets();

	// ----------------------------------------------------------------------------------------------------

	static bool ExportFile(const ResourceData& data, const ResourceBoneData& boneData, std::string& outputFile, bool overwrite = false);
	static uint SaveFile(const ResourceData& data, const ResourceBoneData& boneData, std::string& outputFile, bool overwrite = false);
	static bool LoadFile(const char* file, ResourceBoneData& outputBoneData);

	// ----------------------------------------------------------------------------------------------------

	inline ResourceBoneData& GetSpecificData() { return boneData; }

private:

	bool LoadInMemory();
	bool UnloadFromMemory();

public:

	ResourceBoneData boneData;
};

#endif