#include "PanelUI.h"

#ifndef GAMEMODE

#include "Application.h"
#include "ModuleUI.h"

#include "ImGui\imgui.h"

PanelUI::PanelUI(const char* name) : Panel(name) {}

PanelUI::~PanelUI() { }

bool PanelUI::Draw()
{
	ImGuiWindowFlags editFlags = 0;
	editFlags |= ImGuiWindowFlags_NoFocusOnAppearing;

	if (ImGui::Begin(name, &enabled, editFlags))
	{
		ImGui::Checkbox("Move Screen Canvas", &moveScreenCanvas);
		if (moveScreenCanvas)
		{
			math::float3 pos = App->ui->GetPositionWH();
			if (ImGui::DragFloat3("##Pos", &pos[0], 0.01f, 0.0f, 0.0f, "%.3f"))
				App->ui->SetPositionWH(pos);
		}
	}
	ImGui::End();

	return true;
}

#endif