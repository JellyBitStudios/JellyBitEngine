#include "ResourceFont.h"
#include "imgui/imgui.h"
#include "Application.h"
#include "glew/include/GL/glew.h"
#include "ModuleScene.h"
#include "ModuleFileSystem.h"
#include "ModuleResourceManager.h"
#include "ModuleUi.h"

#include <assert.h>
#pragma comment(lib, "Freetype/libx86/freetype.lib")

ResourceFont::ResourceFont(uint uuid, ResourceData data, ResourceFontData fontData) : Resource(ResourceTypes::FontResource, uuid, data), fontData(fontData) 
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

/*void ResourceFont::OYECHAVALES_HANCAMBIADOESTEMETA()
{

}*/

Resource* ResourceFont::ImportFile(const char * file)
{
	assert(file != nullptr);

	if (!App->fs->Exists(file))
		return nullptr;

	// Search for the meta associated to the file
	char metaFile[DEFAULT_BUF_SIZE];
	strcpy_s(metaFile, strlen(file) + 1, file); // file
	strcat_s(metaFile, strlen(metaFile) + strlen(EXTENSION_META) + 1, EXTENSION_META); // extension

	//Si existe el meta tenemos que buscar todos los sizes en library i exportar los que falten
	if (App->fs->Exists(metaFile))
	{
		// Read the meta
		std::vector<uint> uuids;
		FontImportSettings importerSettings;
		int64_t lastModTimeMeta = 0;
		bool result = ResourceFont::ReadMeta(metaFile, lastModTimeMeta, uuids, importerSettings);
		assert(result);
		std::string name;
		App->fs->GetFileName(file, name);

		for (size_t i = 0; i < importerSettings.sizes.size(); ++i)
		{
			char exportedFile[DEFAULT_BUF_SIZE];
			sprintf(exportedFile, "%s/%s%i.fnt", DIR_LIBRARY_FONT, name.data(), importerSettings.sizes[i]);
			if (!App->fs->Exists(exportedFile))
			{

			}
		}


		ResourceFont* resFont = (ResourceFont*)App->res->GetResource(uuids);
		if (resFont)
		{
			//Actualizar recurso y archivo binario


		}
		else
		{
			//Exportar nuevo binario + Importar nuevo recurso

		}














		ResourceData data;
		data.file = file;
		App->fs->GetFileName(file, data.name);
		data.exportedFile = "";
		ResourceFont* font = new ResourceFont(uuid, data, ResourceFontData());
	}
	//Si no existe el meta creamos creamos un meta con un size por defecto
	else
	{
		//Exportar nuevo binario + Importar nuevo recurso

	
	}

	return nullptr;
}

bool ResourceFont::ExportFile(ResourceData & data, ResourceFontData & fontData, std::string & outputFile, bool overwrite)
{
	SaveFile(data, fontData, outputFile, overwrite);
	return true;
}

uint ResourceFont::SaveFile(ResourceData& data, ResourceFontData& fontData, std::string& outputFile, bool overwrite)
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

bool ResourceFont::LoadFile(const char * file, ResourceFontData & fontData)
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

		CONSOLE_LOG(LogTypes::Normal, "Resource Font: Successfully loaded Font'%s'", file);
		RELEASE_ARRAY(buffer);
	}
	else
		CONSOLE_LOG(LogTypes::Error, "Resource Font: Could not load Font'%s'", file);

	return ret;
}

uint ResourceFont::CreateMeta(const char * file, std::vector<uint> fontUuids, std::string & outputMetaFile, FontImportSettings importSettings)
{
	assert(file != nullptr);

	uint uuidsSize = fontUuids.size();
	uint sizesSize = importSettings.sizes.size();
	// --------------------------------------------------

	uint size =
		sizeof(int64_t) +
		sizeof(uint) * 2 +
		sizeof(uint) * uuidsSize;
		sizeof(uint) * sizesSize;

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

	// 3. Store fonts uuids
	bytes = sizeof(uint) * uuidsSize;
	memcpy(cursor, fontUuids.data(), bytes);
	cursor += bytes;

	//4. Store font import Settings
	bytes = sizeof(uint);
	memcpy(cursor, &sizesSize, bytes);
	cursor += bytes;

	memcpy(cursor, importSettings.sizes.data(), sizesSize);
	cursor += sizesSize;


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

bool ResourceFont::ReadMeta(const char * metaFile, int64_t & lastModTime, std::vector<uint> &fontUuids, FontImportSettings &importSettings)
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
		cursor += bytes;

		// 3. Load font uuid
		fontUuids.resize(uuidsSize);
		bytes = sizeof(uint) * uuidsSize;
		memcpy(fontUuids.data(), cursor, bytes);

		cursor += bytes;

		uint sizesSize;
		//4. Load font import Settings
		bytes = sizeof(uint);
		memcpy(&sizesSize, cursor, bytes);

		cursor += bytes;

		importSettings.sizes.resize(sizesSize);
		memcpy(importSettings.sizes.data(), cursor, sizesSize);
		cursor += sizesSize;


		CONSOLE_LOG(LogTypes::Normal, "Resource Font: Successfully loaded meta '%s'", metaFile);
		RELEASE_ARRAY(buffer);
	}
	else
	{
		CONSOLE_LOG(LogTypes::Error, "Resource Font: Could not load meta '%s'", metaFile);
		return false;
	}

	return true;
}


bool ResourceFont::ReadMetaFromBuffer(char* cursor, int64_t & lastModTime, std::vector<uint> &fontUuids, FontImportSettings &importSettings)
{

	// 1. Load last modification time
	uint bytes = sizeof(int64_t);
	memcpy(&lastModTime, cursor, bytes);

	cursor += bytes;

	// 2. Load uuids size
	uint uuidsSize = 0;
	bytes = sizeof(uint);
	memcpy(&uuidsSize, cursor, bytes);

	cursor += bytes;

	// 3. Load font uuid
	fontUuids.resize(uuidsSize);
	bytes = sizeof(uint) * uuidsSize;
	memcpy(fontUuids.data(), cursor, bytes);

	cursor += bytes;

	uint sizesSize;
	//4. Load font import Settings
	bytes = sizeof(uint);
	memcpy(&sizesSize, cursor, bytes);

	cursor += bytes;

	importSettings.sizes.resize(sizesSize);
	memcpy(importSettings.sizes.data(), cursor, sizesSize);
	cursor += sizesSize;

	return true;
}

void ResourceFont::UpdateImportSettings(FontImportSettings importSettings)
{

	std::vector<uint> uuids;
	FontImportSettings prevImportSettings;
	int64_t lastModTimeMeta = 0;
	bool result = ResourceFont::ReadMeta((importSettings.fontPath + ".meta").data(), lastModTimeMeta, uuids, prevImportSettings);
	if (result)
	{
		for (size_t i = 0; i < prevImportSettings.sizes.size(); ++i)
		{
			if (std::find(importSettings.sizes.begin(), importSettings.sizes.end(), prevImportSettings.sizes[i]) == importSettings.sizes.end())
			{
				//Borrar recurso PAPU
				for (size_t j = 0; j < uuids.size(); ++j)
				{
					ResourceFont* font = (ResourceFont*)App->res->GetResource(uuids[j]);
					if (font)
					{
						if (font->size == prevImportSettings.sizes[i])
						{
							App->res->DeleteResource(uuids[j]);
							break;
						}
					}
				}
			}
		}
		
		ResourceFont::CreateMeta(importSettings.fontPath.data(),);
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		System_Event newEvent;
		newEvent.type = System_Event_Type::DeleteUnusedFiles;
		App->PushSystemEvent(newEvent);

	}
}