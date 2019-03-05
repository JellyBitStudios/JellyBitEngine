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
};

#endif //__MODULE_FREETYPE_H__
