#ifndef __PANEL_PHYSICS_H__
#define __PANEL_PHYSICS_H__



#ifndef GAMEMODE

#include "Panel.h"

class PanelPhysics : public Panel
{
public:

	PanelPhysics(const char* name);
	~PanelPhysics();

	bool Draw();
};

#endif

#endif