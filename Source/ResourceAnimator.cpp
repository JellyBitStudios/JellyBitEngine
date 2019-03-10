#include "ResourceAnimator.h"
#include "imgui/imgui.h"
#include "ModuleScene.h"
#include "ModuleFileSystem.h"
#include "Application.h"

#include <assert.h>

ResourceAnimator::ResourceAnimator(ResourceTypes type, uint uuid, ResourceData data, ResourceAnimatorData animator_data) : Resource(type, uuid, data), animator_data(animator_data)
{}

ResourceAnimator::~ResourceAnimator()
{
}

bool ResourceAnimator::LoadInMemory()
{
	return true;
}

bool ResourceAnimator::UnloadFromMemory()
{
	return true;
}

void ResourceAnimator::OnPanelAssets()
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
		ImGui::SetDragDropPayload("ANIMATOR_RESOURCE", &uuid, sizeof(uint));
		ImGui::EndDragDropSource();
	}
#endif // !GAMEMODE
}

bool ResourceAnimator::ImportFile(const char* file, std::string& name, std::string& outputFile)
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
		bool result = ResourceAnimator::ReadMeta(metaFile, lastModTime, uuid, name);
		assert(result);

		// The uuid of the resource would be the entry
		char entry[DEFAULT_BUF_SIZE];
		sprintf_s(entry, "%u", uuid);
		outputFile = entry;
	}

	return true;
}

bool ResourceAnimator::ExportFile(ResourceData& resourceData, ResourceAnimatorData& animData, std::string& outputFile, bool overwrite)
{
	bool ret = true;

	

	return ret;
}

uint ResourceAnimator::CreateMeta(const char* file, uint animatorUuid, std::string& name, std::string& outputMetaFile)
{
	assert(file != nullptr);

	uint uuidsSize = 1;
	uint nameSize = DEFAULT_BUF_SIZE;

	// Name
	char animator_name[DEFAULT_BUF_SIZE];
	strcpy_s(animator_name, DEFAULT_BUF_SIZE, name.data());

	// --------------------------------------------------

	uint size =
		sizeof(int64_t) +
		sizeof(uint) +
		sizeof(uint) * uuidsSize +
		sizeof(uint) + // name size
		sizeof(char) * nameSize;

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

	// 3. Store animator uuid
	bytes = sizeof(uint) * uuidsSize;
	memcpy(cursor, &animatorUuid, bytes);

	cursor += bytes;

	// 4. Store animator name size
	bytes = sizeof(uint);
	memcpy(cursor, &nameSize, bytes);

	cursor += bytes;

	// 5. Store animator name
	bytes = sizeof(char) * nameSize;
	memcpy(cursor, animator_name, bytes);

	cursor += bytes;



	// --------------------------------------------------

	// Build the path of the meta file and save it
	outputMetaFile = file;
	outputMetaFile.append(EXTENSION_META);
	uint resultSize = App->fs->Save(outputMetaFile.data(), data, size);
	if (resultSize > 0)
	{
		CONSOLE_LOG(LogTypes::Normal, "Resource Animator: Successfully saved meta '%s'", outputMetaFile.data());
	}
	else
	{
		CONSOLE_LOG(LogTypes::Error, "Resource Animator: Could not save meta '%s'", outputMetaFile.data());
		return 0;
	}

	return lastModTime;
}

bool ResourceAnimator::ReadMeta(const char* metaFile, int64_t& lastModTime, uint& animatorUuid, std::string& name)
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

		// 3. Load animation uuid
		bytes = sizeof(uint) * uuidsSize;
		memcpy(&animatorUuid, cursor, bytes);

		cursor += bytes;

		// 4. Load animation name size
		uint nameSize = 0;
		bytes = sizeof(uint);
		memcpy(&nameSize, cursor, bytes);
		assert(nameSize > 0);

		cursor += bytes;

		// 5. Load animation name
		name.resize(nameSize);
		bytes = sizeof(char) * nameSize;
		memcpy(&name[0], cursor, bytes);

		CONSOLE_LOG(LogTypes::Normal, "Resource Animator: Successfully loaded meta '%s'", metaFile);
		RELEASE_ARRAY(buffer);
	}
	else
	{
		CONSOLE_LOG(LogTypes::Error, "Resource Animator: Could not load meta '%s'", metaFile);
		return false;
	}

	return true;
}

bool ResourceAnimator::LoadFile(const char* file, ResourceAnimatorData& outputAnimationData)
{
	assert(file != nullptr);

	bool ret = true;

	return ret;
}

bool ResourceAnimator::GenerateLibraryFiles() const
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
		uint size = App->fs->Copy(metaFile, DIR_LIBRARY_ANIMATORS, outputFile);

		if (size > 0)
		{
			// 2. Copy Animator
			outputFile.clear();
			uint size = App->fs->Copy(data.file.data(), DIR_LIBRARY_ANIMATORS, outputFile);

			if (size > 0)
				return true;
		}
	}

	return false;
}

uint ResourceAnimator::SetNameToMeta(const char* metaFile, const std::string& name)
{
	assert(metaFile != nullptr);

	int64_t lastModTime = 0;
	uint materialUuid = 0;
	std::string oldName;
	ReadMeta(metaFile, lastModTime, materialUuid, oldName);

	uint uuidsSize = 1;
	uint nameSize = DEFAULT_BUF_SIZE;

	// Name
	char materialName[DEFAULT_BUF_SIZE];
	strcpy_s(materialName, DEFAULT_BUF_SIZE, name.data());

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

	// 3. Store animator uuid
	bytes = sizeof(uint) * uuidsSize;
	memcpy(cursor, &materialUuid, bytes);

	cursor += bytes;

	// 4. Store animator name size
	bytes = sizeof(uint);
	memcpy(cursor, &nameSize, bytes);

	cursor += bytes;

	// 5. Store animator name
	bytes = sizeof(char) * nameSize;
	memcpy(cursor, materialName, bytes);
	cursor += bytes;

	// --------------------------------------------------

	// Build the path of the meta file and save it
	uint retSize = App->fs->Save(metaFile, data, size);
	if (retSize > 0)
	{
		CONSOLE_LOG(LogTypes::Normal, "Resource Material: Successfully saved meta '%s'", metaFile);
	}
	else
	{
		CONSOLE_LOG(LogTypes::Error, "Resource Material: Could not save meta '%s'", metaFile);
		return 0;
	}

	return lastModTime;
}