#include "ModuleFreetype.h"
#include "freetype/ft2build.h"
#include FT_FREETYPE_H

#include <stdio.h>
#include <string.h>
#include <math.h>


ModuleFreetype::ModuleFreetype(bool start_enabled) : Module(start_enabled)
{

}

ModuleFreetype::~ModuleFreetype()
{
	FT_Library    library;
	FT_Face       face;

	FT_GlyphSlot  slot;
	FT_Matrix     matrix;                 /* transformation matrix */
	FT_Vector     pen;                    /* untransformed origin  */
	FT_Error      error;

	char*         filename;
	char*         text;

	double        angle;
	int           target_height;
	int           n, num_chars;


	if (argc != 3)
	{
		fprintf(stderr, "usage: %s font sample-text\n", argv[0]);
		exit(1);
	}

	filename = argv[1];                           /* first argument     */
	text = argv[2];                           /* second argument    */
	num_chars = strlen(text);
	angle = (25.0 / 360) * 3.14159 * 2;      /* use 25 degrees     */
	target_height = HEIGHT;

	error = FT_Init_FreeType(&library);              /* initialize library */
	/* error handling omitted */

	error = FT_New_Face(library, filename, 0, &face);/* create face object */
	/* error handling omitted */

	/* use 50pt at 100dpi */
	error = FT_Set_Char_Size(face, 50 * 64, 0,
		100, 0);                /* set character size */
/* error handling omitted */

/* cmap selection omitted;                                        */
/* for simplicity we assume that the font contains a Unicode cmap */

	slot = face->glyph;

	/* set up matrix */
	matrix.xx = (FT_Fixed)(cos(angle) * 0x10000L);
	matrix.xy = (FT_Fixed)(-sin(angle) * 0x10000L);
	matrix.yx = (FT_Fixed)(sin(angle) * 0x10000L);
	matrix.yy = (FT_Fixed)(cos(angle) * 0x10000L);

	/* the pen position in 26.6 cartesian space coordinates; */
	/* start at (300,200) relative to the upper left corner  */
	pen.x = 300 * 64;
	pen.y = (target_height - 200) * 64;

	for (n = 0; n < num_chars; n++)
	{
		/* set transformation */
		FT_Set_Transform(face, &matrix, &pen);

		/* load glyph image into the slot (erase previous one) */
		error = FT_Load_Char(face, text[n], FT_LOAD_RENDER);
		if (error)
			continue;                 /* ignore errors */

		  /* now, draw to our target surface (convert position) */
		draw_bitmap(&slot->bitmap,
			slot->bitmap_left,
			target_height - slot->bitmap_top);

		/* increment pen position */
		pen.x += slot->advance.x;
		pen.y += slot->advance.y;
	}

	show_image();

	FT_Done_Face(face);
	FT_Done_FreeType(library);


}


