#ifndef __RESOURCE_ANIMATOR_H__
#define __RESOURCE_ANIMATOR_H__

#include "Resource.h"
#include <vector>

struct ResourceAnimatorData
{
	std::string name;
	uint avatar_uuid;
	std::vector<uint> meshes_uuids;
	std::vector<uint> animations_uuids;
};

class ResourceAnimator : public Resource
{

public:

	ResourceAnimator(ResourceTypes type, uint uuid, ResourceData data, ResourceAnimatorData animationData);
	~ResourceAnimator();

	bool LoadInMemory();
	bool UnloadFromMemory();

	void OnPanelAssets();

	static bool ImportFile(const char* file, std::string& name, std::string& outputFile);
	static bool ExportFile(ResourceData& data, ResourceAnimatorData& prefabData, std::string& outputFile, bool overwrite = false);
	static uint CreateMeta(const char* file, uint prefab_uuid, std::string& name, std::string& outputMetaFile);
	static bool ReadMeta(const char* metaFile, int64_t& lastModTime, uint& prefab_uuid, std::string& name);
	static bool LoadFile(const char* file, ResourceAnimatorData& prefab_data_output);

	bool GenerateLibraryFiles() const;
	static uint SetNameToMeta(const char* metaFile, const std::string& name);

public:
	ResourceAnimatorData animator_data;
};

#endif // __RESOURCE_ANIMATION_H__