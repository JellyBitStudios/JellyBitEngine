#ifndef __RESOURCE_FONT_H__
#define __RESOURCE_FONT_H__

#include "Resource.h"

#include <vector>
#include <map>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "MathGeoLib\include\Math\float2.h"

struct FontImportSettings
{
	std::string fontPath;
	std::vector<uint> sizes;
};
struct Character
{
	uint textureID;
	math::float2 size;
	math::float2 bearing;
	uint advance;
};

struct ResourceFontData
{
	uint fontSize;
	uint maxCharHeight;
	std::map<char, Character> charactersMap;
};
class ResourceFont : public Resource
{
public:

	ResourceFont(uint uuid, ResourceData data, ResourceFontData boneData);
	~ResourceFont();

	bool LoadInMemory();
	bool UnloadFromMemory();

	static Resource* ImportFile(const char* file);
	static bool ExportFile(ResourceData& data, ResourceFontData& font_data, std::string& outputFile, bool overwrite = false);
	static uint SaveFile(ResourceData & data, ResourceFontData & materialData, std::string & outputFile, bool overwrite);
	static uint CreateMeta(const char * file, std::vector<uint> fontUuids, std::string & outputMetaFile, FontImportSettings importSettings);
	static bool ReadMeta(const char * metaFile, int64_t & lastModTime, std::vector<uint>& fontUuids, FontImportSettings & importSettings);
	static bool ReadMetaFromBuffer(char * cursor, int64_t & lastModTime, std::vector<uint>& fontUuids, FontImportSettings & importSettings);
	static bool LoadFile(const char* file, ResourceFontData& font_data_output);

	static void UpdateImportSettings(FontImportSettings importSettings);
	void OnPanelAssets();

public:

	std::map<char, Character> charactersMap;
	uint size;

	FontImportSettings importSettings;
	ResourceFontData fontData;
	FT_Library library;
	FT_Face face;
};

#endif // __RESOURCE_FONT_H__