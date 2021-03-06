#include "SceneImporter.h"

#include "Application.h"
#include "Globals.h"
#include "ModuleFileSystem.h"
#include "ModuleResourceManager.h"
#include "ModuleGOs.h"
#include "AnimationImporter.h"

#include "ResourceMesh.h"
#include "ResourceBone.h"
#include "ResourceAvatar.h"
#include "ResourceAnimation.h"
#include "ResourcePrefab.h"

#include "GameObject.h"
#include "ComponentMesh.h"
#include "ComponentMaterial.h"
#include "ComponentTransform.h"
#include "ComponentBone.h"

#include "Assimp\include\cimport.h"
#include "Assimp\include\scene.h"
#include "Assimp\include\postprocess.h"
#include "Assimp\include\cfileio.h"
#include "Assimp\include\version.h"

#include "glew\include\GL\glew.h"

#include "MathGeoLib\include\Math\float3.h"
#include "MathGeoLib\include\Math\Quat.h"

#include <assert.h>

#ifdef _DEBUG
#pragma comment (lib, "Assimp\\libx86\\assimp-vc140-mt.lib")
#else
#pragma comment (lib, "Assimp\\libx86\\rls\\assimp-vc140-mt.lib")
#endif

#define T_PRE_ROT "PreRotation"
#define T_ROT "Rotation"
#define T_POST_ROT "PostRotation"
#define T_TRANSLATION "Translation"
#define T_SCALING "Scaling"

void myCallback(const char* msg, char* userData)
{
	CONSOLE_LOG(LogTypes::Normal, "%s", msg);
}

SceneImporter::SceneImporter()
{
	struct aiLogStream stream;
	stream.callback = myCallback;
	aiAttachLogStream(&stream);
}

SceneImporter::~SceneImporter()
{
	aiDetachAllLogStreams();
}

bool SceneImporter::Import(const char* file,
	std::vector<std::string>& mesh_files, std::vector<std::string>& bone_files, std::vector<std::string>& anim_files,
	const ResourceMeshImportSettings& importSettings, 
	std::vector<uint>& forced_mesh_uuids, std::vector<uint>& forced_bone_uuids, std::vector<uint>& forced_anim_uuids) const
{
	assert(file != nullptr);

	bool ret = false;

	std::string fileName;
	App->fs->GetFileName(file, fileName);

	char* buffer;
	uint size = App->fs->Load(file, &buffer);
	if (size > 0)
	{
		CONSOLE_LOG(LogTypes::Normal, "SCENE IMPORTER: Successfully loaded Model '%s'", fileName.data());
		ret = Import(buffer, size, fileName.data(), mesh_files, bone_files, anim_files, importSettings, forced_mesh_uuids, forced_bone_uuids, forced_anim_uuids);
		RELEASE_ARRAY(buffer);
	}
	else
		CONSOLE_LOG(LogTypes::Error, "SCENE IMPORTER: Could not load Model '%s'", fileName.data());

	return ret;
}

bool SceneImporter::Import(const void* buffer, uint size, const char* prefabName, 
	std::vector<std::string>& mesh_files, std::vector<std::string>& bone_files, std::vector<std::string>& anim_files,
	const ResourceMeshImportSettings& importSettings, 
	std::vector<uint>& forced_meshes_uuids, std::vector<uint>& forced_bones_uuids, std::vector<uint>& forced_anim_uuids) const
{
	assert(buffer != nullptr && size > 0);

	bool ret = false;

	uint postProcessConfigurationFlags = 0;
	switch (importSettings.postProcessConfigurationFlags)
	{
	case ResourceMeshImportSettings::PostProcessConfigurationFlags::TARGET_REALTIME_FAST:
		postProcessConfigurationFlags |= aiProcessPreset_TargetRealtime_Fast;
		break;
	case ResourceMeshImportSettings::PostProcessConfigurationFlags::TARGET_REALTIME_QUALITY:
		postProcessConfigurationFlags |= aiProcessPreset_TargetRealtime_Quality;
		break;
	case ResourceMeshImportSettings::PostProcessConfigurationFlags::TARGET_REALTIME_MAX_QUALITY:
		postProcessConfigurationFlags |= aiProcessPreset_TargetRealtime_MaxQuality;
		break;
	case ResourceMeshImportSettings::PostProcessConfigurationFlags::CUSTOM:
		if (importSettings.customConfigurationFlags & ResourceMeshImportSettings::CustomConfigurationFlags::CALC_TANGENT_SPACE)
			postProcessConfigurationFlags |= aiPostProcessSteps::aiProcess_CalcTangentSpace;
		if (importSettings.customConfigurationFlags & ResourceMeshImportSettings::CustomConfigurationFlags::GEN_NORMALS)
			postProcessConfigurationFlags |= aiPostProcessSteps::aiProcess_GenNormals;
		else if (importSettings.customConfigurationFlags & ResourceMeshImportSettings::CustomConfigurationFlags::GEN_SMOOTH_NORMALS)
			postProcessConfigurationFlags |= aiPostProcessSteps::aiProcess_GenSmoothNormals;
		if (importSettings.customConfigurationFlags & ResourceMeshImportSettings::CustomConfigurationFlags::JOIN_IDENTICAL_VERTICES)
			postProcessConfigurationFlags |= aiPostProcessSteps::aiProcess_JoinIdenticalVertices;
		if (importSettings.customConfigurationFlags & ResourceMeshImportSettings::CustomConfigurationFlags::TRIANGULATE)
			postProcessConfigurationFlags |= aiPostProcessSteps::aiProcess_Triangulate;
		if (importSettings.customConfigurationFlags & ResourceMeshImportSettings::CustomConfigurationFlags::GEN_UV_COORDS)
			postProcessConfigurationFlags |= aiPostProcessSteps::aiProcess_GenUVCoords;
		if (importSettings.customConfigurationFlags & ResourceMeshImportSettings::CustomConfigurationFlags::SORT_BY_P_TYPE)
			postProcessConfigurationFlags |= aiPostProcessSteps::aiProcess_SortByPType;
		if (importSettings.customConfigurationFlags & ResourceMeshImportSettings::CustomConfigurationFlags::IMPROVE_CACHE_LOCALITY)
			postProcessConfigurationFlags |= aiPostProcessSteps::aiProcess_ImproveCacheLocality;
		if (importSettings.customConfigurationFlags & ResourceMeshImportSettings::CustomConfigurationFlags::LIMIT_BONE_WEIGHTS)
			postProcessConfigurationFlags |= aiPostProcessSteps::aiProcess_LimitBoneWeights;
		if (importSettings.customConfigurationFlags & ResourceMeshImportSettings::CustomConfigurationFlags::REMOVE_REDUNDANT_MATERIALS)
			postProcessConfigurationFlags |= aiPostProcessSteps::aiProcess_RemoveRedundantMaterials;
		if (importSettings.customConfigurationFlags & ResourceMeshImportSettings::CustomConfigurationFlags::SPLIT_LARGE_MESHES)
			postProcessConfigurationFlags |= aiPostProcessSteps::aiProcess_SplitLargeMeshes;
		if (importSettings.customConfigurationFlags & ResourceMeshImportSettings::CustomConfigurationFlags::FIND_DEGENERATES)
			postProcessConfigurationFlags |= aiPostProcessSteps::aiProcess_FindDegenerates;
		if (importSettings.customConfigurationFlags & ResourceMeshImportSettings::CustomConfigurationFlags::FIND_INVALID_DATA)
			postProcessConfigurationFlags |= aiPostProcessSteps::aiProcess_FindInvalidData;
		if (importSettings.customConfigurationFlags & ResourceMeshImportSettings::CustomConfigurationFlags::FIND_INSTANCES)
			postProcessConfigurationFlags |= aiPostProcessSteps::aiProcess_FindInstances;
		if (importSettings.customConfigurationFlags & ResourceMeshImportSettings::CustomConfigurationFlags::VALIDATE_DATA_STRUCTURE)
			postProcessConfigurationFlags |= aiPostProcessSteps::aiProcess_ValidateDataStructure;
		if (importSettings.customConfigurationFlags & ResourceMeshImportSettings::CustomConfigurationFlags::OPTIMIZE_MESHES)
			postProcessConfigurationFlags |= aiPostProcessSteps::aiProcess_OptimizeMeshes;
		break;
	}

	const aiScene* scene = aiImportFileFromMemory((const char*)buffer, size, postProcessConfigurationFlags, nullptr);

	if (scene != nullptr)
	{
		ret = true;

		// 1. Import the scene
		const aiNode* rootNode = scene->mRootNode;

		GameObject* dummy = new GameObject("Dummy", nullptr);
		GameObject* rootGameObject = new GameObject(rootNode->mName.data, dummy); // Root game object will never be a transformation

		/// Import meshes
		std::vector<uint> dummyForcedMeshesUuids = forced_meshes_uuids;
		GameObject* rootBone = nullptr;
		std::unordered_map<std::string, aiBone*> bonesByName;

		RecursivelyImportNodes(scene, rootNode, rootGameObject, nullptr, 
			rootBone, bonesByName,
			mesh_files, dummyForcedMeshesUuids);
		//rootGameObject->transform->scale *= importSettings.scale; // TODO FIX SCALE

		if (rootBone != nullptr || mode_export_animation)
		{
			/// Import bones
			if (!mode_export_animation) {
				std::vector<uint> dummyForcedBonesUuids = forced_bones_uuids;

				ImportBones(rootGameObject,
					bonesByName,
					bone_files, dummyForcedBonesUuids);
			}

			/// Import animations
			std::vector<uint> dummyForcedAnimationsUuids = forced_anim_uuids;
			ImportAnimations(scene, rootBone, anim_files, prefabName, dummyForcedAnimationsUuids);
		}

		//chill
		mode_export_animation = false;

		// Create Prefab
		GameObject* prefab_go = rootGameObject;
		ResourceData prefabGenericData;
		PrefabData prefabSpecificData;

		prefabGenericData.file = DIR_ASSETS_PREFAB + std::string("/") + prefab_go->GetName() + EXTENSION_PREFAB;
		prefabGenericData.exportedFile = "";
		prefabGenericData.name = prefabName;
		prefabSpecificData.root = prefab_go;

		App->res->ExportFile(ResourceTypes::PrefabResource, prefabGenericData, &prefabSpecificData, std::string());

		// Create Avatar
		/*
		if (rootBone != nullptr)
		{
			ResourceData avatarGenericData;
			ResourceAvatarData avatarSpecificData;

			avatarGenericData.name = prefabName;
			avatarSpecificData.hipsUuid = rootBone->GetUUID();

			std::string outputFile;
			App->res->ExportFile(ResourceTypes::AvatarResource, avatarGenericData, &avatarSpecificData, outputFile);
		}
		*/

		// ----------
	
		aiReleaseImport(scene);

		dummy->RecursiveForceAllResources(0);
		App->GOs->Kill(dummy);
	}

	return ret;
}

void SceneImporter::RecursivelyImportNodes(const aiScene* scene, const aiNode* node, const GameObject* parent, const GameObject* transformation, 
	GameObject*& rootBone, std::unordered_map<std::string, aiBone*>& bonesByName,
	std::vector<std::string>& mesh_files, std::vector<uint>& forcedUuids) const
{
	std::string name = node->mName.data;

	// A game object is a transformation if its name contains a transformation
	bool isTransformation =
		(name.find(T_PRE_ROT) != std::string::npos
			|| name.find(T_ROT) != std::string::npos
			|| name.find(T_POST_ROT) != std::string::npos
			|| name.find(T_TRANSLATION) != std::string::npos
			|| name.find(T_SCALING) != std::string::npos);

	GameObject* gameObject = nullptr;

	// If the node is the first node, then the game object is the parent
	if (node == scene->mRootNode)
		gameObject = (GameObject*)parent;
	// If the previous game object was a transformation, keep the transformation
	else if (transformation != nullptr)
		gameObject = (GameObject*)transformation;
	// If the previous game object wasn't a transformation, create a new game object
	else
		gameObject = new GameObject(name.data(), (GameObject*)parent);

	// If the current game object is not a transformation, update its name (just in case the previous one was)
	if (!isTransformation)
		gameObject->SetName(name.data());

	// Transform
	aiVector3D aPosition;
	aiVector3D aScale;
	aiQuaternion aRotation;
	node->mTransformation.Decompose(aScale, aRotation, aPosition);

	math::float3 newPosition = { aPosition.x, aPosition.y, aPosition.z };
	math::Quat newRotation = { aRotation.x, aRotation.y, aRotation.z, aRotation.w };
	math::float3 newScale = { aScale.x, aScale.y, aScale.z };

	if (transformation != nullptr)
	{
		math::float3 pos = transformation->transform->GetPosition();
		gameObject->transform->SetPosition(pos + newPosition);

		math::Quat rot = transformation->transform->GetRotation();
		gameObject->transform->SetRotation(rot * newRotation);

		math::float3 scale = transformation->transform->GetScale();
		gameObject->transform->SetScale({scale.x * newScale.x, scale.y * newScale.y,scale.z * newScale.z });
	}
	else
	{
		gameObject->transform->SetPosition(newPosition);
		gameObject->transform->SetRotation(newRotation);
		gameObject->transform->SetScale(newScale);
	}

	// Meshes
	if (!isTransformation && node->mNumMeshes > 0) // We assume that transformations don't contain meshes
	{
		aiMesh* nodeMesh = scene->mMeshes[node->mMeshes[0]];

		bool broken = false;
		for (uint i = 0; i < nodeMesh->mNumFaces; ++i)
		{
			broken = nodeMesh->mFaces[i].mNumIndices != 3;

			if (broken)
				break;
		}

		if (!broken)
		{
			// Create the Mesh Component
			if (nodeMesh->mName.length == 0 && nodeMesh->HasBones()) { // .dae without mesh but with animation
				mode_export_animation = true;
			}

			if (!mode_export_animation) {
				gameObject->AddComponent(ComponentTypes::MeshComponent);

				if (forcedUuids.size() > 0)
				{
					gameObject->cmp_mesh->res = forcedUuids.front();
					forcedUuids.erase(forcedUuids.begin());
				}
				else
					gameObject->cmp_mesh->res = App->GenerateRandomNumber();
			}

			float* vertices = nullptr;
			uint verticesSize = 0;

			float* normals = nullptr;
			uint normalsSize = 0;

			float* tangents = nullptr;
			uint tangentsSize = 0;

			float* bitangents = nullptr;
			uint bitangentsSize = 0;

			uchar* colors = nullptr;
			uint colorsSize = 0;

			float* texCoords = nullptr;
			uint texCoordsSize = 0;

			BoneInfluence* boneInfluences = nullptr;
			uint boneInfluencesSize = 0;

			uint* indices = nullptr;
			uint indicesSize = 0;

			// Unique vertices
			verticesSize = nodeMesh->mNumVertices;
			vertices = new float[verticesSize * 3];
			memcpy(vertices, nodeMesh->mVertices, sizeof(float) * verticesSize * 3);

			// Indices
			if (nodeMesh->HasFaces() && !mode_export_animation)
			{
				uint facesSize = nodeMesh->mNumFaces;
				indicesSize = facesSize * 3;
				indices = new uint[indicesSize];

				for (uint i = 0; i < facesSize; ++i)
				{
					if (nodeMesh->mFaces[i].mNumIndices != 3)
					{
						CONSOLE_LOG(LogTypes::Warning, "WARNING, geometry face with != 3 indices!");
					}
					else
						memcpy(&indices[i * 3], nodeMesh->mFaces[i].mIndices, 3 * sizeof(uint));
				}
			}

			// Normals
			if (nodeMesh->HasNormals() && !mode_export_animation)
			{
				normalsSize = verticesSize;
				normals = new float[normalsSize * 3];
				memcpy(normals, nodeMesh->mNormals, sizeof(float) * normalsSize * 3);
			}

			// Tangents and Bitangents
			if (nodeMesh->HasTangentsAndBitangents() && !mode_export_animation)
			{
				tangentsSize = verticesSize;
				tangents = new float[tangentsSize * 3];
				memcpy(tangents, nodeMesh->mTangents, sizeof(float) * tangentsSize * 3);

				bitangentsSize = verticesSize;
				bitangents = new float[bitangentsSize * 3];
				memcpy(bitangents, nodeMesh->mBitangents, sizeof(float) * bitangentsSize * 3);
			}

			colorsSize = 0;
			// Color
			/*if (nodeMesh->HasVertexColors(0))
			{
				colorsSize = verticesSize;
				colors = new GLubyte[colorsSize * 4];
				memcpy(colors, nodeMesh->mColors, sizeof(GLubyte) * colorsSize * 4);
			}*/

			// Bones
			if (nodeMesh->HasBones() && !mode_export_animation)
			{
				boneInfluencesSize = nodeMesh->mNumBones;
				boneInfluences = new BoneInfluence[boneInfluencesSize];

				for (uint i = 0; i < nodeMesh->mNumBones; ++i)
				{
					if (rootBone == nullptr)
						rootBone = gameObject;

					aiBone* bone = nodeMesh->mBones[i];

					// Name
					strcpy_s(boneInfluences[i].boneName, DEFAULT_BUF_SIZE, bone->mName.C_Str());

					boneInfluences[i].bonesWeightsSize = bone->mNumWeights;
					boneInfluences[i].boneWeights = new float[bone->mNumWeights];
					boneInfluences[i].boneIds = new uint[bone->mNumWeights];

					for (uint j = 0; j < bone->mNumWeights; ++j)
					{
						memcpy(&boneInfluences[i].boneWeights[j], &bone->mWeights[j].mWeight, sizeof(float)); // strength of the influence
						memcpy(&boneInfluences[i].boneIds[j], &bone->mWeights[j].mVertexId, sizeof(uint)); // index of the vertex influenced by this bone
					}

					// Import the bone later
					bonesByName[boneInfluences[i].boneName] = bone; // only stores the last bone with this name
				}
			}

			// Texture coords
			if (nodeMesh->HasTextureCoords(0) && !mode_export_animation)
			{
				texCoordsSize = verticesSize;
				texCoords = new float[texCoordsSize * 2];

				for (uint i = 0; i < verticesSize; ++i)
				{
					memcpy(&texCoords[i * 2], &nodeMesh->mTextureCoords[0][i].x, sizeof(float));
					memcpy(&texCoords[(i * 2) + 1], &nodeMesh->mTextureCoords[0][i].y, sizeof(float));
				}

				// Material
				if (scene->mMaterials[nodeMesh->mMaterialIndex] != nullptr)
				{
					aiString textureName;
					scene->mMaterials[nodeMesh->mMaterialIndex]->GetTexture(aiTextureType_DIFFUSE, 0, &textureName);

					if (textureName.length > 0)
					{
						std::string fileName;
						App->fs->GetFileName(textureName.data, fileName, true);

						// Check if the texture exists in Assets
						std::string outputFile = DIR_ASSETS;
						if (App->fs->RecursiveExists(fileName.data(), DIR_ASSETS, outputFile))
						{
							uint uuid = 0;
							std::vector<uint> uuids;
							if (App->res->GetResourcesUuidsByFile(outputFile.data(), uuids))
								uuid = uuids.front();
							else
								// If the texture is not a resource yet, import it
								uuid = App->res->ImportFile(outputFile.data())->GetUuid();

							assert(uuid > 0);
							//gameObject->cmp_material->res[0].res = uuid; // TODO UNIFORMS
						}
					}
				}
			}


			// --------------------------------------------------

			if (!mode_export_animation) {

				uint nameSize = DEFAULT_BUF_SIZE;

				// Name
				char meshName[DEFAULT_BUF_SIZE];
				strcpy_s(meshName, DEFAULT_BUF_SIZE, name.data());

				// Vertices + Normals + Tangents + Bitangents + Colors + Texture Coords + Bones + Indices + Name
				uint ranges[] =
				{
					verticesSize, normalsSize,
					tangentsSize, bitangentsSize,
					colorsSize, texCoordsSize,

					// Bones
					boneInfluencesSize,

					indicesSize, nameSize
				};

				uint bonesSize = 0;
				for (uint i = 0; i < boneInfluencesSize; ++i)
				{
					bonesSize += sizeof(uint); // bonesWeightsSize
					bonesSize += sizeof(float) * boneInfluences[i].bonesWeightsSize; // boneWeights
					bonesSize += sizeof(uint) * boneInfluences[i].bonesWeightsSize; // boneIds
					bonesSize += sizeof(char) * nameSize; // boneName
				}

				uint size = sizeof(ranges) +
					sizeof(float) * verticesSize * 3 +
					sizeof(float) * normalsSize * 3 +
					sizeof(float) * tangentsSize * 3 +
					sizeof(float) * bitangentsSize * 3 +
					sizeof(uchar) * colorsSize * 4 +
					sizeof(float) * texCoordsSize * 2 +

					// Bones
					bonesSize +

					sizeof(uint) * indicesSize +
					sizeof(char) * nameSize;

				char* data = new char[size];
				char* cursor = data;

				// 1. Store ranges
				uint bytes = sizeof(ranges);
				memcpy(cursor, ranges, bytes);

				cursor += bytes;

				// 2. Store vertices
				bytes = sizeof(float) * verticesSize * 3;
				memcpy(cursor, vertices, bytes);

				cursor += bytes;

				// 3. Store normals
				if (normalsSize > 0)
				{
					bytes = sizeof(float) * normalsSize * 3;
					memcpy(cursor, normals, bytes);

					cursor += bytes;
				}

				// 4. Store tangents
				if (tangentsSize > 0)
				{
					bytes = sizeof(float) * tangentsSize * 3;
					memcpy(cursor, tangents, bytes);

					cursor += bytes;
				}

				// 5. Store bitangents
				if (bitangentsSize > 0)
				{
					bytes = sizeof(float) * bitangentsSize * 3;
					memcpy(cursor, bitangents, bytes);

					cursor += bytes;
				}

				// 6. Store colors
				if (colorsSize > 0)
				{
					bytes = sizeof(uchar) * colorsSize * 4;
					memcpy(cursor, colors, bytes);

					cursor += bytes;
				}

				// 7. Store texture coords
				if (texCoordsSize > 0)
				{
					bytes = sizeof(float) * texCoordsSize * 2;
					memcpy(cursor, texCoords, bytes);

					cursor += bytes;
				}

				// 8. Store bones
				for (uint i = 0; i < boneInfluencesSize; ++i)
				{
					bytes = sizeof(uint);
					memcpy(cursor, &boneInfluences[i].bonesWeightsSize, bytes);
					cursor += bytes;

					bytes = sizeof(float) * boneInfluences[i].bonesWeightsSize;
					memcpy(cursor, boneInfluences[i].boneWeights, bytes);
					cursor += bytes;

					bytes = sizeof(uint) * boneInfluences[i].bonesWeightsSize;
					memcpy(cursor, boneInfluences[i].boneIds, bytes);
					cursor += bytes;

					bytes = sizeof(char) * nameSize;
					memcpy(cursor, boneInfluences[i].boneName, bytes);
					cursor += bytes;
				}

				// 9. Store indices
				bytes = sizeof(uint) * indicesSize;
				memcpy(cursor, indices, bytes);

				cursor += bytes;

				// 10. Store name
				bytes = sizeof(char) * nameSize;
				memcpy(cursor, meshName, bytes);

				// Create the Mesh Resource
				std::string outputFile = std::to_string(gameObject->cmp_mesh->res);
				if (App->fs->SaveInGame(data, size, FileTypes::MeshFile, outputFile) > 0)
				{
					CONSOLE_LOG(LogTypes::Normal, "SCENE IMPORTER: Successfully saved Mesh '%s' to own format", gameObject->GetName());
					mesh_files.push_back(outputFile);
				}
				else
					CONSOLE_LOG(LogTypes::Error, "SCENE IMPORTER: Could not save Mesh '%s' to own format", gameObject->GetName());

				RELEASE_ARRAY(data);
				RELEASE_ARRAY(vertices);
				RELEASE_ARRAY(normals);
				RELEASE_ARRAY(tangents);
				RELEASE_ARRAY(bitangents);
				RELEASE_ARRAY(colors);
				RELEASE_ARRAY(texCoords);
				RELEASE_ARRAY(boneInfluences);
				RELEASE_ARRAY(indices);
			}
		}
	}

	for (uint i = 0; i < node->mNumChildren; ++i)
	{
		if (isTransformation)
			// If the current game object is a transformation, keep its parent and pass it as the new transformation for the next game object
			RecursivelyImportNodes(scene, node->mChildren[i], parent, gameObject, rootBone, bonesByName, mesh_files, forcedUuids);
		else
			// Else, the current game object becomes the new parent for the next game object
			RecursivelyImportNodes(scene, node->mChildren[i], gameObject, nullptr, rootBone, bonesByName, mesh_files, forcedUuids);
	}
}

void SceneImporter::ImportBones(GameObject* root,
	std::unordered_map<std::string, aiBone*>& bonesByName,
	std::vector<std::string>& bone_files, std::vector<uint>& forcedUuids) const
{
	std::vector<GameObject*> children;
	root->GetChildrenVector(children);

	for (uint i = 0; i < children.size(); ++i)
	{
		GameObject* gameObject = children[i];

		const char* boneName = gameObject->GetName();
		std::unordered_map<std::string, aiBone*>::const_iterator it = bonesByName.find(boneName);
		if (it == bonesByName.end())
			continue;

		aiBone* bone = it->second;

		// Create the Bone Component
		gameObject->AddComponent(ComponentTypes::BoneComponent);

		if (forcedUuids.size() > 0)
		{
			gameObject->cmp_bone->res = forcedUuids.front();
			forcedUuids.erase(forcedUuids.begin());
		}
		else
			gameObject->cmp_bone->res = App->GenerateRandomNumber();

		// Create the Bone Resource
		ResourceData data;
		data.name = std::to_string(gameObject->cmp_bone->res);

		ResourceBoneData boneData;
		boneData.name = boneName;
		memcpy(&boneData.offsetMatrix, &bone->mOffsetMatrix.a1, sizeof(float) * 16);

		// Export the new file
		std::string outputFile;
		if (ResourceBone::ExportFile(data, boneData, outputFile, false))
			bone_files.push_back(outputFile);
	}
}

void SceneImporter::ImportAnimations(const aiScene * scene, GameObject* rootBone, std::vector<std::string>& anim_files, const char* anim_name, std::vector<uint>& forcedUuids)const
{
	for (uint i = 0; i < scene->mNumAnimations; ++i)
	{
		const aiAnimation* anim = scene->mAnimations[i];
		CONSOLE_LOG(LogTypes::Normal, "Importing animation [%s] -----------------", anim->mName.C_Str());
		std::string output;

		if (rootBone || mode_export_animation)
		{
			uint res_uuid = 0u;
			if (forcedUuids.size() > 0)
			{
				res_uuid = forcedUuids.front();
				forcedUuids.erase(forcedUuids.begin());
			}
			else
				res_uuid = App->GenerateRandomNumber();

			std::string outputFile = std::to_string(res_uuid);

			ResourceData data;
			data.name = outputFile;
			data.file = outputFile;

			ResourceAnimationData res_data;

			std::string filename = anim_name;
			res_data.name = filename;

			res_data.ticksPerSecond = anim->mTicksPerSecond != 0 ? anim->mTicksPerSecond : 24;
			res_data.duration = anim->mDuration / res_data.ticksPerSecond;

			res_data.numKeys = anim->mNumChannels;

			// Once we have the animation data we populate the animation keys with the bones' data
			res_data.boneKeys = new BoneTransformation[res_data.numKeys];

			for (uint i = 0; i < anim->mNumChannels; ++i)
				App->animImporter->ImportBoneTransform(anim->mChannels[i], res_data.boneKeys[i]);

			App->animImporter->SaveAnimation(data, res_data, outputFile, false);

			anim_files.push_back(outputFile);
		}
	}
}

bool SceneImporter::Load(const char* exportedFile, ResourceData& outputData, ResourceMeshData& outputMeshData) const
{
	assert(exportedFile != nullptr);

	bool ret = false;

	char* buffer;
	uint size = App->fs->Load(exportedFile, &buffer);
	if (size > 0)
	{
		CONSOLE_LOG(LogTypes::Normal, "SCENE IMPORTER: Successfully loaded Mesh '%s' (own format)", exportedFile);
		ret = Load(buffer, size, outputData, outputMeshData);
		RELEASE_ARRAY(buffer);
	}
	else
		CONSOLE_LOG(LogTypes::Normal, "SCENE IMPORTER: Could not load Mesh '%s' (own format)", exportedFile);

	return ret;
}

bool SceneImporter::Load(const void* buffer, uint size, ResourceData& outputData, ResourceMeshData& outputMeshData) const
{
	assert(buffer != nullptr && size > 0);

	char* cursor = (char*)buffer;

	// Vertices + Normals + Tangents + Bitangents + Colors + Texture Coords + Bones + Indices + Name
	uint ranges[9];

	// 1. Load ranges
	uint bytes = sizeof(ranges);
	memcpy(ranges, cursor, bytes);

	cursor += bytes;

	outputMeshData.verticesSize = ranges[0];
	uint normalsSize = ranges[1];
	uint tangentsSize = ranges[2];
	uint bitangentsSize = ranges[3];
	uint colorsSize = ranges[4];
	uint texCoordsSize = ranges[5];

	// Bones
	outputMeshData.boneInfluencesSize = ranges[6];

	outputMeshData.indicesSize = ranges[7];
	uint nameSize = ranges[8];

	char* normalsCursor = cursor + ranges[0] * sizeof(float) * 3;
	char* tangentsCursor = normalsCursor + ranges[1] * sizeof(float) * 3;
	char* bitangentsCursor = tangentsCursor + ranges[2] * sizeof(float) * 3;
	char* colorCursor = bitangentsCursor + ranges[3] * sizeof(float) * 3;
	char* texCoordsCursor = colorCursor + ranges[4] * sizeof(uchar) * 4;

	outputMeshData.vertices = new Vertex[outputMeshData.verticesSize];
	for (uint i = 0; i < outputMeshData.verticesSize; ++i)
	{
		// 2. Load vertices
		bytes = sizeof(float) * 3;
		memcpy(outputMeshData.vertices[i].position, cursor, bytes);

		cursor += bytes;

		// 3. Load normals
		if (normalsSize > 0)
		{
			bytes = sizeof(float) * 3;
			memcpy(outputMeshData.vertices[i].normal, normalsCursor, bytes);

			normalsCursor += bytes;
		}

		// 4. Load tangents
		if (tangentsSize > 0)
		{
			bytes = sizeof(float) * 3;
			memcpy(outputMeshData.vertices[i].tangent, tangentsCursor, bytes);

			tangentsCursor += bytes;
		}

		// 5. Load bitangents
		if (bitangentsSize > 0)
		{
			bytes = sizeof(float) * 3;
			memcpy(outputMeshData.vertices[i].bitangent, bitangentsCursor, bytes);

			bitangentsCursor += bytes;
		}

		// 6. Load colors
		if (colorsSize > 0)
		{
			bytes = sizeof(uchar) * 4;
			memcpy(outputMeshData.vertices[i].color, colorCursor, bytes);

			colorCursor += bytes;
		}

		// 7. Load texture coords
		if (texCoordsSize > 0)
		{
			bytes = sizeof(float) * 2;
			memcpy(outputMeshData.vertices[i].texCoord, texCoordsCursor, bytes);

			texCoordsCursor += bytes;
		}
	}

	cursor = texCoordsCursor;

	// 8. Load bones
	if (outputMeshData.boneInfluencesSize > 0)
		outputMeshData.boneInfluences = new BoneInfluence[outputMeshData.boneInfluencesSize];

	for (uint i = 0; i < outputMeshData.boneInfluencesSize; ++i)
	{
		bytes = sizeof(uint);
		memcpy(&outputMeshData.boneInfluences[i].bonesWeightsSize, cursor, bytes);
		cursor += bytes;

		bytes = sizeof(float) * outputMeshData.boneInfluences[i].bonesWeightsSize;
		outputMeshData.boneInfluences[i].boneWeights = new float[outputMeshData.boneInfluences[i].bonesWeightsSize];
		memcpy(outputMeshData.boneInfluences[i].boneWeights, cursor, bytes);
		cursor += bytes;

		bytes = sizeof(uint) * outputMeshData.boneInfluences[i].bonesWeightsSize;
		outputMeshData.boneInfluences[i].boneIds = new uint[outputMeshData.boneInfluences[i].bonesWeightsSize];
		memcpy(outputMeshData.boneInfluences[i].boneIds, cursor, bytes);
		cursor += bytes;

		bytes = sizeof(char) * DEFAULT_BUF_SIZE;
		memcpy(outputMeshData.boneInfluences[i].boneName, cursor, bytes);
		cursor += bytes;
	}
	
	// 9. Load indices
	bytes = sizeof(uint) * outputMeshData.indicesSize;
	outputMeshData.indices = new uint[outputMeshData.indicesSize];
	memcpy(outputMeshData.indices, cursor, bytes);

	cursor += bytes;

	// 10. Load name
	bytes = sizeof(char) * nameSize;
	outputData.name.resize(nameSize);
	memcpy(&outputData.name[0], cursor, bytes);

	CONSOLE_LOG(LogTypes::Normal, "SCENE IMPORTER: New mesh loaded with %u vertices and %u indices", outputMeshData.verticesSize, outputMeshData.indicesSize);

	return true;
}

void SceneImporter::GenerateVBO(uint& VBO, Vertex* vertices, uint verticesSize) const
{
	assert(vertices != nullptr && verticesSize > 0);

	// Vertex Buffer Object

	// Generate a VBO
	glGenBuffers(1, &VBO);
	// Bind the VBO
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * verticesSize, vertices, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void SceneImporter::GenerateIBO(uint& IBO, uint* indices, uint indicesSize) const
{
	assert(indices != nullptr && indicesSize > 0);

	// Index Buffer Object

	// Generate a IBO
	glGenBuffers(1, &IBO);
	// Bind the IBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indicesSize, indices, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void SceneImporter::GenerateVAO(uint& VAO, uint& VBO, uint attrFlag) const
{
	// Vertex Array Object

	// Generate a VAO
	glGenVertexArrays(1, &VAO);
	// Bind the VAO
	glBindVertexArray(VAO);

	// Bind the VBO
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	// Set the vertex attributes pointers

	// 1. Position
	//if (attrFlag & ResourceMeshImportSettings::ATTR_POSITION)
	//{
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, position)));
		glEnableVertexAttribArray(0);
	//}

	// 2. Normal
	//if (attrFlag & ResourceMeshImportSettings::ATTR_NORMAL)
	//{
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, normal)));
		glEnableVertexAttribArray(1);
	//}

	// 3. Color
	//if (attrFlag & ResourceMeshImportSettings::ATTR_COLOR)
	//{
		glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void*)(offsetof(Vertex, color)));
		glEnableVertexAttribArray(2);
	//}

	// 4. Tex coords
	//if (attrFlag & ResourceMeshImportSettings::ATTR_TEXCOORD)
	//{
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, texCoord)));
		glEnableVertexAttribArray(3);
	//}

	// 5. Tangents
	//if (attrFlag & ResourceMeshImportSettings::ATTR_TANGENT)
	//{
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, tangent)));
		glEnableVertexAttribArray(4);
	//}

	// 6. Bitangents
	//if (attrFlag & ResourceMeshImportSettings::ATTR_BITANGENT)
	//{
		glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, bitangent)));
		glEnableVertexAttribArray(5);
	//}

	//if (attrFlag & ResourceMeshImportSettings::ATTR_ANIMATION)
	//{
		// 7. Weights
		glVertexAttribPointer(6, MAX_BONES_PER_VERTEX, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, boneWeight)));
		glEnableVertexAttribArray(6);

		// 8. Ids
		glVertexAttribIPointer(7, MAX_BONES_PER_VERTEX, GL_INT, sizeof(Vertex), (void*)(offsetof(Vertex, boneId)));
		glEnableVertexAttribArray(7);
	//}

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void SceneImporter::DeleteBufferObject(uint& name) const
{
	glDeleteBuffers(1, &name);
	name = 0;
}

void SceneImporter::DeleteVertexArrayObject(uint& name) const
{
	glDeleteVertexArrays(1, &name);
	name = 0;
}

// ----------------------------------------------------------------------------------------------------

uint SceneImporter::GetAssimpMajorVersion() const
{
	return aiGetVersionMajor();
}

uint SceneImporter::GetAssimpMinorVersion() const
{
	return aiGetVersionMinor();
}

uint SceneImporter::GetAssimpRevisionVersion() const
{
	return aiGetVersionRevision();
}