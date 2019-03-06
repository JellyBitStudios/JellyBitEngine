#ifndef __MODULE_FREETYPE_H__
#define __MODULE_FREETYPE_H__

#include "Module.h"
#include <string>

struct FT_Library;
struct FT_Face;      
struct FT_Error;
struct FT_GlyphSlot;
struct FT_Matrix;
struct FT_Vector;

class ModuleFreetype : public Module
{

public:
	ModuleFreetype(bool start_enabled = true);
	~ModuleFreetype();

	bool Init(JSON_Object * jObject);
	bool Start();
	update_status Update();

	bool LoadFont(const char * path);

	void draw_bitmap(FT_Bitmap * bitmap, FT_Int x, FT_Int y);
	void show_image();

private:
	FT_Library library;   /* handle to library     */
	FT_Face face;      /* handle to face object */
	FT_Error error;
	FT_GlyphSlot slot;
	FT_Matrix matrix;
	FT_Vector pen;
	std::string text;
	unsigned char image[640][480];
};

#endif //__MODULE_FREETYPE_H__
