#include "ModuleFreetype.h"
#include "freetype/ft2build.h"
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

	FT_Library  library;   /* handle to library     */
	FT_Face     face;      /* handle to face object */
	FT_Error	error;

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

	return UPDATE_CONTINUE;
}

