#include "FontImporter.h"

#include "Application.h"
#include "Globals.h"
#include "ModuleFileSystem.h"
#include "ResourceTexture.h"
#include "ResourceFont.h"

#include "SDL/include/SDL_opengl.h"
#include "DevIL\include\ilu.h"

#include <assert.h>

#pragma comment (lib, "DevIL\\libx86\\DevIL.lib")
#pragma comment (lib, "DevIL\\libx86\\ILU.lib")
#pragma comment (lib, "DevIL\\libx86\\ILUT.lib")

// Reference: https://open.gl/textures

FontImporter::FontImporter()
{
	if (FT_Init_FreeType(&library))
		CONSOLE_LOG(LogTypes::Error, "Error when it's initialization FreeType");
}

FontImporter::~FontImporter() 
{
	FT_Done_FreeType(library);
}

bool FontImporter::Import(const char * file, std::string & outputFile, const ResourceFontData & importSettings) const
{
	assert(file != nullptr);

	bool ret = false;

	char* buffer;
	uint size = App->fs->Load(file, &buffer);
	if (size > 0)
	{
		CONSOLE_LOG(LogTypes::Normal, "FONT IMPORTER: Successfully loaded Font '%s' (original format)", file);
		uint maxCharHeight = 0;

		FT_Face face;      /* handle to face object */
		if (!FT_New_Memory_Face(library, (FT_Byte*)buffer, size, 0, &face))
		{
			uint fontSize = 48;
			FT_Set_Pixel_Sizes(face, 0, fontSize);

			glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction
			for (uint c = 32; c < 128; c++)
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
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);

				// Set texture options
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

				// Now store character for later use
				Character character = {
					texture,
					math::float2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
					math::float2(face->glyph->bitmap_left, face->glyph->bitmap_top),
					face->glyph->advance.x / 64
				};
				importSettings.charactersMap.insert(std::pair<char, Character>(c, character));
				if (face->glyph->bitmap.rows > maxCharHeight)
					maxCharHeight = face->glyph->bitmap.rows;
			}
			importSettings.maxCharHeight = maxCharHeight;
			importSettings.fontSize = fontSize;
			glBindTexture(GL_TEXTURE_2D, 0);
			FT_Done_Face(face);
		}
		else
			CONSOLE_LOG(LogTypes::Error, "The font file couldn't be opened, read or this format is unsupported");

		RELEASE_ARRAY(buffer);
	}
	else
		CONSOLE_LOG(LogTypes::Error, "MATERIAL FONT: Could not load Font '%s' (original format)", file);

	return ret;

}
/*
bool FontImporter::Import(const void* buffer, uint size, std::string& outputFile, const ResourceTextureImportSettings& importSettings, uint forcedUuid) const
{
	assert(buffer != nullptr && size > 0);

	bool ret = false;

	// Generate the image name
	uint imageName = 0;
	ilGenImages(1, &imageName);

	// Bind the image
	ilBindImage(imageName);

	// Load the image
	if (ilLoadL(IL_TYPE_UNKNOWN, buffer, size))
	{
		ilEnable(IL_FILE_OVERWRITE);

		uint size = 0;
		ILubyte* data = nullptr;

		// Pick a specific DXT compression use
		int compression = 0;

		switch (importSettings.compression)
		{
		case ResourceTextureImportSettings::TextureCompression::DXT1:
			compression = IL_DXT1;
			break;
		case ResourceTextureImportSettings::TextureCompression::DXT3:
			compression = IL_DXT3;
			break;
		case ResourceTextureImportSettings::TextureCompression::DXT5:
			compression = IL_DXT5;
			break;
		}

		ilSetInteger(IL_DXTC_FORMAT, compression);

		// Get the size of the data buffer
		size = ilSaveL(IL_DDS, NULL, 0);

		if (size > 0)
		{
			ilEnable(IL_FILE_OVERWRITE);

			// Allocate the data buffer
			data = new ILubyte[size];

			// Save to the buffer
			if (ilSaveL(IL_DDS, data, size) > 0)
			{
				uint uuid = forcedUuid == 0 ? App->GenerateRandomNumber() : forcedUuid;
				outputFile = std::to_string(uuid);
				if (App->fs->SaveInGame((char*)data, size, FileTypes::TextureFile, outputFile) > 0)
				{
					CONSOLE_LOG(LogTypes::Normal, "MATERIAL IMPORTER: Successfully saved Texture '%s' to own format", outputFile.data());
					ret = true;
				}
				else
					CONSOLE_LOG(LogTypes::Error, "MATERIAL IMPORTER: Could not save Texture '%s' to own format", outputFile.data());
			}

			RELEASE_ARRAY(data);
		}

		ilDeleteImages(1, &imageName);
	}
	else
		CONSOLE_LOG(LogTypes::Error, "MATERIAL IMPORTER: DevIL could not load the image. ERROR: %s", iluErrorString(ilGetError()));

	return ret;
}
*/
bool FontImporter::Load(const char* exportedFile, ResourceData& outputData, ResourceFontData& outputFontData) const
{

}

// ----------------------------------------------------------------------------------------------------
/*
void FontImporter::LoadInMemory(uint& id, const FontData& textureData)
{
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Generate the texture name
	uint texName = 0;
	glGenTextures(1, &texName);

	// Bind the texture
	glBindTexture(GL_TEXTURE_2D, texName);

	// http://openil.sourceforge.net/tuts/tut_8/index.htm

	// Set texture wrap mode
	int wrap = 0;
	switch (textureData.textureImportSettings.wrapS)
	{
	case ResourceTextureImportSettings::TextureWrapMode::REPEAT:
		wrap = GL_REPEAT;
		break;
	case ResourceTextureImportSettings::TextureWrapMode::MIRRORED_REPEAT:
		wrap = GL_MIRRORED_REPEAT;
		break;
	case ResourceTextureImportSettings::TextureWrapMode::CLAMP_TO_EDGE:
		wrap = GL_CLAMP_TO_EDGE;
		break;
	case ResourceTextureImportSettings::TextureWrapMode::CLAMP_TO_BORDER:
		wrap = GL_CLAMP_TO_BORDER;
		break;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);

	switch (textureData.textureImportSettings.wrapT)
	{
	case ResourceTextureImportSettings::TextureWrapMode::REPEAT:
		wrap = GL_REPEAT;
		break;
	case ResourceTextureImportSettings::TextureWrapMode::MIRRORED_REPEAT:
		wrap = GL_MIRRORED_REPEAT;
		break;
	case ResourceTextureImportSettings::TextureWrapMode::CLAMP_TO_EDGE:
		wrap = GL_CLAMP_TO_EDGE;
		break;
	case ResourceTextureImportSettings::TextureWrapMode::CLAMP_TO_BORDER:
		wrap = GL_CLAMP_TO_BORDER;
		break;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);

	// Set texture filter mode (Mipmap for the highest visual quality)
	int filter = 0;
	bool mipmap = false;

	switch (textureData.textureImportSettings.minFilter)
	{
	case ResourceTextureImportSettings::TextureFilterMode::NEAREST:
		filter = GL_NEAREST;
		break;
	case ResourceTextureImportSettings::TextureFilterMode::LINEAR:
		filter = GL_LINEAR;
		break;
	case ResourceTextureImportSettings::TextureFilterMode::NEAREST_MIPMAP_NEAREST:
		filter = GL_NEAREST_MIPMAP_NEAREST;
		break;
	case ResourceTextureImportSettings::TextureFilterMode::LINEAR_MIPMAP_NEAREST:
		filter = GL_LINEAR_MIPMAP_LINEAR;
		break;
	case ResourceTextureImportSettings::TextureFilterMode::NEAREST_MIPMAP_LINEAR:
		filter = GL_NEAREST_MIPMAP_LINEAR;
		break;
	case ResourceTextureImportSettings::TextureFilterMode::LINEAR_MIPMAP_LINEAR:
		filter = GL_LINEAR_MIPMAP_LINEAR;
		break;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);

	switch (textureData.textureImportSettings.magFilter)
	{
	case ResourceTextureImportSettings::TextureFilterMode::NEAREST:
		filter = GL_NEAREST;
		break;
	case ResourceTextureImportSettings::TextureFilterMode::LINEAR:
		filter = GL_LINEAR;
		break;
	case ResourceTextureImportSettings::TextureFilterMode::NEAREST_MIPMAP_NEAREST:
		filter = GL_NEAREST_MIPMAP_NEAREST;
		break;
	case ResourceTextureImportSettings::TextureFilterMode::LINEAR_MIPMAP_NEAREST:
		filter = GL_LINEAR_MIPMAP_LINEAR;
		break;
	case ResourceTextureImportSettings::TextureFilterMode::NEAREST_MIPMAP_LINEAR:
		filter = GL_NEAREST_MIPMAP_LINEAR;
		break;
	case ResourceTextureImportSettings::TextureFilterMode::LINEAR_MIPMAP_LINEAR:
		filter = GL_LINEAR_MIPMAP_LINEAR;
		break;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);

	// Anisotropic filtering
	if (isAnisotropySupported)
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, textureData.textureImportSettings.anisotropy);

	glTexImage2D(GL_TEXTURE_2D, 0, textureData.bpp, textureData.width, textureData.height,
		0, GL_RGBA, GL_UNSIGNED_BYTE, textureData.data);

	if (textureData.textureImportSettings.UseMipmap())
		glGenerateMipmap(GL_TEXTURE_2D);

	id = texName;

	CONSOLE_LOG(LogTypes::Normal, "MATERIAL IMPORTER: New texture loaded with: %i id", texName);

	glBindTexture(GL_TEXTURE_2D, 0);
}
*/