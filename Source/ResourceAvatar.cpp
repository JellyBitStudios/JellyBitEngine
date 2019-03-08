#include "ResourceAvatar.h"

#include "Application.h"
#include "ModuleFileSystem.h"
#include "ModuleGOs.h"
#include "ModuleResourceManager.h"
#include "ModuleScene.h"

#include "ComponentBone.h"
#include "ResourceBone.h"
#include "ResourceAnimation.h"

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
		sizeof(uint); // hips uuid

	char* buffer = new char[size];
	char* cursor = buffer;

	// 1. Store hips uuid
	uint bytes = sizeof(uint);
	memcpy(cursor, &avatarData.hipsUuid, bytes);

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
		// Read the info of the meta
		char* buffer;
		uint size = App->fs->Load(metaFile, &buffer);
		if (size > 0)
		{
			// Create a new name for the meta
			std::string extension = metaFile;

			uint found = extension.find_first_of(".");
			if (found != std::string::npos)
				extension = extension.substr(found, extension.size());

			char newMetaFile[DEFAULT_BUF_SIZE];
			sprintf_s(newMetaFile, "%s/%u%s", DIR_LIBRARY_TEXTURES, uuid, extension.data());

			// Save the new meta (info + new name)
			size = App->fs->Save(newMetaFile, buffer, size);
			if (size > 0)
				return true;

			RELEASE_ARRAY(buffer);
		}
	}

	return false;
}

// ----------------------------------------------------------------------------------------------------

uint ResourceAvatar::GetHipsUuid() const
{
	return avatarData.hipsUuid;
}

// ----------------------------------------------------------------------------------------------------

void ResourceAvatar::StepAnimation(uint animationUuid, float time, float blendTime)
{
	ResourceAnimation* animationResource = (ResourceAnimation*)App->res->GetResource(animationUuid);
	if (animationResource == nullptr)
	{
		CONSOLE_LOG(LogTypes::Error, "The animation that needs to be stepped does not exist...");
		return;
	}

	for (uint i = 0; i < animationResource->animationData.numKeys; ++i)
	{
		// Transformation to step the bone with
		BoneTransformation boneTransformation = animationResource->animationData.boneKeys[i];

		// Bone to be stepped with the transformation
		ResourceBone* boneResource = (ResourceBone*)App->res->GetResource(bones[boneTransformation.bone_name.c_str()]);
		assert(boneResource != nullptr);

		// ----------


	}



}

// ----------------------------------------------------------------------------------------------------

bool ResourceAvatar::LoadInMemory()
{
	assert(avatarData.hipsUuid > 0);

	// Hips
	GameObject* boneGameObject = App->GOs->GetGameObjectByUID(avatarData.hipsUuid);
	if (boneGameObject == nullptr)
	{
		CONSOLE_LOG(LogTypes::Error, "The root bone does not exist...");
		return false;
	}

	// Skeleton
	std::vector<GameObject*> boneGameObjects;
	boneGameObject->GetChildrenVector(boneGameObjects);

	for (uint i = 0; i < boneGameObjects.size(); ++i)
	{
		ComponentBone* boneComponent = boneGameObjects[i]->cmp_bone;
		if (boneComponent == nullptr)
		{
			CONSOLE_LOG(LogTypes::Error, "A bone does not exist...");
			continue;
		}

		ResourceBone* boneResource = (ResourceBone*)App->res->GetResource(boneComponent->res);
		assert(boneResource != nullptr);

		const char* boneName = boneResource->boneData.name.data();
		CONSOLE_LOG(LogTypes::Normal, "The bone %s has been loaded correctly", boneName);
		bones[boneName] = boneResource->GetUuid();
	}

	return bones.size() > 0;
}

bool ResourceAvatar::UnloadFromMemory()
{
	bones.clear();

	return true;
}