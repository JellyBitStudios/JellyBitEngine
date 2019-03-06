#ifndef __MODULE_FREETYPE_H__
#define __MODULE_FREETYPE_H__

#include "Module.h"

class ModuleFreetype : public Module
{

public:
	ModuleFreetype(bool start_enabled = true);
	~ModuleFreetype();

	bool Start();
	update_status Update();


private:
	FT_Library library;   /* handle to library     */
	FT_Face face;      /* handle to face object */
	FT_Error error;
	FT_GlyphSlot slot;
	FT_Matrix matrix;
};

#endif //__MODULE_FREETYPE_H__
