#ifndef __SCENE_IMPORTER_H__
#define __SCENE_IMPORTER_H__

#include "Globals.h"

#include <string>
#include <vector>
#include <map>
#include <unordered_map>

struct aiScene;
struct aiNode;
struct aiBone;

class GameObject;

struct Vertex;
struct ResourceData;
struct ResourceMeshData;
struct ResourceMeshImportSettings;

class SceneImporter
{
public:

	SceneImporter();
	~SceneImporter();

	bool Import(const char* file, std::vector<std::string>& mesh_files, std::vector<std::string>& bone_files, std::vector<std::string>& anim_files, const ResourceMeshImportSettings& importSettings,
		std::vector<uint>& forced_meshes_uuids = std::vector<uint>(), std::vector<uint>& forced_bones_uuids = std::vector<uint>(), std::vector<uint>& forced_anims_uuids = std::vector<uint>()) const;
	
	bool Load(const char* exportedFile, ResourceData& outputData, ResourceMeshData& outputMeshData) const;

	// ----------------------------------------------------------------------------------------------------

	void GenerateVBO(uint& VBO, Vertex* vertices, uint verticesSize) const;
	void GenerateIBO(uint& IBO, uint* indices, uint indicesSize) const;
	void GenerateVAO(uint& VAO, uint& VBO, uint attrFlag) const;

	void DeleteBufferObject(uint& name) const;
	void DeleteVertexArrayObject(uint& name) const;

	// ----------------------------------------------------------------------------------------------------

	uint GetAssimpMajorVersion() const;
	uint GetAssimpMinorVersion() const;
	uint GetAssimpRevisionVersion() const;

private:

	bool Import(const void* buffer, uint size, const char* prefabName, std::vector<std::string>& mesh_files, std::vector<std::string>& bone_files, std::vector<std::string>& anim_files, const ResourceMeshImportSettings& importSettings,
		std::vector<uint>& forced_meshes_uuids = std::vector<uint>(), std::vector<uint>& forced_bones_uuids = std::vector<uint>(), std::vector<uint>& forced_anims_uuids = std::vector<uint>()) const;
	
	void RecursivelyImportNodes(const aiScene* scene, const aiNode* node, const GameObject* parent, const GameObject* transformation, 
		GameObject*& rootBone, std::unordered_map<std::string, aiBone*>& bonesByName,
		std::vector<std::string>& mesh_files, std::vector<uint>& forcedUuids = std::vector<uint>()) const;

	void ImportBones(GameObject* gameObject, 
		std::unordered_map<std::string, aiBone*>& bonesByName,
		std::vector<std::string>& bone_files, std::vector<uint>& forcedUuids = std::vector<uint>()) const;

	void ImportAnimations(const aiScene* scene, GameObject* rootBone,  std::vector<std::string>& anim_files, const char* anim_name, std::vector<uint>& forcedUuids = std::vector<uint>()) const;

	bool Load(const void* buffer, uint size, ResourceData& outputData, ResourceMeshData& outputMeshData) const;

private:

	/*Mutable stuff uwu*/
	mutable GameObject* imported_root_go = nullptr;
	mutable std::map<std::string, uint> imported_bones;
};

#endif