#include "ModuleFreetype.h"
#include "freetype/ft2build.h"
#include FT_FREETYPE_H


ModuleFreetype::ModuleFreetype(bool start_enabled) : Module(start_enabled)
{
	//math::Frustum::ViewportToScreenSpace();
}

ModuleFreetype::~ModuleFreetype()
{
}


