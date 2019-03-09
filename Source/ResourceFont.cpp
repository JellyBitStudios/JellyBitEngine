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
	char* buffer = nullptr;
	uint size = 0u;
	size = App->fs->Load(data.file, &buffer);

	if (size <= 0 && FT_New_Memory_Face(library, (FT_Byte*)buffer, size, 0, &face))
	{
		CONSOLE_LOG(LogTypes::Error, "The font file couldn't be opened, read or this format is unsupported");
	}

	else
	{
		FT_Set_Pixel_Sizes(face, 0, 20);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction

		for (uint c = 0; c < 128; c++)
		{
			// Load character glyph 
			if (FT_Load_Char(face, c, FT_LOAD_RENDER))
			{
				CONSOLE_LOG(LogTypes::Error, "Failed to load Glyph from Freetype");
				continue;
			}
			// Generate texture
			GLuint texture;
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RGB, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);

			// Set texture options
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			// Now store character for later use
			CharacterData character = {
				texture,
				math::float2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
				math::float2(face->glyph->bitmap_left, face->glyph->bitmap_top),
				face->glyph->advance.x
			};
			charactersMap.insert(std::pair<char, CharacterData>(c, character));
		}
		FT_Done_Face(face);
		FT_Done_FreeType(library);
	}
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

