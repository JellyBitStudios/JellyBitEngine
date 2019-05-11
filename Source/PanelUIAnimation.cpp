#include "PanelUIAnimation.h"

#ifndef GAMEMODE

#include "Application.h"
#include "ModuleUI.h"

#include "ComponentUIAnimation.h"

#include "ImGui\imgui.h"

#include <list>

PanelUIAnimation::PanelUIAnimation(const char* name) : Panel(name) {  }

PanelUIAnimation::~PanelUIAnimation() { }

bool PanelUIAnimation::Draw()
{
	ImGuiWindowFlags editFlags = 0;
	editFlags |= ImGuiWindowFlags_NoFocusOnAppearing;

	if (ImGui::Begin(name, &enabled, editFlags))
	{
		if (current_cmp)
		{
			current_cmp->ImGuiKeys();
		}
	}
	ImGui::End();

	return true;
}

void PanelUIAnimation::SetCmp(ComponentUIAnimation * cmp)
{
	current_cmp = cmp;
	if (!IsEnabled())
		OnOff();
}

void PanelUIAnimation::ClearCmp()
{
	current_cmp = nullptr;
	if (IsEnabled())
		OnOff();
}

#endif