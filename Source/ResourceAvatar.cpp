#include "ResourceAvatar.h"

#include "Application.h"
#include "ModuleFileSystem.h"
#include "ModuleScene.h"

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

// Returns the last modification time of the file
uint ResourceAvatar::CreateMeta(const char* file, uint avatarUuid, std::string& name, std::string& outputMetaFile)
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
	uint resultSize = App->fs->Save(outputMetaFile.data(), data, size);
	if (resultSize > 0)
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

void ResourceAvatar::StepAnimation(uint animationUuid, float time, float blendTime) const
{
	// TODO
}

// ----------------------------------------------------------------------------------------------------

bool ResourceAvatar::LoadInMemory()
{
	assert(avatarData.hipsUuid > 0);

	return true;
}

bool ResourceAvatar::UnloadFromMemory()
{
	return true;
}