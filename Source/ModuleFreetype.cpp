#include "ModuleFreetype.h"
#include "freetype/ft2build.h"
#include "Freetype/freetype.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

ModuleFreetype::ModuleFreetype(bool start_enabled) : Module(start_enabled)
{

}

ModuleFreetype::~ModuleFreetype()
{

}

bool ModuleFreetype::Start() {



	return true;
}
update_status ModuleFreetype::Update()
{
	FT_Library  library;   /* handle to library     */
	FT_Face     face;      /* handle to face object */
	FT_Error	error;

	FT_Init_FreeType(&library);

	error = FT_New_Face(library, "../Game/Assets/Textures/Font/ariali.ttf", 0, &face);
	/*if (error == FT_Err_Unknown_File_Format)
		CONSOLE_LOG(LogTypes::Error, "The font file could be opened and read, but this format is unsupported");

	else if (error)
		CONSOLE_LOG(LogTypes::Error, "The font file couldn't be opened or read, or it's a broken format");
		*/

	return UPDATE_CONTINUE;
}

