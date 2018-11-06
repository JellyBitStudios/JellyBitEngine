#include "GameMode.h"

#ifndef __SCENE_IMPORTER_H__
#define __SCENE_IMPORTER_H__

#include "Importer.h"

#include "MathGeoLib/include/Math/float3.h"
#include "MathGeoLib/include/Math/Quat.h"

#include "Assimp/include/cimport.h"
#include "Assimp/include/scene.h"
#include "Assimp/include/postprocess.h"
#include "Assimp/include/cfileio.h"
#include "Assimp/include/version.h"

#pragma comment (lib, "Assimp/libx86/assimp-vc140-mt.lib")

#include <vector>

struct ResourceMesh;

struct ModelImportSettings
{
	math::float3 scale = math::float3::one;
	bool useFileScale = true;

	// Post Process
	int configuration = aiProcessPreset_TargetRealtime_MaxQuality;
	bool calcTangentSpace = true;
	bool genNormals = false;
	bool genSmoothNormals = true;
	bool joinIdenticalVertices = true;
	bool triangulate = true;
	bool genUVCoords = true;
	bool sortByPType = true;
	bool improveCacheLocality = true;
	bool limitBoneWeights = true;
	bool removeRedundantMaterials = true;
	bool splitLargeMeshes = true;
	bool findDegenerates = true;
	bool findInvalidData = true;
	bool findInstances = true;
	bool validateDataStructure = true;
	bool optimizeMeshes = true;
};

struct aiScene;
struct aiNode;

class GameObject;
class ResourceMesh;

class SceneImporter : public Importer
{
public:

	SceneImporter();
	~SceneImporter();

	bool Import(const char* importFileName, const char* importPath, std::string& outputFileName);
	bool Import(const void* buffer, uint size, std::string& outputFileName);
	void RecursivelyImportNodes(const aiScene* scene, const aiNode* node, const GameObject* parentGO, GameObject* transformationGO, std::string& outputFileName);

	void GenerateMeta(Resource* resource);

	bool Load(const char* exportedFileName, ResourceMesh* outputMesh);
	bool Load(const void* buffer, uint size, ResourceMesh* outputMesh);

	uint GetAssimpMajorVersion() const;
	uint GetAssimpMinorVersion() const;
	uint GetAssimpRevisionVersion() const;

public:

	// Default import values
	ModelImportSettings defaultImportSettings;
};

#endif