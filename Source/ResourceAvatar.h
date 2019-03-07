#ifndef __RESOURCE_AVATAR_H__
#define __RESOURCE_AVATAR_H__

#include "Resource.h"

#include <vector>

struct ResourceAvatarData
{
	uint hipsUuid = 0; // uuid of the root game object of the skeleton
	std::vector<uint> meshesUuids; // uuid of the resources of the affected meshes 
};

class ResourceAvatar : public Resource
{
public:

	ResourceAvatar(ResourceTypes type, uint uuid, ResourceData data, ResourceAvatarData avatarData);
	~ResourceAvatar();

	void OnPanelAssets();

	// ----------------------------------------------------------------------------------------------------

	static bool ImportFile(const char* file, std::string& name, std::string& outputFile);
	static uint CreateMeta(const char* file, uint avatarUuid, std::string& name, std::string& outputMetaFile);
	static bool ReadMeta(const char* metaFile, int64_t& lastModTime, uint& avatarUuid, std::string& name);
	
	bool GenerateLibraryFiles() const;

	// ----------------------------------------------------------------------------------------------------

	inline ResourceAvatarData& GetSpecificData() { return avatarData; }

	// ----------------------------------------------------------------------------------------------------

	void StepAnimation(uint animationUuid, float time, float blendTime) const;

private:

	bool LoadInMemory();
	bool UnloadFromMemory();

private:

	ResourceAvatarData avatarData;
};

#endif