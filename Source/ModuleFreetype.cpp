#include "ModuleFreetype.h"
#include "Application.h"

#include "SDL/include/SDL_opengl.h"
#include "DevIL\include\ilu.h"


ModuleFreetype::ModuleFreetype(bool start_enabled) : Module(start_enabled)
{

}

ModuleFreetype::~ModuleFreetype()
{
	FT_Done_FreeType(library);
}

bool ModuleFreetype::Init(JSON_Object* jObject)
{
	if(FT_Init_FreeType(&library))
		CONSOLE_LOG(LogTypes::Error, "Error when it's initialization FreeType");

	return true;
}

bool ModuleFreetype::Start() {
	//LoadFont("../Game/Assets/Textures/Font/ariali.ttf",16);
	return true;
}

update_status ModuleFreetype::Update()
{
	return UPDATE_CONTINUE;
}
void ModuleFreetype::LoadFont(const char* path, int size, std::map<char, Character> &charactersBitmap)
{
	FT_Face face;      /* handle to face object */
	if (FT_New_Face(library, path, 0, &face))
	{
		CONSOLE_LOG(LogTypes::Error, "The font file couldn't be opened, read or this format is unsupported");
	}

	else
	{
		FT_Set_Pixel_Sizes(face, 0, size);

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
			charactersBitmap.insert(std::pair<char, Character>(c, character));
		}
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	FT_Done_Face(face);
}

