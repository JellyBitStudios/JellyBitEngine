#include "ModuleFreetype.h"
#include "Freetype/ft2build.h"
#include "Freetype/freetype.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

#pragma comment(lib, "Freetype/libx86/freetype26.lib")

ModuleFreetype::ModuleFreetype(bool start_enabled) : Module(start_enabled)
{

}

ModuleFreetype::~ModuleFreetype()
{

}

bool ModuleFreetype::Start() {

	FT_Init_FreeType(&library);

	error = FT_New_Face(library, "../Game/Assets/Textures/Font/ariali.ttf", 0, &face);
	
	if (error > 0)
	{
		CONSOLE_LOG(LogTypes::Error, "The font file could be opened and read, but this format is unsupported");
	}
	else if (error == 0)
	{
		CONSOLE_LOG(LogTypes::Normal, "FLOAT");
	}

	return true;
}
update_status ModuleFreetype::Update()
{

	slot = face->glyph;

	/* set up matrix */
	/*matrix.xx = (FT_Fixed)(cos(angle) * 0x10000L);
	matrix.xy = (FT_Fixed)(-sin(angle) * 0x10000L);
	matrix.yx = (FT_Fixed)(sin(angle) * 0x10000L);
	matrix.yy = (FT_Fixed)(cos(angle) * 0x10000L);
	*/
	return UPDATE_CONTINUE;
}

