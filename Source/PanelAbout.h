#ifndef __PANEL_ABOUT_H__
#define __PANEL_ABOUT_H__



#ifndef GAMEMODE

#include "Panel.h"

class PanelAbout : public Panel
{
public:

	PanelAbout(const char* name);
	~PanelAbout();

	bool Draw();
};

#endif

#endif // GAME
