#include "ComponentUIAnimation.h"

#ifndef GAMEMODE
#include "imgui\imgui.h"
#endif


ComponentUIAnimation::ComponentUIAnimation(GameObject * parent, bool includeComponents) :
	Component(parent, ComponentTypes::UIAnimationComponent)
{
}

ComponentUIAnimation::ComponentUIAnimation(const ComponentUIAnimation & component_ui_anim, GameObject * parent, bool includeComponents) :
	Component(parent, ComponentTypes::UIAnimationComponent)
{

	// TODO end this

	if (!keys.empty() && includeComponents)
		current_key = &keys.front();
}

ComponentUIAnimation::~ComponentUIAnimation()
{
}

void ComponentUIAnimation::Update()
{
	//float dt = App->timeManager->GetDt();
}

uint ComponentUIAnimation::GetInternalSerializationBytes()
{
	return uint();
}

void ComponentUIAnimation::OnInternalSave(char *& cursor)
{
}

void ComponentUIAnimation::OnInternalLoad(char *& cursor)
{
}

void ComponentUIAnimation::OnUniqueEditor()
{
#ifndef GAMEMODE
	if (ImGui::CollapsingHeader("UI Animation", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text("UI Animation");

		if (!keys.empty()) {

			if (current_key) {
				ImGui::Text("Key rect X: %i Y: %i W: %i H:%i", 
					current_key->rect[0], current_key->rect[1], current_key->rect[2], current_key->rect[3]);
				ImGui::Text("Time to key: %f", current_key->time_to_key);
			}
			

			if (ImGui::Button("Play"))
				ImGui::Text("UI Animation");

			ImGui::SameLine();

			if (ImGui::Button("Pause"))
				ImGui::Text("UI Animation");

			ImGui::SameLine();

			if (ImGui::Button("Stop"))
				ImGui::Text("UI Animation");

			ImGui::SameLine();

			if (ImGui::Button("Next Key"))
				ImGui::Text("UI Animation");

			ImGui::SameLine();

			if (ImGui::Button("Previous Key"))
				ImGui::Text("UI Animation");
		}
		else {
			ImGui::Text("There is no key for this UI GO ...");
		}
	}
#endif
}

void ComponentUIAnimation::OnSystemEvent(System_Event event)
{
	switch (event.type)
	{
	case System_Event_Type::ScreenChanged:
		break;
	case System_Event_Type::CanvasChanged:
	case System_Event_Type::RectTransformUpdated:
		break;
	}
}