#ifndef __PANEL_UI_H__
#define __PANEL_UI_H__

#include "Panel.h"

#ifndef GAMEMODE

class PanelUI : public Panel
{
public:
	PanelUI(const char* name);
	~PanelUI();

	bool Draw();

private:
	bool moveScreenCanvas = false;
};
#endif

#endif