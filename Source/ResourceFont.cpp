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

	uint size =	sizeof(uint) + sizeof(uint) +
		sizeof(Character) * fontData.charactersMap.size();

	char* buffer = new char[size];
	char* cursor = buffer;

	uint bytes = sizeof(uint);
	memcpy(cursor, &fontData.fontSize, bytes);
	cursor += bytes;

	uint listSize = fontData.charactersMap.size();
	memcpy(cursor, &listSize, bytes);
	cursor += bytes;

	bytes = sizeof(Character);
	for (std::map<char, Character>::iterator it = fontData.charactersMap.begin(); it != fontData.charactersMap.end(); ++it)
	{
		memcpy(cursor, &(*it).second, bytes);
		cursor += bytes;
	}
	// --------------------------------------------------

	// Build the path of the file
	if (overwrite)
		outputFile = data.file;
	else
		outputFile = data.name;

	bool ret;
	// Save the file
	ret = App->fs->SaveInGame(buffer, size, FileTypes::MaterialFile, outputFile, overwrite) > 0;

	if (ret)
	{
		CONSOLE_LOG(LogTypes::Normal, "Resource Material: Successfully saved Material '%s'", outputFile.data());
	}
	else
		CONSOLE_LOG(LogTypes::Error, "Resource Material: Could not save Material '%s'", outputFile.data());

	RELEASE_ARRAY(buffer);

	return ret;
}

bool ResourceFont::LoadFile(const char * file, FontData & fontData)
{
	assert(file != nullptr);

	bool ret = false;

	char* buffer;
	uint size = App->fs->Load(file, &buffer);
	if (size > 0)
	{
		char* cursor = (char*)buffer;

		uint bytes = sizeof(uint);
		memcpy(&fontData.fontSize, cursor, bytes);
		cursor += bytes;

		uint charactersSize = 0;
		bytes = sizeof(uint);
		memcpy(&charactersSize, cursor, bytes);
		cursor += bytes;

		bytes = sizeof(Character);
		for (uint i = 0; i < charactersSize; ++i)
		{
			Character character;
			memcpy(&character, cursor, bytes);
			fontData.charactersMap.insert(std::pair<char, Character>(i + 32, character));

			if (i < charactersSize - 1)
				cursor += bytes;
		}

		CONSOLE_LOG(LogTypes::Normal, "Resource Material: Successfully loaded Material '%s'", file);
		RELEASE_ARRAY(buffer);
	}
	else
		CONSOLE_LOG(LogTypes::Error, "Resource Material: Could not load Material '%s'", file);

	return ret;
}

uint ResourceFont::CreateMeta(const char * file, uint fontUuid, std::string & name, std::string & outputMetaFile)
{
	assert(file != nullptr);

	uint uuidsSize = 1;
	uint nameSize = DEFAULT_BUF_SIZE;

	// Name
	char fontName[DEFAULT_BUF_SIZE];
	strcpy_s(fontName, DEFAULT_BUF_SIZE, name.data());

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

	// 3. Store material uuid
	bytes = sizeof(uint) * uuidsSize;
	memcpy(cursor, &fontUuid, bytes);

	cursor += bytes;

	// 4. Store material name size
	bytes = sizeof(uint);
	memcpy(cursor, &nameSize, bytes);

	cursor += bytes;

	// 5. Store material name
	bytes = sizeof(char) * nameSize;
	memcpy(cursor, fontName, bytes);

	// --------------------------------------------------
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

bool ResourceFont::ReadMeta(const char * metaFile, int64_t & lastModTime, uint & fontUuid, std::string & name)
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

		// 3. Load material uuid
		bytes = sizeof(uint) * uuidsSize;
		memcpy(&fontUuid, cursor, bytes);

		cursor += bytes;

		// 4. Load material name size
		uint nameSize = 0;
		bytes = sizeof(uint);
		memcpy(&nameSize, cursor, bytes);
		assert(nameSize > 0);

		cursor += bytes;

		// 5. Load material name
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


