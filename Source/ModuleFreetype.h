#ifndef __MODULE_FREETYPE_H__
#define __MODULE_FREETYPE_H__

#include "Module.h"
#include <map>

#include "MathGeoLib/include/Math/float2.h"

#include <ft2build.h>
#include FT_FREETYPE_H

struct Character
{
	uint textureID;
	math::float2 size;
	math::float2 bearing;
	uint advance;
};
class ModuleFreetype : public Module
{

public:
	ModuleFreetype(bool start_enabled = true);
	~ModuleFreetype();

	bool Init(JSON_Object * jObject);
	bool Start();
	update_status Update();

	void LoadFont(const char * path, int size, std::map<char, Character>& charactersBitmap);

public:

private:
	FT_Library library;   /* handle to library     */


};

#endif //__MODULE_FREETYPE_H__
