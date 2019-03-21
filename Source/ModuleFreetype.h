#ifndef __MODULE_FREETYPE_H__
#define __MODULE_FREETYPE_H__

#include "Module.h"
#include <map>

#include "MathGeoLib/include/Math/float2.h"

#include "ResourceFont.h"

class ModuleFreetype : public Module
{

public:
	ModuleFreetype(bool start_enabled = true);
	~ModuleFreetype();

	bool Init(JSON_Object * jObject);
	bool Start();
	update_status Update();

	uint LoadFont(const char * path, int size, std::map<char, Character>& charactersBitmap);

public:

private:
	FT_Library library;

};

#endif //__MODULE_FREETYPE_H__
