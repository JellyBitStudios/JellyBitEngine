#include "ResourceMesh.h"

#include "Application.h"
#include "ModuleFileSystem.h"
#include "ModuleResourceManager.h"
#include "ModuleGOs.h"
#include "ModuleScene.h"

#include "SceneImporter.h"
#include "GameObject.h"

#include "ResourceBone.h"
#include "ComponentBone.h"

#include "imgui\imgui.h"

#include <assert.h>

ResourceMesh::ResourceMesh(ResourceTypes type, uint uuid, ResourceData data, ResourceMeshData meshData) : Resource(type, uuid, data), meshData(meshData) {}

ResourceMesh::~ResourceMesh() 
{
	RELEASE_ARRAY(meshData.vertices);
	RELEASE_ARRAY(meshData.indices);
	//RELEASE_ARRAY(meshData.adjacentIndices); //TODO S
	RELEASE_ARRAY(meshData.boneInfluences);
}

void ResourceMesh::OnPanelAssets()
{
#ifndef GAMEMODE
	ImGuiTreeNodeFlags flags = 0;
	flags |= ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_Leaf;

	if (App->scene->selectedObject == this)
		flags |= ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_Selected;

	char id[DEFAULT_BUF_SIZE];
	sprintf(id, "%s##%d", data.name.data(), uuid);

	if (ImGui::TreeNodeEx(id, flags))
		ImGui::TreePop();

	if (ImGui::IsMouseReleased(0) && ImGui::IsItemHovered() /*&& (mouseDelta.x == 0 && mouseDelta.y == 0)*/)
	{
		SELECT(this);
	}

	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
	{
		ImGui::SetDragDropPayload("MESH_INSPECTOR_SELECTOR", &uuid, sizeof(uint));
		ImGui::EndDragDropSource();
	}
#endif
}

// ----------------------------------------------------------------------------------------------------

bool ResourceMesh::ImportFile(const char* file, ResourceMeshImportSettings& meshImportSettings, 
	std::vector<std::string>& mesh_files, std::vector<std::string>& bone_files, std::vector<std::string>& animation_files)
{
	assert(file != nullptr);

	bool imported = false;

	// Search for the meta associated to the file
	char metaFile[DEFAULT_BUF_SIZE];
	strcpy_s(metaFile, strlen(file) + 1, file); // file
	strcat_s(metaFile, strlen(metaFile) + strlen(EXTENSION_META) + 1, EXTENSION_META); // extension

	// CASE 1 (file). The file has no meta associated (the file is new)
	if (!App->fs->Exists(metaFile))
	{
		// Import the file (using the default import settings)
		CONSOLE_LOG(LogTypes::Normal, "Resource Mesh: The file '%s' needs to be imported", file);

		imported = App->sceneImporter->Import(file, mesh_files, bone_files, animation_files, meshImportSettings);
	}
	else
	{
		std::vector<uint> mesh_uuids;
		std::vector<uint> bones_uuids;
		std::vector<uint> animation_uuids;
		
		int64_t lastModTime = 0;
		bool result = ResourceMesh::ReadMeta(metaFile, lastModTime, meshImportSettings, mesh_uuids, bones_uuids, animation_uuids);
		assert(result);

		char entry[DEFAULT_BUF_SIZE]; 

		std::vector<std::string> mesh_entry_files;
		std::vector<std::string> bone_entry_files;
		std::vector<std::string> animation_entry_files;

		std::string entryFile;
		for (uint i = 0; i < mesh_uuids.size(); ++i)
		{
			sprintf_s(entry, "%u%s", mesh_uuids[i], EXTENSION_MESH);
			entryFile = DIR_LIBRARY;
			if (App->fs->RecursiveExists(entry, DIR_LIBRARY, entryFile))
				mesh_entry_files.push_back(entryFile);
			entryFile.clear();
		}

		for (uint i = 0; i < bones_uuids.size(); ++i)
		{
			sprintf_s(entry, "%u%s", bones_uuids[i], EXTENSION_BONE);
			entryFile = DIR_LIBRARY;
			if (App->fs->RecursiveExists(entry, DIR_LIBRARY, entryFile))
				bone_entry_files.push_back(entryFile);
			entryFile.clear();
		}

		for (uint i = 0; i < animation_uuids.size(); ++i)
		{
			sprintf_s(entry, "%u%s", animation_uuids[i], EXTENSION_ANIMATION);
			entryFile = DIR_LIBRARY;
			if (App->fs->RecursiveExists(entry, DIR_LIBRARY, entryFile))
				animation_entry_files.push_back(entryFile);
			entryFile.clear();
		}

		// CASE 2 (file + meta + Library file(s)). The resource(s) do(es)n't exist
		if (mesh_entry_files.size() == mesh_uuids.size() && 
			bone_entry_files.size() == bones_uuids.size() && 
			animation_entry_files.size() == animation_uuids.size())
		{ 
			mesh_files = mesh_entry_files;
			bone_files = bone_entry_files;
			animation_files = animation_entry_files;
			imported = true;
		}			
		// CASE 3 (file + meta). The file(s) in Libray associated to the meta do(es)n't exist
		else
		{
			// Import the file (using the import settings from the meta)
			CONSOLE_LOG(LogTypes::Normal, "Resource Mesh: The file '%s' has Library file(s) that need(s) to be reimported", file);

			imported = App->sceneImporter->Import(file, mesh_files,bone_files, animation_files, meshImportSettings, mesh_uuids,bones_uuids,animation_uuids);
		}
	}

	return imported;
}

// Returns the last modification time of the file
uint ResourceMesh::CreateMeta(const char* file, ResourceMeshImportSettings& meshImportSettings, 
	std::vector<uint>& meshesUuids, std::vector<uint>& bonesUuids, std::vector<uint>& animationUuids, std::string& outputMetaFile)
{
	assert(file != nullptr);

	uint mesh_uuids_size = meshesUuids.size();
	uint bones_uuids_size = bonesUuids.size();
	uint animations_uuids_size = animationUuids.size();

	uint size =
		sizeof(int64_t) +						// data mod
		sizeof(uint) +							// mesh size num
		sizeof(uint) * mesh_uuids_size +		// mesh uuids

		sizeof(uint) +							// size bone num
		sizeof(uint) * bones_uuids_size +		// bone uuids

		sizeof(uint) +							// size anim num
		sizeof(uint) * animations_uuids_size +	// anim uuids

		sizeof(ResourceMeshImportSettings);

	char* data = new char[size];
	char* cursor = data;

	// 1. Store last modification time
	int64_t lastModTime = App->fs->GetLastModificationTime(file);
	assert(lastModTime > 0);
	uint bytes = sizeof(int64_t);
	memcpy(cursor, &lastModTime, bytes);

	cursor += bytes;

	// 2. Store meshes uuids size
	bytes = sizeof(uint);
	assert(mesh_uuids_size > 0);
	memcpy(cursor, &mesh_uuids_size, bytes);

	cursor += bytes;

	// 3. Store meshes uuids
	bytes = sizeof(uint) * meshesUuids.size();
	memcpy(cursor, &meshesUuids[0], bytes);

	cursor += bytes;

	// 4. Store bones uuids size
	bytes = sizeof(uint);
	memcpy(cursor, &bones_uuids_size, bytes);

	cursor += bytes;

	// 5. Store bones uuids
	bytes = sizeof(uint) * bonesUuids.size();
	if(bonesUuids.size()>0)
		memcpy(cursor, &bonesUuids[0], bytes);

	cursor += bytes;

	// 6. Store anim uuids size
	bytes = sizeof(uint);
	memcpy(cursor, &animations_uuids_size, bytes);

	cursor += bytes;

	// 7. Store anim uuids
	bytes = sizeof(uint) * animationUuids.size();
	if (animationUuids.size() > 0)
		memcpy(cursor, &animationUuids[0], bytes);

	cursor += bytes;

	// 8. Store import settings
	bytes = sizeof(ResourceMeshImportSettings);
	memcpy(cursor, &meshImportSettings, bytes);

	cursor += bytes;

	// --------------------------------------------------

	// Build the path of the meta file and save it
	outputMetaFile = file;
	outputMetaFile.append(EXTENSION_META);
	uint resultSize = App->fs->Save(outputMetaFile.data(), data, size);
	if (resultSize > 0)
	{
		CONSOLE_LOG(LogTypes::Normal, "Resource Mesh: Successfully saved meta '%s'", outputMetaFile.data());
	}
	else
	{
		CONSOLE_LOG(LogTypes::Error, "Resource Mesh: Could not save meta '%s'", outputMetaFile.data());
		return 0;
	}

	return lastModTime;
}

bool ResourceMesh::ReadMeta(const char* metaFile, int64_t& lastModTime, ResourceMeshImportSettings& meshImportSettings,
	std::vector<uint>& meshesUuids, std::vector<uint>& bonesUuids, std::vector<uint>& animationUuids)
{
	assert(metaFile != nullptr);

	char* buffer;
	uint size = App->fs->Load(metaFile, &buffer);
	if (size > 0)
	{		
		char* cursor = (char*)buffer;

		// 1. Load last modification time
		uint bytes = sizeof(int64_t);
		memcpy(&lastModTime, cursor, bytes);

		cursor += bytes;

		// 2. Load uuids size
		uint uuidsSize = 0;
		bytes = sizeof(uint);
		memcpy(&uuidsSize, cursor, bytes);
		assert(uuidsSize > 0);

		cursor += bytes;

		// 3. Load meshes uuids
		meshesUuids.resize(uuidsSize);
		bytes = sizeof(uint) * uuidsSize;
		memcpy(&meshesUuids[0], cursor, bytes);

		cursor += bytes;

		// 4. Load bones uuids size
		uuidsSize = 0;
		bytes = sizeof(uint);
		memcpy(&uuidsSize, cursor, bytes);
		//assert(uuidsSize > 0);

		cursor += bytes;

		// 5. Load bones uuids
		bonesUuids.resize(uuidsSize);
		bytes = sizeof(uint) * uuidsSize;
		if( bonesUuids.size() > 0)
			memcpy(&bonesUuids[0], cursor, bytes);

		cursor += bytes;

		// 6. Load anims uuids size
		uuidsSize = 0;
		bytes = sizeof(uint);
		memcpy(&uuidsSize, cursor, bytes);
		//assert(uuidsSize > 0);

		cursor += bytes;

		// 7. Load anims uuids
		animationUuids.resize(uuidsSize);
		bytes = sizeof(uint) * uuidsSize;
		if (animationUuids.size() > 0)
			memcpy(&animationUuids[0], cursor, bytes);

		cursor += bytes;

		// 8. Load import settings
		bytes = sizeof(ResourceMeshImportSettings);
		memcpy(&meshImportSettings, cursor, bytes);

		cursor += bytes;

		CONSOLE_LOG(LogTypes::Normal, "Resource Mesh: Successfully loaded meta '%s'", metaFile);
		RELEASE_ARRAY(buffer);
	}
	else
	{
		CONSOLE_LOG(LogTypes::Error, "Resource Mesh: Could not load meta '%s'", metaFile);
		return false;
	}

	return true;
}

// Returns true if the meshes uuids vector is not empty. Else, returns false
bool ResourceMesh::ReadMeshesUuidsFromBuffer(const char* buffer, 
	std::vector<uint>& meshesUuids, std::vector<uint>& bonesUuids, std::vector<uint>& animationUuids)
{
	char* cursor = (char*)buffer;

	// 1. (Last modification time)
	uint bytes = sizeof(int64_t);
	cursor += bytes;

	// 2. Load uuids size
	uint uuidsSize = 0;
	bytes = sizeof(uint);
	memcpy(&uuidsSize, cursor, bytes);
	assert(uuidsSize > 0);

	cursor += bytes;

	// 3. Load meshes uuids
	meshesUuids.resize(uuidsSize);
	bytes = sizeof(uint) * uuidsSize;
	memcpy(&meshesUuids[0], cursor, bytes);

	cursor += bytes;

	// 4. Load bones uuids size
	uuidsSize = 0;
	bytes = sizeof(uint);
	memcpy(&uuidsSize, cursor, bytes);
	//assert(uuidsSize > 0);

	cursor += bytes;

	// 5. Load bones uuids
	bonesUuids.resize(uuidsSize);
	bytes = sizeof(uint) * uuidsSize;
	if (bonesUuids.size() > 0)
		memcpy(&bonesUuids[0], cursor, bytes);

	cursor += bytes;

	// 6. Load anims uuids size
	uuidsSize = 0;
	bytes = sizeof(uint);
	memcpy(&uuidsSize, cursor, bytes);
	//assert(uuidsSize > 0);

	cursor += bytes;

	// 7. Load anims uuids
	animationUuids.resize(uuidsSize);
	bytes = sizeof(uint) * uuidsSize;
	if (animationUuids.size() > 0)
		memcpy(&animationUuids[0], cursor, bytes);

	cursor += bytes;

	if (meshesUuids.size() > 0)
		return true;

	return false;
}

uint ResourceMesh::SetMeshImportSettingsToMeta(const char* metaFile, const ResourceMeshImportSettings& meshImportSettings)
{
	assert(metaFile != nullptr);

	int64_t lastModTime = 0;
	ResourceMeshImportSettings oldMeshImportSettings;
	std::vector<uint> meshesUuids;
	std::vector<uint> bonesUuids;
	std::vector<uint> animationUuids;
	ReadMeta(metaFile, lastModTime, oldMeshImportSettings, meshesUuids, bonesUuids, animationUuids);

	uint uuidsSize = meshesUuids.size();
	uint bones_uuids_size = bonesUuids.size();
	uint animation_uuids_size = animationUuids.size();

	uint size =
		sizeof(int64_t) +
		sizeof(uint) +
		sizeof(uint) * uuidsSize +

		sizeof(uint) +
		sizeof(uint) * bones_uuids_size +

		sizeof(uint) +
		sizeof(uint) * animation_uuids_size +

		sizeof(ResourceMeshImportSettings);

	char* data = new char[size];
	char* cursor = data;

	// 1. Store last modification time
	uint bytes = sizeof(int64_t);
	assert(lastModTime > 0);
	memcpy(cursor, &lastModTime, bytes);

	cursor += bytes;

	// 2. Store uuids size
	bytes = sizeof(uint);
	assert(uuidsSize > 0);
	memcpy(cursor, &uuidsSize, bytes);

	cursor += bytes;

	// 3. Store meshes uuids
	bytes = sizeof(uint) * meshesUuids.size();
	memcpy(cursor, &meshesUuids[0], bytes);

	cursor += bytes;

	// 4. Store bones uuids size
	bytes = sizeof(uint);
	memcpy(cursor, &bones_uuids_size, bytes);

	cursor += bytes;

	// 5. Store bones uuids
	bytes = sizeof(uint) * bonesUuids.size();
	if(bonesUuids.size() > 0)
		memcpy(cursor, &bonesUuids[0], bytes);

	cursor += bytes;

	// 6. Store anims uuids size
	bytes = sizeof(uint);
	memcpy(cursor, &animation_uuids_size, bytes);

	cursor += bytes;

	// 7. Store anims uuids
	bytes = sizeof(uint) * animationUuids.size();
	if (animationUuids.size() > 0)
		memcpy(cursor, &animationUuids[0], bytes);

	cursor += bytes;

	// 8. Store import settings
	bytes = sizeof(ResourceMeshImportSettings);
	memcpy(cursor, &meshImportSettings, bytes);

	cursor += bytes;

	// --------------------------------------------------

	// Save the meta file
	uint resultSize = App->fs->Save(metaFile, data, size);
	if (resultSize > 0)
	{
		CONSOLE_LOG(LogTypes::Normal, "Resource Mesh: Successfully saved meta '%s'", metaFile);
	}
	else
	{
		CONSOLE_LOG(LogTypes::Error, "Resource Mesh: Could not save meta '%s'", metaFile);
		return 0;
	}

	return lastModTime;
}

bool ResourceMesh::GenerateLibraryFiles() const
{
	assert(data.file.data() != nullptr);

	// Search for the meta associated to the file
	char metaFile[DEFAULT_BUF_SIZE];
	strcpy_s(metaFile, strlen(data.file.data()) + 1, data.file.data()); // file
	strcat_s(metaFile, strlen(metaFile) + strlen(EXTENSION_META) + 1, EXTENSION_META); // extension

	// 1. Copy meta
	if (App->fs->Exists(metaFile))
	{
		// Read the info of the meta
		char* buffer;
		uint size = App->fs->Load(metaFile, &buffer);
		if (size > 0)
		{
			// Create a new name for the meta
			char newMetaFile[DEFAULT_BUF_SIZE];
			sprintf_s(newMetaFile, "%s/%u%s%s", DIR_LIBRARY_MESHES, uuid, EXTENSION_MESH, EXTENSION_META);

			// Save the new meta (info + new name)
			size = App->fs->Save(newMetaFile, buffer, size);
			if (size > 0)
			{
				RELEASE_ARRAY(buffer);
				return true;
			}
		}
	}

	return false;
}

// ----------------------------------------------------------------------------------------------------

void ResourceMesh::GetVerticesReference(Vertex*& vertices) const
{
	vertices = meshData.vertices;
}

void ResourceMesh::GetTris(float* verticesPosition) const
{
	for (int i = 0, j = -1; i < meshData.verticesSize; ++i)
	{
		verticesPosition[++j] = meshData.vertices[i].position[0];
		verticesPosition[++j] = meshData.vertices[i].position[1];
		verticesPosition[++j] = meshData.vertices[i].position[2];
	}
}

void ResourceMesh::GetIndicesReference(uint*& indices) const
{
	indices = meshData.indices;
}

void ResourceMesh::GetIndices(uint* indices) const
{
	memcpy(indices, meshData.indices, meshData.indicesSize * sizeof(uint));
}

uint ResourceMesh::GetVerticesCount() const
{
	return meshData.verticesSize;
}

uint ResourceMesh::GetIndicesCount() const
{
	return meshData.indicesSize;
}

bool ResourceMesh::UseAdjacency() const
{
	return meshData.meshImportSettings.adjacency;
}

bool ResourceMesh::AddBones(const std::unordered_map<std::string, uint>& bones)
{
	uint addedBones = 0;

	uint boneId = 0; // this id matches the uniform bones array id
	for (std::unordered_map<std::string, uint>::const_iterator it = bones.begin(); it != bones.end(); ++it, ++boneId)
	{
		/// Bone game object
		GameObject* boneGameObject = App->GOs->GetGameObjectByUID(it->second);
		if (boneGameObject == nullptr)
			continue;

		/// Bone component
		ComponentBone* boneComponent = boneGameObject->cmp_bone;
		if (boneComponent == nullptr)
			continue;

		/// Bone resource
		ResourceBone* boneResource = (ResourceBone*)App->res->GetResource(boneComponent->res);
		if (boneResource == nullptr)
			continue;

		// Bone
		for (uint i = 0; i < meshData.boneInfluencesSize; ++i)
		{
			if (strcmp(boneResource->boneData.name.data(), meshData.boneInfluences[i].boneName) == 0)
			{
				// Vertices influenced by the bone
				for (uint j = 0; j < meshData.boneInfluences[i].bonesWeightsSize; ++j)
				{
					if (AddBone(meshData.boneInfluences[i].boneIds[j], meshData.boneInfluences[i].boneWeights[j], boneId))
						++addedBones;
					else
						CONSOLE_LOG(LogTypes::Error, "Resource Mesh: The bone %s could not be added to the mesh", boneResource->boneData.name.data());
				}
			}
			else
				CONSOLE_LOG(LogTypes::Error, "Resource Mesh: The bone %s could not be found nor added to the mesh", boneResource->boneData.name.data());
		}
	}

	DeleteVAO();
	GenerateVAO();

	return addedBones == boneId;
}

bool ResourceMesh::AddBone(uint vertexId, float boneWeight, uint boneId)
{
	assert(vertexId >= 0 && vertexId < meshData.verticesSize);
	
	for (uint i = 0; i < MAX_BONES_PER_VERTEX; ++i)
	{
		// Find an empy slot
		if (meshData.vertices[vertexId].boneWeight[i] == 0.0f)
		{
			meshData.vertices[vertexId].boneWeight[i] = boneWeight;
			meshData.vertices[vertexId].boneId[i] = boneId;
			return true;
		}
	}

	return false;
}

void ResourceMesh::CalculateAdjacentIndices(uint* indices, uint indicesSize, uint*& adjacentIndices)
{
	adjacentIndices = new uint[indicesSize * 2];

	for (uint i = 0; i < indicesSize; ++i)
	{
		adjacentIndices[i * 2 + 0] = indices[i];
		adjacentIndices[i * 2 + 1] = 0;
		adjacentIndices[i * 2 + 2] = indices[i + 1];
		adjacentIndices[i * 2 + 3] = 0;
		adjacentIndices[i * 2 + 4] = indices[i + 2];
		adjacentIndices[i * 2 + 5] = 0;
	}

	// Find matching edges
	for (uint i = 0; i < indicesSize * 2; i+=6)
	{
		// Triangle A
		uint a1 = adjacentIndices[i];
		uint b1 = adjacentIndices[i + 2];
		uint c1 = adjacentIndices[i + 4];

		// Scan subsequent triangles
		for (uint j = i + 6; j < indicesSize * 2; j += 6)
		{
			// Triangle B
			uint a2 = adjacentIndices[j];
			uint b2 = adjacentIndices[j + 2];
			uint c2 = adjacentIndices[j + 4];

			// Edge 1 == Edge 1
			if ((a1 == a2 && b1 == b2) || (a1 == b2 && b1 == a2))
			{
				adjacentIndices[i + 1] = c2;
				adjacentIndices[j + 1] = c1;
			}
			// Edge 1 == Edge 2
			if ((a1 == b2 && b1 == c2) || (a1 == c2 && b1 == b2))
			{
				adjacentIndices[i + 1] = a2;
				adjacentIndices[j + 3] = c1;
			}
			// Edge 1 == Edge 3
			if ((a1 == c2 && b1 == a2) || (a1 == a2 && b1 == c2))
			{
				adjacentIndices[i + 1] = b2;
				adjacentIndices[j + 5] = c1;
			}
			// Edge 2 == Edge 1
			if ((b1 == a2 && c1 == b2) || (b1 == b2 && c1 == a2))
			{
				adjacentIndices[i + 3] = c2;
				adjacentIndices[j + 1] = a1;
			}
			// Edge 2 == Edge 2
			if ((b1 == b2 && c1 == c2) || (b1 == c2 && c1 == b2))
			{
				adjacentIndices[i + 3] = a2;
				adjacentIndices[j + 3] = a1;
			}
			// Edge 2 == Edge 3
			if ((b1 == c2 && c1 == a2) || (b1 == a2 && c1 == c2))
			{
				adjacentIndices[i + 3] = b2;
				adjacentIndices[j + 5] = a1;
			}
			// Edge 3 == Edge 1
			if ((c1 == a2 && a1 == b2) || (c1 == b2 && a1 == a2))
			{
				adjacentIndices[i + 5] = c2;
				adjacentIndices[j + 1] = b1;
			}
			// Edge 3 == Edge 2
			if ((c1 == b2 && a1 == c2) || (c1 == c2 && a1 == b2))
			{
				adjacentIndices[i + 5] = a2;
				adjacentIndices[j + 3] = b1;
			}
			// Edge 3 == Edge 3
			if ((c1 == c2 && a1 == a2) || (c1 == a2 && a1 == c2))
			{
				adjacentIndices[i + 5] = b2;
				adjacentIndices[j + 5] = b1;
			}
		}
	}

	// Look for any outside edges
	for (uint i = 0; i < indicesSize * 2; i += 6)
	{
		if (adjacentIndices[i + 1] == 0)
			adjacentIndices[i + 1] = adjacentIndices[i + 4];
		if (adjacentIndices[i + 3] == 0)
			adjacentIndices[i + 3] = adjacentIndices[i];
		if (adjacentIndices[i + 5] == 0)
			adjacentIndices[i + 5] = adjacentIndices[i + 2];
	}
}

uint ResourceMesh::GetVBO() const
{
	return VBO;
}

uint ResourceMesh::GetIBO() const
{
	return IBO;
}

uint ResourceMesh::GetVAO() const
{
	return VAO;
}

// ----------------------------------------------------------------------------------------------------

bool ResourceMesh::LoadInMemory()
{
	assert(meshData.vertices != nullptr && meshData.verticesSize > 0
		&& meshData.indices != nullptr && meshData.indicesSize > 0);

	// Calculate adjacent indices
	if (UseAdjacency())
		ResourceMesh::CalculateAdjacentIndices(meshData.indices, meshData.indicesSize, meshData.adjacentIndices);

	GenerateVAO();

	return true;
}

bool ResourceMesh::UnloadFromMemory()
{
	DeleteVAO();

	return true;
}

void ResourceMesh::GenerateVAO()
{
	App->sceneImporter->GenerateVBO(VBO, meshData.vertices, meshData.verticesSize);
	App->sceneImporter->GenerateIBO(IBO, UseAdjacency() ? meshData.adjacentIndices : meshData.indices, UseAdjacency() ? meshData.indicesSize * 2 : meshData.indicesSize);
	App->sceneImporter->GenerateVAO(VAO, VBO, meshData.meshImportSettings.attributes);
}

void ResourceMesh::DeleteVAO()
{
	App->sceneImporter->DeleteBufferObject(VBO);
	App->sceneImporter->DeleteBufferObject(IBO);
	App->sceneImporter->DeleteVertexArrayObject(VAO);
}