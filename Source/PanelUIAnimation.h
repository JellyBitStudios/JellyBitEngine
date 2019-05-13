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

	//Call from component UI Animation
	bool CheckItsMe(ComponentUIAnimation* cmp);

private:
	//storage the direction of component for call the imgui keys.
	ComponentUIAnimation* current_cmp = nullptr;
	
};

#endif

#endif