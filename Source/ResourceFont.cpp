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

bool ResourceFont::ExportFile(ResourceData & data, FontData & bone_data, std::string & outputFile, bool overwrite)
{
	return true;
		//App->boneImporter->SaveBone(data, bone_data, outputFile, overwrite);
}

uint ResourceFont::CreateMeta(const char * file, uint prefab_uuid, std::string & name, std::string & outputMetaFile)
{
	return uint();
}

bool ResourceFont::ReadMeta(const char * metaFile, int64_t & lastModTime, uint & prefab_uuid, std::string & name)
{
	return false;
}

bool ResourceFont::LoadFile(const char * file, FontData & prefab_data_output)
{
	return false;
}

