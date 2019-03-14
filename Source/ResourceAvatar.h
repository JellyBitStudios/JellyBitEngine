#ifndef __RESOURCE_AVATAR_H__
#define __RESOURCE_AVATAR_H__

#include "Resource.h"

#include <vector>
#include <unordered_map>

class GameObject;

struct ResourceAvatarData
{
	uint hipsUuid = 0; // uuid of the root game object that forms the skeleton
	uint rootUuid = 0;  // uuid of the root game object that will be affected by the skeleton
};

class ResourceAvatar : public Resource
{
public:

	ResourceAvatar(ResourceTypes type, uint uuid, ResourceData data, ResourceAvatarData avatarData);
	~ResourceAvatar();

	void OnPanelAssets();

	// ----------------------------------------------------------------------------------------------------

	static bool ImportFile(const char* file, std::string& name, std::string& outputFile);
	static bool ExportFile(const ResourceData& data, const ResourceAvatarData& avatarData, std::string& outputFile, bool overwrite = false);
	static uint SaveFile(const ResourceData& data, const ResourceAvatarData& avatarData, std::string& outputFile, bool overwrite = false);
	static bool LoadFile(const char* file, ResourceAvatarData& outputAvatarData);

	static uint CreateMeta(const char* file, uint avatarUuid, const std::string& name, std::string& outputMetaFile);
	static bool ReadMeta(const char* metaFile, int64_t& lastModTime, uint& avatarUuid, std::string& name);
	static uint SetNameToMeta(const char* metaFile, const std::string& name);

	bool GenerateLibraryFiles() const;

	// ----------------------------------------------------------------------------------------------------

	inline ResourceAvatarData& GetSpecificData() { return avatarData; }
	bool SetHipsUuid(uint hipsUuid);
	uint GetHipsUuid() const;
	bool SetRootUuid(uint rootUuid);
	uint GetRootUuid() const;
	std::vector<uint> GetBonesUuids() const;

	// ----------------------------------------------------------------------------------------------------

	void CreateSkeleton(GameObject* gameObject);
	void AddBones(GameObject* gameObject) const;

	void StepBones(uint animationUuid, float time, float blend = 1.0f);

private:

	void ClearSkeleton();
	void ClearBones();
	void ClearSkeletonAndBones();

	bool LoadInMemory();
	bool UnloadFromMemory();

private:

	std::unordered_map<const char*, uint> bones; // bone name, bone game object uuid

	ResourceAvatarData avatarData;
};

#endif