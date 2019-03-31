#include "ResourceAvatar.h"

#include "Application.h"
#include "ModuleFileSystem.h"
#include "ModuleGOs.h"
#include "ModuleResourceManager.h"
#include "ModuleScene.h"

#include "ComponentBone.h"
#include "ComponentMesh.h"
#include "ResourceBone.h"
#include "ResourceMesh.h"
#include "ResourceAnimation.h"
#include "ComponentTransform.h"
#include "GameObject.h"

#include "MathGeoLib\include\MathGeoLib.h"

#include "imgui\imgui.h"

#include <assert.h>

ResourceAvatar::ResourceAvatar(ResourceTypes type, uint uuid, ResourceData data, ResourceAvatarData avatarData) : Resource(type, uuid, data), avatarData(avatarData) {}

ResourceAvatar::~ResourceAvatar() {}

void ResourceAvatar::OnPanelAssets()
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
		ImGui::SetDragDropPayload("AVATAR_INSPECTOR_SELECTOR", &uuid, sizeof(uint));
		ImGui::EndDragDropSource();
	}
#endif
}

// ----------------------------------------------------------------------------------------------------

bool ResourceAvatar::ImportFile(const char* file, std::string& name, std::string& outputFile)
{
	assert(file != nullptr);

	// Search for the meta associated to the file
	char metaFile[DEFAULT_BUF_SIZE];
	strcpy_s(metaFile, strlen(file) + 1, file); // file
	strcat_s(metaFile, strlen(metaFile) + strlen(EXTENSION_META) + 1, EXTENSION_META); // extension

	if (App->fs->Exists(metaFile))
	{
		// Read the meta
		uint uuid = 0;
		int64_t lastModTime = 0;
		bool result = ResourceAvatar::ReadMeta(metaFile, lastModTime, uuid, name);
		assert(result);

		// The uuid of the resource would be the entry
		char entry[DEFAULT_BUF_SIZE];
		sprintf_s(entry, "%u", uuid);
		outputFile = entry;
	}

	return true;
}

bool ResourceAvatar::ExportFile(const ResourceData& data, const ResourceAvatarData& avatarData, std::string& outputFile, bool overwrite)
{
	return SaveFile(data, avatarData, outputFile, overwrite) > 0;
}

uint ResourceAvatar::SaveFile(const ResourceData& data, const ResourceAvatarData& avatarData, std::string& outputFile, bool overwrite)
{
	uint size =
		sizeof(uint) + // hips uuid
		sizeof(uint); // root uuid

	char* buffer = new char[size];
	char* cursor = buffer;

	// 1. Store hips uuid
	uint bytes = sizeof(uint);
	memcpy(cursor, &avatarData.hipsUuid, bytes);

	cursor += bytes;

	// 2. Store root uuid
	bytes = sizeof(uint);
	memcpy(cursor, &avatarData.rootUuid, bytes);

	//cursor += bytes;

	// --------------------------------------------------

	// Build the path of the file
	if (overwrite)
		outputFile = data.file;
	else
		outputFile = data.name;

	// Save the file
	uint retSize = App->fs->SaveInGame(buffer, size, FileTypes::AvatarFile, outputFile, overwrite) > 0;

	if (retSize > 0)
	{
		CONSOLE_LOG(LogTypes::Normal, "Resource Avatar: Successfully saved Avatar '%s'", outputFile.data());
	}
	else
		CONSOLE_LOG(LogTypes::Error, "Resource Avatar: Could not save Avatar '%s'", outputFile.data());

	RELEASE_ARRAY(buffer);

	return retSize;
}

bool ResourceAvatar::LoadFile(const char* file, ResourceAvatarData& outputAvatarData)
{
	assert(file != nullptr);

	bool ret = false;

	char* buffer;
	uint size = App->fs->Load(file, &buffer);
	if (size > 0)
	{
		char* cursor = (char*)buffer;

		// 1. Load hips uuid
		uint bytes = sizeof(uint);
		memcpy(&outputAvatarData.hipsUuid, cursor, bytes);

		cursor += bytes;

		// 2. Load root uuid
		bytes = sizeof(uint);
		memcpy(&outputAvatarData.rootUuid, cursor, bytes);

		//cursor += bytes;

		CONSOLE_LOG(LogTypes::Normal, "Resource Avatar: Successfully loaded Avatar '%s'", file);
		RELEASE_ARRAY(buffer);
	}
	else
		CONSOLE_LOG(LogTypes::Error, "Resource Avatar: Could not load Avatar '%s'", file);

	return ret;
}

// Returns the last modification time of the file
uint ResourceAvatar::CreateMeta(const char* file, uint avatarUuid, const std::string& name, std::string& outputMetaFile)
{
	assert(file != nullptr);

	uint uuidsSize = 1;
	uint nameSize = DEFAULT_BUF_SIZE;

	// Name
	char materialName[DEFAULT_BUF_SIZE];
	strcpy_s(materialName, DEFAULT_BUF_SIZE, name.data());

	// --------------------------------------------------

	uint size =
		sizeof(int64_t) +
		sizeof(uint) +
		sizeof(uint) * uuidsSize +

		sizeof(uint) + // name size
		sizeof(char) * nameSize; // name

	char* data = new char[size];
	char* cursor = data;

	// 1. Store last modification time
	int64_t lastModTime = App->fs->GetLastModificationTime(file);
	assert(lastModTime > 0);
	uint bytes = sizeof(int64_t);
	memcpy(cursor, &lastModTime, bytes);

	cursor += bytes;

	// 2. Store uuids size
	bytes = sizeof(uint);
	memcpy(cursor, &uuidsSize, bytes);

	cursor += bytes;

	// 3. Store avatar uuid
	bytes = sizeof(uint) * uuidsSize;
	memcpy(cursor, &avatarUuid, bytes);

	cursor += bytes;

	// 4. Store avatar name size
	bytes = sizeof(uint);
	memcpy(cursor, &nameSize, bytes);

	cursor += bytes;

	// 5. Store avatar name
	bytes = sizeof(char) * nameSize;
	memcpy(cursor, materialName, bytes);

	// --------------------------------------------------

	// Build the path of the meta file and save it
	outputMetaFile = file;
	outputMetaFile.append(EXTENSION_META);
	uint retSize = App->fs->Save(outputMetaFile.data(), data, size);
	if (retSize > 0)
	{
		CONSOLE_LOG(LogTypes::Normal, "Resource Avatar: Successfully saved meta '%s'", outputMetaFile.data());
	}
	else
	{
		CONSOLE_LOG(LogTypes::Error, "Resource Avatar: Could not save meta '%s'", outputMetaFile.data());
		return 0;
	}

	return lastModTime;
}

bool ResourceAvatar::ReadMeta(const char* metaFile, int64_t& lastModTime, uint& avatarUuid, std::string& name)
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

		// 3. Load avatar uuid
		bytes = sizeof(uint) * uuidsSize;
		memcpy(&avatarUuid, cursor, bytes);

		cursor += bytes;

		// 4. Load avatar name size
		uint nameSize = 0;
		bytes = sizeof(uint);
		memcpy(&nameSize, cursor, bytes);
		assert(nameSize > 0);

		cursor += bytes;

		// 5. Load avatar name
		name.resize(nameSize);
		bytes = sizeof(char) * nameSize;
		memcpy(&name[0], cursor, bytes);

		CONSOLE_LOG(LogTypes::Normal, "Resource Material: Successfully loaded meta '%s'", metaFile);
		RELEASE_ARRAY(buffer);
	}
	else
	{
		CONSOLE_LOG(LogTypes::Error, "Resource Material: Could not load meta '%s'", metaFile);
		return false;
	}

	return true;
}

uint ResourceAvatar::SetNameToMeta(const char* metaFile, const std::string& name)
{
	assert(metaFile != nullptr);

	int64_t lastModTime = 0;
	uint avatarUuid = 0;
	std::string oldName;
	ReadMeta(metaFile, lastModTime, avatarUuid, oldName);

	uint uuidsSize = 1;
	uint nameSize = DEFAULT_BUF_SIZE;

	// Name
	char avatarName[DEFAULT_BUF_SIZE];
	strcpy_s(avatarName, DEFAULT_BUF_SIZE, name.data());

	// --------------------------------------------------

	uint size =
		sizeof(int64_t) +
		sizeof(uint) +
		sizeof(uint) * uuidsSize +

		sizeof(uint) + // name size
		sizeof(char) * nameSize; // name

	char* data = new char[size];
	char* cursor = data;

	// 1. Store last modification time
	uint bytes = sizeof(int64_t);
	memcpy(cursor, &lastModTime, bytes);

	cursor += bytes;

	// 2. Store uuids size
	bytes = sizeof(uint);
	memcpy(cursor, &uuidsSize, bytes);

	cursor += bytes;

	// 3. Store avatar uuid
	bytes = sizeof(uint) * uuidsSize;
	memcpy(cursor, &avatarUuid, bytes);

	cursor += bytes;

	// 4. Store avatar name size
	bytes = sizeof(uint);
	memcpy(cursor, &nameSize, bytes);

	cursor += bytes;

	// 5. Store avatar name
	bytes = sizeof(char) * nameSize;
	memcpy(cursor, avatarName, bytes);

	// --------------------------------------------------

	// Build the path of the meta file and save it
	uint retSize = App->fs->Save(metaFile, data, size);
	if (retSize > 0)
	{
		CONSOLE_LOG(LogTypes::Normal, "Resource Avatar: Successfully saved meta '%s'", metaFile);
	}
	else
	{
		CONSOLE_LOG(LogTypes::Error, "Resource Avatar: Could not save meta '%s'", metaFile);
		return 0;
	}

	return lastModTime;
}

bool ResourceAvatar::GenerateLibraryFiles() const
{
	assert(data.file.data() != nullptr);

	// Search for the meta associated to the file
	char metaFile[DEFAULT_BUF_SIZE];
	strcpy_s(metaFile, strlen(data.file.data()) + 1, data.file.data()); // file
	strcat_s(metaFile, strlen(metaFile) + strlen(EXTENSION_META) + 1, EXTENSION_META); // extension

	// 1. Copy meta
	if (App->fs->Exists(metaFile))
	{
		std::string outputFile;
		uint size = App->fs->Copy(metaFile, DIR_LIBRARY_AVATARS, outputFile);

		if (size > 0)
		{
			// 2. Copy avatar
			outputFile.clear();
			uint size = App->fs->Copy(data.file.data(), DIR_LIBRARY_AVATARS, outputFile);

			if (size > 0)
				return true;
		}
	}

	return false;
}

// ----------------------------------------------------------------------------------------------------

bool ResourceAvatar::SetHipsUuid(uint hipsUuid)
{
	if (hipsUuid == 0)
	{
		avatarData.hipsUuid = hipsUuid;

		// 1. Clear the skeleton and the bones
		ClearSkeletonAndBones();

		return true;
	}

	GameObject* skeleton = App->GOs->GetGameObjectByUID(hipsUuid);
	if (skeleton == nullptr)
	{
		//CONSOLE_LOG(LogTypes::Error, "The root bone does not exist...");
		return false;
	}

	avatarData.hipsUuid = hipsUuid;

	if (!IsInMemory())
		return true;

	// 1. Clear the skeleton and the bones
	ClearSkeletonAndBones();

	// 2. Create the skeleton
	CreateSkeleton(skeleton);

	// 3. Add the skeleton bones to the meshes
	GameObject* root = App->GOs->GetGameObjectByUID(avatarData.rootUuid);
	if (root == nullptr)
	{
		//CONSOLE_LOG(LogTypes::Error, "The root game object does not exist...");
		return false;
	}
	AddBones(root);

	return bones.size() > 0;
}

uint ResourceAvatar::GetHipsUuid() const
{
	return avatarData.hipsUuid;
}

bool ResourceAvatar::SetRootUuid(uint rootUuid)
{
	if (rootUuid == 0)
	{
		avatarData.rootUuid = rootUuid;

		// 1. Clear the bones
		ClearBones();

		return true;
	}

	GameObject* root = App->GOs->GetGameObjectByUID(rootUuid);
	if (root == nullptr)
	{
		//CONSOLE_LOG(LogTypes::Error, "The root game object does not exist...");
		return false;
	}

	avatarData.rootUuid = rootUuid;

	if (!IsInMemory())
		return true;

	// 1. Clear the bones
	ClearBones();

	// 3. Add the skeleton bones to the meshes
	AddBones(root);

	return bones.size() > 0;
}

uint ResourceAvatar::GetRootUuid() const
{
	return avatarData.rootUuid;
}

std::vector<uint> ResourceAvatar::GetBonesUuids() const
{
	std::vector<uint> bonesUuids;

	for (std::unordered_map<std::string, uint>::const_iterator it = bones.begin(); it != bones.end(); ++it)
		bonesUuids.push_back(it->second);

	return bonesUuids;
}

void ResourceAvatar::SetIsAnimated(bool animated)
{
	this->animated = animated;
}

bool ResourceAvatar::GetIsAnimated() const
{
	return animated;
}

// ----------------------------------------------------------------------------------------------------

void ResourceAvatar::CreateSkeleton(GameObject* gameObject)
{
	assert(gameObject != nullptr);

	std::vector<GameObject*> children;
	gameObject->GetChildrenVector(children);

	for (uint i = 0; i < children.size(); ++i)
	{
		/// Bone component
		ComponentBone* boneComponent = children[i]->cmp_bone;
		if (boneComponent == nullptr)
			continue;

		/// Bone resource
		ResourceBone* boneResource = (ResourceBone*)App->res->GetResource(boneComponent->res);
		assert(boneResource != nullptr);

		const char* boneName = boneResource->boneData.name.data();
		bones[boneName] = children[i]->GetUUID();

		//CONSOLE_LOG(LogTypes::Normal, "The bone %s has been loaded correctly", boneName);
	}
}

void ResourceAvatar::AddBones(GameObject* gameObject) const
{
	assert(gameObject != nullptr);

	std::vector<GameObject*> children;
	gameObject->GetChildrenVector(children);

	for (uint i = 0; i < children.size(); ++i)
	{
		ComponentMesh* meshComponent = children[i]->cmp_mesh;
		if (meshComponent != nullptr)
		{
			meshComponent->avatarResource = uuid;

			ResourceMesh* meshResource = (ResourceMesh*)App->res->GetResource(meshComponent->res);
			if (meshResource != nullptr)
				meshResource->AddBones(bones);
		}
	}
}

void ResourceAvatar::CreateSkeletonAndAddBones()
{
	assert(avatarData.hipsUuid > 0 && avatarData.rootUuid > 0);

	// 1. Create the skeleton
	GameObject* skeleton = App->GOs->GetGameObjectByUID(avatarData.hipsUuid);
	if (skeleton == nullptr)
	{
		//CONSOLE_LOG(LogTypes::Error, "The root bone does not exist...");
		return;
	}
	CreateSkeleton(skeleton);

	// 2. Add the skeleton bones to the meshes
	GameObject* root = App->GOs->GetGameObjectByUID(avatarData.rootUuid);
	if (root == nullptr)
	{
		//CONSOLE_LOG(LogTypes::Error, "The root game object does not exist...");
		return;
	}
	AddBones(root);
}

void ResourceAvatar::StepBones(uint animationUuid, float time, float blend)
{
	if (bones.empty())
		return;

	ResourceAnimation* animationResource = (ResourceAnimation*)App->res->GetResource(animationUuid);
	if (animationResource == nullptr)
	{
		//CONSOLE_LOG(LogTypes::Error, "The animation that needs to be stepped does not exist...");
		return;
	}

	// Step all bones
	for (uint i = 0; i < animationResource->animationData.numKeys; ++i)
	{
		const char* boneName = animationResource->animationData.boneKeys[i].bone_name.data();
		std::unordered_map<std::string, uint>::const_iterator it = bones.find(boneName);
		if (it == bones.end())
			continue;

		uint boneGameObjectUuid = it->second;

		GameObject* boneGameObject = App->GOs->GetGameObjectByUID(boneGameObjectUuid);
		if (boneGameObject == nullptr)
			continue;

		// ----------

		math::float3 pos = boneGameObject->transform->GetPosition();
		math::float3 scale = boneGameObject->transform->GetScale();
		math::Quat rot = boneGameObject->transform->GetRotation();

		// 1. Find next and previous transformations

		float nextTime = 0.0f;
		float prevTime = 0.0f;

		/// a) Positions
		float* prevPos = nullptr;
		float* nextPos = nullptr;
		float timePos = 0.0f;

		if (animationResource->animationData.boneKeys[i].positions.count == 1)
		{
			// Save next and prev pos
			nextPos = prevPos = &animationResource->animationData.boneKeys[i].positions.value[0];

			// Does not need interpolation
		}
		else
		{
			for (uint j = 0; j < animationResource->animationData.boneKeys[i].positions.count; ++j)
			{
				if (time == animationResource->animationData.boneKeys[i].positions.time[j])
				{
					// Save next and prev pos
					nextPos = prevPos = &animationResource->animationData.boneKeys[i].positions.value[j * 3];

					// Does not need interpolation

					break;
				}
				else if (animationResource->animationData.boneKeys[i].positions.time[j] > time)
				{
					// Save next and prev time and pos
					nextTime = animationResource->animationData.boneKeys[i].positions.time[j];
					nextPos = &animationResource->animationData.boneKeys[i].positions.value[j * 3];

					if (j == 0)
					{
						prevTime = animationResource->animationData.boneKeys[i].positions.time[animationResource->animationData.boneKeys[i].positions.count - 1];
						prevPos = &animationResource->animationData.boneKeys[i].positions.value[(animationResource->animationData.boneKeys[i].positions.count - 1) * 3];
					}
					else
					{
						prevTime = animationResource->animationData.boneKeys[i].positions.time[j - 1];
						prevPos = &animationResource->animationData.boneKeys[i].positions.value[(j * 3) - 3];
					}
					
					// Needs interpolation
					timePos = (time - prevTime) / (nextTime - prevTime);

					break;
				}
			}
		}

		/// b) Scalings
		float* prevScale = nullptr;
		float* nextScale = nullptr;
		float timeScale = 0.0f;

		if (animationResource->animationData.boneKeys[i].scalings.count == 1)
		{
			// Save next and prev scale
			nextScale = prevScale = &animationResource->animationData.boneKeys[i].scalings.value[0];

			// Does not need interpolation
		}
		else
		{
			for (uint j = 0; j < animationResource->animationData.boneKeys[i].scalings.count; ++j)
			{
				if (time == animationResource->animationData.boneKeys[i].scalings.time[j])
				{
					// Save next and prev scale
					nextScale = prevScale = &animationResource->animationData.boneKeys[i].scalings.value[j * 3];

					// Does not need interpolation

					break;
				}
				else if (animationResource->animationData.boneKeys[i].scalings.time[j] > time)
				{
					// Save next and prev time and scale
					nextTime = animationResource->animationData.boneKeys[i].scalings.time[j];
					nextScale = &animationResource->animationData.boneKeys[i].scalings.value[j * 3];

					if (j == 0)
					{
						prevTime = animationResource->animationData.boneKeys[i].scalings.time[animationResource->animationData.boneKeys[i].scalings.count - 1];
						prevScale = &animationResource->animationData.boneKeys[i].scalings.value[(animationResource->animationData.boneKeys[i].scalings.count - 1) * 3];
					}
					else
					{
						prevTime = animationResource->animationData.boneKeys[i].scalings.time[j - 1];
						prevScale = &animationResource->animationData.boneKeys[i].scalings.value[(j * 3) - 3];
					}

					// Needs interpolation
					timeScale = (time - prevTime) / (nextTime - prevTime);

					break;
				}
			}
		}

		/// c) Rotations
		float* prevRot = nullptr;
		float* nextRot = nullptr;
		float timeRot = 0.0f;

		if (animationResource->animationData.boneKeys[i].rotations.count == 1)
		{
			// Save next and prev scale
			nextRot = prevRot = &animationResource->animationData.boneKeys[i].rotations.value[0];

			// Does not need interpolation
		}
		else
		{
			for (uint j = 0; j < animationResource->animationData.boneKeys[i].rotations.count; ++j)
			{
				if (time == animationResource->animationData.boneKeys[i].rotations.time[j])
				{
					// Save next and prev scale
					nextRot = prevRot = &animationResource->animationData.boneKeys[i].rotations.value[j * 4];

					// Does not need interpolation

					break;
				}
				else if (animationResource->animationData.boneKeys[i].rotations.time[j] > time)
				{
					// Save next and prev time and scale
					nextTime = animationResource->animationData.boneKeys[i].rotations.time[j];
					nextRot = &animationResource->animationData.boneKeys[i].rotations.value[j * 4];

					if (j == 0)
					{
						prevTime = animationResource->animationData.boneKeys[i].rotations.time[animationResource->animationData.boneKeys[i].rotations.count - 1];
						prevRot = &animationResource->animationData.boneKeys[i].rotations.value[(animationResource->animationData.boneKeys[i].rotations.count - 1) * 4];
					}
					else
					{
						prevTime = animationResource->animationData.boneKeys[i].rotations.time[j - 1];
						prevRot = &animationResource->animationData.boneKeys[i].rotations.value[(j * 4) - 4];
					}

					// Needs interpolation
					timeRot = (time - prevTime) / (nextTime - prevTime);

					break;
				}
			}
		}

		// 2. Interpolate (or not)

		/// a) Positions
		if (animationResource->animationData.interpolate && prevPos != nextPos
			&& prevPos != nullptr && nextPos != nullptr)
		{
			math::float3 prevPosLerp(prevPos[0], prevPos[1], prevPos[2]);
			math::float3 nextPosLerp(nextPos[0], nextPos[1], nextPos[2]);
			pos = math::float3::Lerp(prevPosLerp, nextPosLerp, timePos);
		}
		else if ((!animationResource->animationData.interpolate || prevPos == nextPos)
			&& prevPos != nullptr)
			pos = math::float3(prevPos[0], prevPos[1], prevPos[2]);

		/// b) Scalings
		if (animationResource->animationData.interpolate && prevScale != nextScale
			&& prevScale != nullptr && nextScale != nullptr)
		{
			math::float3 prevScaleLerp(prevScale[0], prevScale[1], prevScale[2]);
			math::float3 nextScaleLerp(nextScale[0], nextScale[1], nextScale[2]);
			scale = math::float3::Lerp(prevScaleLerp, nextScaleLerp, timeScale);
		}
		else if ((!animationResource->animationData.interpolate || prevScale == nextScale)
			&& prevScale != nullptr)
			scale = math::float3(prevScale[0], prevScale[1], prevScale[2]);

		/// c) Rotations
		if (animationResource->animationData.interpolate && prevRot != nextRot
			&& prevRot != nullptr && nextRot != nullptr)
		{
			math::Quat prevRotSlerp(prevRot[0], prevRot[1], prevRot[2], prevRot[3]);
			math::Quat nextRotSlerp(nextRot[0], nextRot[1], nextRot[2], nextRot[3]);
			rot = math::Quat::Slerp(prevRotSlerp, nextRotSlerp, timeRot);
		}
		else if ((!animationResource->animationData.interpolate || prevRot == nextRot)
			&& prevRot != nullptr)
			rot = math::Quat(prevRot[0], prevRot[1], prevRot[2], prevRot[3]);

		// 3. Blending between two animations

		if (blend == 1.0f)
		{
			// Not blend
			boneGameObject->transform->SetPosition(pos);
			boneGameObject->transform->SetScale(scale);
			boneGameObject->transform->SetRotation(rot);
		}
		else
		{
			// Blend
			math::float3 blendPos = math::float3::Lerp(boneGameObject->transform->GetPosition(), pos, blend);
			math::float3 blendScale = math::float3::Lerp(boneGameObject->transform->GetScale(), scale, blend);
			math::Quat blendRot = math::Quat::Slerp(boneGameObject->transform->GetRotation(), rot, blend);

			boneGameObject->transform->SetPosition(blendPos);
			boneGameObject->transform->SetScale(blendScale);
			boneGameObject->transform->SetRotation(blendRot);
		}
	}
}

// ----------------------------------------------------------------------------------------------------

void ResourceAvatar::ClearSkeleton()
{
	bones.clear();
}

void ResourceAvatar::ClearBones()
{
	GameObject* root = App->GOs->GetGameObjectByUID(avatarData.hipsUuid);
	if (root == nullptr)
	{
		//CONSOLE_LOG(LogTypes::Error, "The root bone does not exist...");
		return;
	}

	std::vector<GameObject*> children;
	root->GetChildrenVector(children);

	for (uint i = 0; i < children.size(); ++i)
	{
		ComponentMesh* meshComponent = children[i]->cmp_mesh;
		
		if (meshComponent != nullptr)
		{
			if (meshComponent->avatarResource == uuid)
				meshComponent->avatarResource = 0;
		}
	}
}

void ResourceAvatar::ClearSkeletonAndBones()
{
	ClearSkeleton();
	ClearBones();
}

bool ResourceAvatar::LoadInMemory()
{
	// Create the skeleton and the bones
	CreateSkeletonAndAddBones();

	return true;
}

bool ResourceAvatar::UnloadFromMemory()
{
	// Clear the skeleton and the bones
	ClearSkeletonAndBones();

	return true;
}