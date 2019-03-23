#ifndef __FONT_IMPORTER_H__
#define __FONT_IMPORTER_H__

#include <ft2build.h>
#include FT_FREETYPE_H

struct ResourceData;
struct ResourceFontData;

#include <string>
class FontImporter
{
public:

	FontImporter();
	~FontImporter();

	bool Load(const char * exportedFile, ResourceData & outputData, ResourceFontData & outputFontData) const;
/*
	bool Import(const char * file, std::string& outputFile, const ResourceFontData& importSettings) const;
	

	bool Load(const char* exportedFile, ResourceData& outputData, FontData & outputTextureData) const;

	// ----------------------------------------------------------------------------------------------------

	void LoadInMemory(uint& id, const FontData & textureData);
*/
private:

	FT_Library library;
};
#endif