#ifndef __RESOURCE_MESH_H__
#define __RESOURCE_MESH_H__

#include "Resource.h"

#include <vector>
#include <unordered_map>

#define MAX_BONES 4

struct ResourceMeshImportSettings
{
	enum PostProcessConfigurationFlags
	{
		TARGET_REALTIME_FAST,
		TARGET_REALTIME_QUALITY,
		TARGET_REALTIME_MAX_QUALITY,
		CUSTOM
	};

	enum CustomConfigurationFlags
	{
		CALC_TANGENT_SPACE = 1 << 0,
		GEN_NORMALS = 1 << 1,
		GEN_SMOOTH_NORMALS = 1 << 2,
		JOIN_IDENTICAL_VERTICES = 1 << 3,
		TRIANGULATE = 1 << 4,
		GEN_UV_COORDS = 1 << 5,
		SORT_BY_P_TYPE = 1 << 6,
		IMPROVE_CACHE_LOCALITY = 1 << 7,
		LIMIT_BONE_WEIGHTS = 1 << 8,
		REMOVE_REDUNDANT_MATERIALS = 1 << 9,
		SPLIT_LARGE_MESHES = 1 << 10,
		FIND_DEGENERATES = 1 << 11,
		FIND_INVALID_DATA = 1 << 12,
		FIND_INSTANCES = 1 << 13,
		VALIDATE_DATA_STRUCTURE = 1 << 14,
		OPTIMIZE_MESHES = 1 << 15
	};

	enum AttrConfiguration
	{
		ATTR_POSITION = 1 << 0,
		ATTR_NORMAL = 1 << 1,
		ATTR_COLOR = 1 << 2,
		ATTR_TEXCOORD = 1 << 3,
		ATTR_TANGENT = 1 << 4,
		ATTR_BITANGENT = 1 << 5,
	};

	PostProcessConfigurationFlags postProcessConfigurationFlags = PostProcessConfigurationFlags::TARGET_REALTIME_MAX_QUALITY;
	uint customConfigurationFlags = 0;
	uint attributes = ATTR_POSITION | ATTR_NORMAL | ATTR_COLOR | ATTR_TEXCOORD | ATTR_TANGENT | ATTR_BITANGENT;

	float scale = 1.0f;
	bool adjacency = false;

	char modelPath[DEFAULT_BUF_SIZE];
};

struct Vertex
{
	float position[3];
	float normal[3];
	float tangent[3];
	float bitangent[3];
	uchar color[4];
	float texCoord[2];

	float boneWeight[MAX_BONES] = { 0.0f, 0.0f, 0.0f, 0.0f };
	int boneId[MAX_BONES] = { 0,0,0,0 };
};

struct BoneInfluence
{
	uint bonesWeightsSize = 0;
	float* boneWeights = nullptr;
	uint* boneIds = nullptr;
	char boneName[DEFAULT_BUF_SIZE];
};

struct ResourceMeshData
{
	Vertex* vertices = nullptr;
	uint verticesSize = 0;

	uint* indices = nullptr;
	uint indicesSize = 0;

	uint* adjacentIndices = nullptr;

	BoneInfluence* boneInfluences = nullptr;
	uint boneInfluencesSize = 0;

	ResourceMeshImportSettings meshImportSettings;
};

class ResourceMesh : public Resource
{
public:

	ResourceMesh(ResourceTypes type, uint uuid, ResourceData data, ResourceMeshData meshData);
	~ResourceMesh();

	void OnPanelAssets();

	// ----------------------------------------------------------------------------------------------------

	static bool ImportFile(const char* file, ResourceMeshImportSettings& meshImportSettings,
		std::vector<std::string>& mesh_files, std::vector<std::string>& bone_files, std::vector<std::string>& animation_files);
	static uint CreateMeta(const char* file, ResourceMeshImportSettings& meshImportSettings,
		std::vector<uint>& meshesUuids, std::vector<uint>& bonesUuids, std::vector<uint>& animationUuids, std::string& outputMetaFile);
	static bool ReadMeta(const char* metaFile, int64_t& lastModTime, ResourceMeshImportSettings& meshImportSettings,
		std::vector<uint>& meshesUuids, std::vector<uint>& bonesUuids, std::vector<uint>& animationUuids);
	static bool ReadMeshesUuidsFromBuffer(const char* buffer, 
		std::vector<uint>& meshesUuids, std::vector<uint>& bonesUuids, std::vector<uint>& animationUuids);
	static uint SetMeshImportSettingsToMeta(const char* metaFile, const ResourceMeshImportSettings& meshImportSettings);

	// ----------------------------------------------------------------------------------------------------

	inline ResourceMeshData& GetSpecificData() { return meshData; }
	void GetVerticesReference(Vertex*& vertices) const;
	void GetTris(float* verticesPosition) const;
	void GetIndicesReference(uint*& indices) const;
	void GetIndices(uint* indices) const;
	uint GetVerticesCount() const;
	uint GetIndicesCount() const;
	bool UseAdjacency() const;

	bool AddBones(const std::unordered_map<std::string, uint>& bones);
	bool AddBone(uint vertexId, float boneWeight, uint boneId);
	static void CalculateAdjacentIndices(uint* indices, uint indicesSize, uint*& adjacentIndices);

	uint GetVBO() const;
	uint GetIBO() const;
	uint GetVAO() const;

private:

	void GenerateVAO();
	void DeleteVAO();

	bool LoadInMemory();
	bool UnloadFromMemory();

private:

	uint VBO = 0;
	uint IBO = 0;
	uint VAO = 0;

	ResourceMeshData meshData;
};

#endif