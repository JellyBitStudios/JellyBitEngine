#include "ModuleFreetype.h"
#include "Freetype/ft2build.h"
#include "Freetype/freetype.h"
#include "Freetype/ftimage.h"
#include "Freetype/fttypes.h"

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
bool ModuleFreetype::Init(JSON_Object* jObject)
{
	FT_Init_FreeType(&library);
}
bool ModuleFreetype::Start() 
{
	LoadFont("../Game/Assets/Textures/Font/ariali.ttf");
	return true;
}
update_status ModuleFreetype::Update()
{
	slot = face->glyph;

	float angle = (25.0 / 360) * 3.14159 * 2;      /* use 25 degrees     */
	/* set up matrix */
	matrix.xx = (FT_Fixed)(cos(angle) * 0x10000L);
	matrix.xy = (FT_Fixed)(-sin(angle) * 0x10000L);
	matrix.yx = (FT_Fixed)(sin(angle) * 0x10000L);
	matrix.yy = (FT_Fixed)(cos(angle) * 0x10000L);
	
	/* the pen position in 26.6 cartesian space coordinates; */
	/* start at (300,200) relative to the upper left corner  */
	pen.x = 300 * 64;
	pen.y = (480 - 200) * 64;

	for (int n = 0; n < text.size(); n++)
	{
		/* set transformation */
		FT_Set_Transform(face, &matrix, &pen);

		/* load glyph image into the slot (erase previous one) */
		FT_Load_Char(face, text[n], FT_LOAD_RENDER);

		  /* now, draw to our target surface (convert position) */
		draw_bitmap(&slot->bitmap, slot->bitmap_left, 480 - slot->bitmap_top);

		/* increment pen position */
		pen.x += slot->advance.x;
		pen.y += slot->advance.y;
	}
	return UPDATE_CONTINUE;
}

bool ModuleFreetype::LoadFont(const char* path)
{
	bool ret = false;
	error = FT_New_Face(library, path, 0, &face);

	if (error > 0)
	{
		CONSOLE_LOG(LogTypes::Error, "The font file could be opened and read, but this format is unsupported");
	}
	else if (error == 0)
	{
		ret = true;
		CONSOLE_LOG(LogTypes::Normal, "FLOAT");
	}
	
	return ret;
}

void ModuleFreetype::draw_bitmap(FT_Bitmap*  bitmap, FT_Int x, FT_Int y)
{
	FT_Int  i, j, p, q;
	FT_Int  x_max = x + bitmap->width;
	FT_Int  y_max = y + bitmap->rows;


	/* for simplicity, we assume that `bitmap->pixel_mode' */
	/* is `FT_PIXEL_MODE_GRAY' (i.e., not a bitmap font)   */

	for (i = x, p = 0; i < x_max; i++, p++)
	{
		for (j = y, q = 0; j < y_max; j++, q++)
		{
			if (i < 0 || j < 0 ||
				i >= 640 || j >= 480)
				continue;

			image[j][i] |= bitmap->buffer[q * bitmap->width + p];
		}
	}
}


void ModuleFreetype::show_image()
{
	int  i, j;
	for (i = 0; i < 640; i++)
	{
		for (j = 0; j < 640; j++)
			putchar(image[i][j] == 0 ? ' '
				: image[i][j] < 128 ? '+'
				: '*');
		putchar('\n');
	}
}
