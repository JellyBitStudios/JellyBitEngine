#include "ModuleFreetype.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

#pragma comment(lib, "Freetype/libx86/freetype.lib")

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
	matrix.xx = (FT_Fixed)(cos(angle) * 0x10000L);
	matrix.xy = (FT_Fixed)(-sin(angle) * 0x10000L);
	matrix.yx = (FT_Fixed)(sin(angle) * 0x10000L);
	matrix.yy = (FT_Fixed)(cos(angle) * 0x10000L);

	/* the pen position in 26.6 cartesian space coordinates; */
	/* start at (300,200) relative to the upper left corner  */
	pen.x = 300 * 64;
	pen.y = (64 - 200) * 64;
	char* text;
	for (int n = 0; n < num_chars; n++)
	{
		/* set transformation */
		FT_Set_Transform(face, &matrix, &pen);

		/* load glyph image into the slot (erase previous one) */
		error = FT_Load_Char(face, text[n], FT_LOAD_RENDER);
		if (error)
			continue;                 /* ignore errors */

									  /* now, draw to our target surface (convert position) */
		/*draw_bitmap(&slot->bitmap,
			slot->bitmap_left,
			target_height - slot->bitmap_top);*/

		/* increment pen position */
		pen.x += slot->advance.x;
		pen.y += slot->advance.y;
	}
	
	return UPDATE_CONTINUE;
}

