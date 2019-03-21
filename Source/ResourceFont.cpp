#include "ResourceFont.h"
#include "imgui/imgui.h"
#include "Application.h"
#include "glew/include/GL/glew.h"
#include "ModuleScene.h"
#include "ModuleFileSystem.h"

#include <assert.h>
#pragma comment(lib, "Freetype/libx86/freetype.lib")

ResourceFont::ResourceFont(ResourceTypes type, uint uuid, ResourceData data, FontData fontData) : Resource(type, uuid, data), fontData(fontData) 
{
}

ResourceFont::~ResourceFont()
{
}

bool ResourceFont::LoadInMemory()
{
	return true;
}

bool ResourceFont::UnloadFromMemory()
{
	return true;
}

void ResourceFont::OnPanelAssets()
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

	if (ImGui::IsMouseReleased(0) && ImGui::IsItemHovered())
	{
		SELECT(this);
	}

	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
	{
		ImGui::SetDragDropPayload("FONT_RESOURCE", &uuid, sizeof(uint));
		ImGui::EndDragDropSource();
	}
#endif 
}


bool ResourceFont::ImportFile(const char * file, std::string & name, std::string & outputFile)
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
		bool result = ResourceFont::ReadMeta(metaFile, lastModTime, uuid, name);
		assert(result);

		// The uuid of the resource would be the entry
		char entry[DEFAULT_BUF_SIZE];
		sprintf_s(entry, "%u", uuid);
		outputFile = entry;
	}
	return true;
}

bool ResourceFont::ExportFile(ResourceData & data, FontData & fontData, std::string & outputFile, bool overwrite)
{
	SaveFile(data, fontData, outputFile, overwrite);
	return true;
}


uint ResourceFont::SaveFile(ResourceData& data, FontData& fontData, std::string& outputFile, bool overwrite)
{
	bool ret = false;

	return ret;
}

bool ResourceFont::LoadFile(const char * file, FontData & prefab_data_output)
{
	return false;
}

uint ResourceFont::CreateMeta(const char * file, uint prefab_uuid, std::string & name, std::string & outputMetaFile)
{
	assert(file != nullptr);

	uint uuidsSize = 1;
	uint nameSize = DEFAULT_BUF_SIZE;

	// Name
	char FontName[DEFAULT_BUF_SIZE];
	strcpy_s(FontName, DEFAULT_BUF_SIZE, name.data());

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







	// Build the path of the meta file and save it
	outputMetaFile = file;
	outputMetaFile.append(EXTENSION_META);
	uint retSize = App->fs->Save(outputMetaFile.data(), data, size);
	if (retSize > 0)
	{
		CONSOLE_LOG(LogTypes::Normal, "Resource Font: Successfully saved meta '%s'", outputMetaFile.data());
	}
	else
	{
		CONSOLE_LOG(LogTypes::Error, "Resource Font: Could not save meta '%s'", outputMetaFile.data());
		return 0;
	}

	return lastModTime;
}

bool ResourceFont::ReadMeta(const char * metaFile, int64_t & lastModTime, uint & prefab_uuid, std::string & name)
{
	assert(metaFile != nullptr);

	char* buffer;
	uint size = App->fs->Load(metaFile, &buffer);
	if (size > 0)
	{
		char* cursor = (char*)buffer;

		// 1. Load last modification time

		// 2. Load uuids size

		// 3. Load material uuid

		// 4. Load material name size


		// 5. Load material name


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


