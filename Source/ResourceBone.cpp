#include "ResourceBone.h"

#include "Application.h"
#include "ModuleFileSystem.h"
#include "ModuleScene.h"

#include "imgui/imgui.h"

#include <assert.h>

ResourceBone::ResourceBone(ResourceTypes type, uint uuid, ResourceData data, ResourceBoneData boneData) : Resource(type, uuid, data), boneData(boneData) {}

ResourceBone::~ResourceBone() {}

void ResourceBone::OnPanelAssets()
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
		ImGui::SetDragDropPayload("BONE_INSPECTOR_SELECTOR", &uuid, sizeof(uint));
		ImGui::EndDragDropSource();
	}
#endif
}

bool ResourceBone::ExportFile(const ResourceData& data, const ResourceBoneData& boneData, std::string& outputFile, bool overwrite)
{
	return SaveFile(data, boneData, outputFile, overwrite) > 0;
}

uint ResourceBone::SaveFile(const ResourceData& data, const ResourceBoneData& boneData, std::string& outputFile, bool overwrite)
{
	uint nameSize = DEFAULT_BUF_SIZE;

	// Name
	char boneName[DEFAULT_BUF_SIZE];
	strcpy_s(boneName, DEFAULT_BUF_SIZE, boneData.name.data());

	// --------------------------------------------------

	uint size =
		sizeof(math::float4x4) +
		sizeof(float) * MAX_WEIGHTS +
		sizeof(uint) * MAX_WEIGHTS +
		sizeof(uint) +					// name size
		sizeof(char) * nameSize;		// name

	char* buffer = new char[size];
	char* cursor = buffer;

	// 1. Store offset matrix
	uint bytes = sizeof(math::float4x4);
	memcpy(cursor, &boneData.offsetMatrix, bytes);

	cursor += bytes;

	// 2. Store weights
	bytes = sizeof(float) * MAX_WEIGHTS;
	memcpy(cursor, &boneData.weights, bytes);

	cursor += bytes;

	// 3. Store indices
	bytes = sizeof(uint) * MAX_WEIGHTS;
	memcpy(cursor, &boneData.indices, bytes);

	cursor += bytes;

	// 4. Store name size
	bytes = sizeof(uint);
	memcpy(cursor, &nameSize, bytes);

	cursor += bytes;

	// 5. Store name
	bytes = sizeof(char) * nameSize;
	memcpy(cursor, &boneName, bytes);

	//cursor += bytes;

	// --------------------------------------------------

	// Build the path of the file
	if (overwrite)
		outputFile = data.file;
	else
		outputFile = data.name;

	// Save the file
	uint retSize = App->fs->SaveInGame(buffer, size, FileTypes::BoneFile, outputFile, overwrite) > 0;

	if (retSize > 0)
	{
		CONSOLE_LOG(LogTypes::Normal, "Resource Bone: Successfully saved Bone '%s'", outputFile.data());
	}
	else
		CONSOLE_LOG(LogTypes::Error, "Resource Bone: Could not save Bone '%s'", outputFile.data());

	RELEASE_ARRAY(buffer);

	return retSize;
}

bool ResourceBone::LoadFile(const char* file, ResourceBoneData& outputBoneData)
{
	assert(file != nullptr);

	bool ret = false;

	char* buffer;
	uint size = App->fs->Load(file, &buffer);
	if (size > 0)
	{
		char* cursor = (char*)buffer;

		// 1. Load offset matrix
		uint bytes = sizeof(math::float4x4);
		memcpy(&outputBoneData.offsetMatrix, cursor, bytes);

		cursor += bytes;

		// 2. Load weights
		bytes = sizeof(float) * MAX_WEIGHTS;
		memcpy(&outputBoneData.weights, cursor, bytes);

		cursor += bytes;

		// 3. Load indices
		bytes = sizeof(uint) * MAX_WEIGHTS;
		memcpy(&outputBoneData.indices, cursor, bytes);

		cursor += bytes;

		// 4. Load name size
		bytes = sizeof(uint);
		uint nameSize = 0;
		memcpy(&nameSize, cursor, bytes);
		assert(nameSize > 0);

		cursor += bytes;

		// 5. Load name
		bytes = sizeof(char) * nameSize;
		memcpy(&outputBoneData.name[0], cursor, bytes);

		//cursor += bytes;

		CONSOLE_LOG(LogTypes::Normal, "Resource Bone: Successfully loaded Bone '%s'", file);
		RELEASE_ARRAY(buffer);
	}
	else
		CONSOLE_LOG(LogTypes::Error, "Resource Bone: Could not load Bone '%s'", file);

	return ret;
}

// ----------------------------------------------------------------------------------------------------

bool ResourceBone::LoadInMemory()
{
	return true;
}

bool ResourceBone::UnloadFromMemory()
{
	return true;
}