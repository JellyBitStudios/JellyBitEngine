#ifndef __RESOURCE_FONT_H__
#define __RESOURCE_FONT_H__

#include "Resource.h"

#include <map>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "MathGeoLib\include\Math\float2.h"

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

	ResourceFont(ResourceTypes type, uint uuid, ResourceData data, ResourceFontData boneData);
	~ResourceFont();

	bool LoadInMemory();
	bool UnloadFromMemory();

	static bool ImportFile(const char* file, std::string& name, std::string& outputFile);
	static bool ExportFile(ResourceData& data, ResourceFontData& font_data, std::string& outputFile, bool overwrite = false);
	static uint SaveFile(ResourceData & data, ResourceFontData & materialData, std::string & outputFile, bool overwrite);
	static uint CreateMeta(const char* file, uint font_uuid, std::string& name, std::string& outputMetaFile);
	static bool ReadMeta(const char* metaFile, int64_t& lastModTime, uint& font_uuid, std::string& name);
	static bool LoadFile(const char* file, ResourceFontData& font_data_output);

	void OnPanelAssets();

public:

	//std::map<char, CharacterData> charactersMap;

	ResourceFontData fontData;
	FT_Library library;
	FT_Face face;
};

#endif // __RESOURCE_FONT_H__