#ifndef __PANEL_UIANIMATION_H__
#define __PANEL_UIANIMATION_H__

#ifndef GAMEMODE

#include "Panel.h"

class ComponentUIAnimation;

class PanelUIAnimation : public Panel
{
public:
	PanelUIAnimation(const char* name);
	~PanelUIAnimation();

	bool Draw();

	bool CheckItsMe(ComponentUIAnimation* cmp);

private:
	ComponentUIAnimation* current_cmp = nullptr;
	
};

#endif

#endif