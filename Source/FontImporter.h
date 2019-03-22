#ifndef __FONT_IMPORTER_H__
#define __FONT_IMPORTER_H__

class FontImporter
{
public:

	FontImporter();
	~FontImporter();

	bool Import(const char* file, std::string& outputFile, const ResourceTextureImportSettings& importSettings, uint forcedUuid = 0) const;

	bool Load(const char* exportedFile, ResourceData& outputData, FontData & outputTextureData) const;

	// ----------------------------------------------------------------------------------------------------

	void LoadInMemory(uint& id, const FontData & textureData);

private:

	bool Import(const void* buffer, uint size, std::string& outputFile, const ResourceTextureImportSettings& importSettings, uint forcedUuid = 0) const;

	bool Load(const void * buffer, uint size, ResourceData & outputData, FontData & outputTextureData) const;

private:

	bool isAnisotropySupported = false;
	float largestSupportedAnisotropy = 0.0f;
};
#endif