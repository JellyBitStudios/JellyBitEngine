/*#include "FontImporter.h"

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

bool FontImporter::Import(const char * file, std::string & outputFile, ResourceFontData & importSettings) const
{
	assert(file != nullptr);

	bool ret = false;

	char* buffer;
	uint size = App->fs->Load(file, &buffer);
	if (size > 0)
	{
		CONSOLE_LOG(LogTypes::Normal, "FONT IMPORTER: Successfully loaded Font '%s' (original format)", file);
		uint maxCharHeight = 0;

		FT_Face face;    
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

bool FontImporter::Load(const char* exportedFile, ResourceData& outputData, ResourceFontData& outputFontData) const
{
	assert(exportedFile != nullptr);

	bool ret = false;

	char* buffer;
	uint size = App->fs->Load(exportedFile, &buffer);
	if (size > 0)
	{
		char* cursor = (char*)buffer;

		uint bytes = sizeof(uint);
		memcpy(&outputFontData.fontSize, cursor, bytes);
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
			outputFontData.charactersMap.insert(std::pair<char, Character>(i + 32, character));

			if (i < charactersSize - 1)
				cursor += bytes;
		}

		CONSOLE_LOG(LogTypes::Normal, "Resource Font: Successfully loaded Font'%s'", exportedFile);
		RELEASE_ARRAY(buffer);
	}
	else
		CONSOLE_LOG(LogTypes::Error, "Resource Font: Could not load Font'%s'", exportedFile);

	return ret;
	return true;
}

// ----------------------------------------------------------------------------------------------------

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