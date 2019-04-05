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

Resource* ResourceFont::ImportFile(const char * file)
{
	assert(file != nullptr);

	if (!App->fs->Exists(file))
		return nullptr;

	// Search for the meta associated to the file
	char metaFile[DEFAULT_BUF_SIZE];
	strcpy_s(metaFile, strlen(file) + 1, file); // file
	strcat_s(metaFile, strlen(metaFile) + strlen(EXTENSION_META) + 1, EXTENSION_META); // extension

	//If .meta exists we have to search all sizes in library and exports those that are missing
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

		bool recalculateMeta = false;
		for (size_t i = 0; i < importerSettings.sizes.size(); ++i)
		{
			char exportedFile[DEFAULT_BUF_SIZE];
			sprintf(exportedFile, "%s/%s%i_%u.fnt", DIR_LIBRARY_FONT, name.data(), importerSettings.sizes[i], uuids[i]);
			ResourceFont* resFont = nullptr;
			if (!App->fs->Exists(exportedFile))
			{
				//We supposed that there aren't resources in memory at this point
				resFont = ResourceFont::ImportFontBySize(file, importerSettings.sizes[i],uuids[i]);
			}
			else
			{
				//Make sure that resource is in memory
				std::vector<Resource*> fonts = App->res->GetResourcesByType(ResourceTypes::FontResource);
				bool found = false;
				for (uint j = 0; j < fonts.size(); ++j)
				{
					ResourceFont* font = (ResourceFont*)fonts[j];
					if (font->fontData.fontSize == importerSettings.sizes[i])
					{
						found = true;
					}
				}
				if (!found)
					resFont = ResourceFont::LoadFile(exportedFile);
			}
			if (resFont)
			{
				App->res->AddResource(resFont);
			}
		}
	}
	//If meta doesn't exist we create it with default font size
	else
	{
		//Exportar new binary + Import new resource
		std::vector<uint> uuids;
		uuids.push_back(App->GenerateRandomNumber());
		FontImportSettings fontImport;
		fontImport.sizes.push_back(48);//Default size
		ResourceFont::CreateMeta(file, uuids, fontImport);
	
		ResourceFont* resource = ResourceFont::ImportFontBySize(file, 48, uuids[0]);
		if (resource)
			App->res->AddResource(resource);
		return resource;
	}

	return nullptr;
}

uint ResourceFont::SaveFile(ResourceData& data, ResourceFontData& fontData)
{
	uint sizeBuffer = 0;
	for (uint i = 0; i < fontData.charactersMap.size(); ++i)
	{
		sizeBuffer += (fontData.charactersMap[i+32].size.x * fontData.charactersMap[i+32].size.y);
	}
	uint size = sizeof(uint) * 3 + sizeBuffer +
		sizeof(Character) * fontData.charactersMap.size();

	char* buffer = new char[size];
	char* cursor = buffer;

	uint bytes = sizeof(uint);
	memcpy(cursor, &fontData.fontSize, bytes);
	cursor += bytes;

	memcpy(cursor, &fontData.maxCharHeight, bytes);
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

	for (uint i = 0; i < listSize; ++i)
	{
		bytes = (fontData.charactersMap[i+32].size.x * fontData.charactersMap[i+32].size.y);
		memcpy(cursor, fontData.fontBuffer[i], bytes);
		cursor += bytes;
	}
	// --------------------------------------------------

	// Save the file
	uint ret = App->fs->Save(data.exportedFile, buffer, size);

	if (ret > 0)
	{ 
		CONSOLE_LOG(LogTypes::Normal, "Resource Font: Successfully saved Font '%s'", data.exportedFile.data());
	}
	else
		CONSOLE_LOG(LogTypes::Error, "Resource Font: Could not save Font '%s'", data.exportedFile.data());

	RELEASE_ARRAY(buffer);

	return ret;
}

ResourceFont* ResourceFont::LoadFile(const char * file)
{
	assert(file != nullptr);

	ResourceFont* res = nullptr;

	char* buffer;
	uint size = App->fs->Load(file, &buffer);
	if (size > 0)
	{
		//Get Uuid
		std::string filePath = file;
		App->fs->GetFileName(file, filePath);
		std::string uuidStr = filePath.substr(filePath.find("_") + 1, filePath.find("."));
		uint uuid = std::stoul(uuidStr.data());

		ResourceFontData fontData;
		char* cursor = (char*)buffer;

		uint bytes = sizeof(uint);
		memcpy(&fontData.fontSize, cursor, bytes);
		cursor += bytes;

		memcpy(&fontData.maxCharHeight, cursor, bytes);
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

			cursor += bytes;
		}

		fontData.fontBuffer.resize(charactersSize);
		for (uint i = 0; i < charactersSize; ++i)
		{
			uint width = fontData.charactersMap[i + 32].size.x;
			uint height = fontData.charactersMap[i + 32].size.y;

			bytes = sizeof(uint8_t) * width * height;
			uint8_t* buffer = new uint8_t[width * height];
			memcpy(buffer, cursor, bytes);
			fontData.fontBuffer[i] = buffer;
			fontData.charactersMap[i + 32].textureID = ResourceFont::LoadTextureCharacter(width, height, fontData.fontBuffer[i]);

			cursor += bytes;
		}

		CONSOLE_LOG(LogTypes::Normal, "Resource Font: Successfully loaded Font'%s'", file);
		RELEASE_ARRAY(buffer);
		
		ResourceData data;
		data.name = filePath.substr(0, filePath.find("_"));
		data.file = DIR_ASSETS_FONT + std::string("/") + data.name + EXTENSION_FONT;
		data.exportedFile = file;

		res = new ResourceFont(uuid, data, fontData);
	}
	else
		CONSOLE_LOG(LogTypes::Error, "Resource Font: Could not load Font'%s'", file);

	return res;
}

uint ResourceFont::CreateMeta(const char * file, std::vector<uint> fontUuids, FontImportSettings importSettings)
{
	assert(file != nullptr);

	uint uuidsSize = fontUuids.size();
	uint sizesSize = importSettings.sizes.size();
	// --------------------------------------------------

	uint size =
		sizeof(int64_t) +
		sizeof(uint) * 2 +
		sizeof(uint) * uuidsSize +
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

	bytes = sizeof(uint) * sizesSize;
	memcpy(cursor, importSettings.sizes.data(), bytes);
	cursor += bytes;

	// --------------------------------------------------
	// Build the path of the meta file and save it
	std::string outputMetaFile = file;
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

	delete[] data;
	return size;
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
		
		bytes = sizeof(uint) * sizesSize;
		importSettings.sizes.resize(sizesSize);
		memcpy(importSettings.sizes.data(), cursor, bytes);
		cursor += bytes;
		

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

bool ResourceFont::ReadMetaFromBuffer(char* &exCursor, int64_t & lastModTime, std::vector<uint> &fontUuids, FontImportSettings &importSettings)
{

	char* cursor = exCursor;
	
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

	bytes = sizeof(uint) * sizesSize;
	importSettings.sizes.resize(sizesSize);
	memcpy(importSettings.sizes.data(), cursor, bytes);
	cursor += bytes;
	
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
				//Delete unused resource
				for (size_t j = 0; j < uuids.size(); ++j)
				{
					ResourceFont* font = (ResourceFont*)App->res->GetResource(uuids[j]);
					if (font)
					{
						if (font->fontData.fontSize == prevImportSettings.sizes[i])
						{
							App->res->DeleteResource(uuids[j]);
							uuids.erase(uuids.begin() + j);
							break;
						}
					}
				}
			}
		}
		
		for (size_t i = 0; i < importSettings.sizes.size(); ++i)
		{
			if (std::find(prevImportSettings.sizes.begin(), prevImportSettings.sizes.end(), importSettings.sizes[i]) == prevImportSettings.sizes.end())
			{
				//Create new resource
				ResourceFont* newFont = ImportFontBySize(importSettings.fontPath.data(), importSettings.sizes[i]);
				if (newFont)
				{
					App->res->AddResource(newFont);
					uuids.push_back(newFont->uuid);
				}
			}
		}

		ResourceFont::CreateMeta(importSettings.fontPath.data(), uuids, importSettings);		
		
		System_Event newEvent;
		newEvent.type = System_Event_Type::DeleteUnusedFiles;
		App->PushSystemEvent(newEvent);
	}
}

ResourceFont* ResourceFont::ImportFontBySize(const char * file, uint size, uint uuid)
{
	ResourceFont* res = nullptr;
	uint maxHeight = 0;
	std::map<char, Character> charactersBitmap;

	FT_Face face;      /* handle to face object */
	if (FT_New_Face(App->ui->library, file, 0, &face))
	{
		CONSOLE_LOG(LogTypes::Error, "The font file couldn't be opened, read or this format is unsupported");
	}

	else
	{
		ResourceFontData fontData;
		FT_Set_Pixel_Sizes(face, 0, size);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction
		for (uint c = 32; c < 128; c++)
		{
			// Load character glyph 
			if (FT_Load_Char(face, c, FT_LOAD_RENDER))
			{
				CONSOLE_LOG(LogTypes::Error, "Failed to load Glyph from Freetype");
				continue;
			}
			GLuint texture = ResourceFont::LoadTextureCharacter(face->glyph->bitmap.width, face->glyph->bitmap.rows, face->glyph->bitmap.buffer);

			// Now store character for later use
			Character character = {
				texture,
				math::float2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
				math::float2(face->glyph->bitmap_left, face->glyph->bitmap_top),
				face->glyph->advance.x / 64
			};
			charactersBitmap.insert(std::pair<char, Character>(c, character));
			if (face->glyph->bitmap.rows > maxHeight)
				maxHeight = face->glyph->bitmap.rows;
			
			//Save buffer for next loads
			uint bufferSize = face->glyph->bitmap.width * face->glyph->bitmap.rows;
			uint8_t* characterBuffer = new uint8_t[bufferSize];
			uint bytes = sizeof(uint8_t) * bufferSize;
			memcpy(characterBuffer, (uint8_t*)face->glyph->bitmap.buffer, bytes);			

			fontData.fontBuffer.push_back(characterBuffer);
		}
		glBindTexture(GL_TEXTURE_2D, 0);
		FT_Done_Face(face);
	
		ResourceData data;
		data.file = file;
		App->fs->GetFileName(file, data.name);
		data.name += std::to_string(size);

		uuid = uuid != 0 ? uuid : App->GenerateRandomNumber();
		data.exportedFile = DIR_LIBRARY_FONT + std::string("/") + data.name + "_" + std::to_string(uuid) + ".fnt";

		fontData.charactersMap = charactersBitmap;
		fontData.fontSize = size;
		fontData.maxCharHeight = maxHeight;

		res = new ResourceFont(uuid, data, fontData);


		ResourceFont::SaveFile(data, fontData);
	}

	return res;
}

uint ResourceFont::LoadTextureCharacter(uint width, uint height, uchar* buffer)
{
	// Generate texture
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, buffer);

	// Set texture options
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	return texture;
}

