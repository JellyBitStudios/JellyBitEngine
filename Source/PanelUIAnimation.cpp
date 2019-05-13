#include "PanelUIAnimation.h"

#ifndef GAMEMODE

#include "Application.h"
#include "ModuleUI.h"
#include "ModuleScene.h"
#include "CurrentSelection.h"

#include "GameObject.h"

#include "ComponentUIAnimation.h"

#include "ImGui\imgui.h"

#include <list>

PanelUIAnimation::PanelUIAnimation(const char* name) : Panel(name) { }

PanelUIAnimation::~PanelUIAnimation() { }

bool PanelUIAnimation::Draw()
{
	ImGuiWindowFlags editFlags = 0;
	editFlags |= ImGuiWindowFlags_NoFocusOnAppearing;
	
	GameObject* selected_go = App->scene->selectedObject.GetCurrGameObject();
	// check selected go have the same component, if not change
	if (selected_go)
	{
		if (selected_go->cmp_uiAnimation != current_cmp)
			current_cmp = selected_go->cmp_uiAnimation;
	}
	else
		current_cmp = nullptr;

	if (ImGui::Begin(name, &enabled, ImVec2(300,400), -1.0f, editFlags))
	{
		//show keys if component is selected
		if (current_cmp)
			current_cmp->ImGuiKeys();
		else
			ImGui::Text("No GameObject selected\nwith UIComponent Animation");
	}
	ImGui::End();

	return true;
}

//Call from component UI Animation
bool PanelUIAnimation::CheckItsMe(ComponentUIAnimation * cmp)
{
	return (cmp == current_cmp);
}
#endif