#include "ComponentUIAnimation.h"
#include "ComponentRectTransform.h"
#include "GameObject.h"

#ifndef GAMEMODE
#include "imgui\imgui.h"
#endif


ComponentUIAnimation::ComponentUIAnimation(GameObject * parent, bool includeComponents) :
	Component(parent, ComponentTypes::UIAnimationComponent)
{
	if (includeComponents)
	{
		change_origin_rect = true;
	}
}

ComponentUIAnimation::ComponentUIAnimation(const ComponentUIAnimation & component_ui_anim, GameObject * parent, bool includeComponents) :
	Component(parent, ComponentTypes::UIAnimationComponent)
{
	version = component_ui_anim.version;
	keys = component_ui_anim.keys;

	// TODO end this
	if (includeComponents)
	{
		if (!keys.empty())
			current_key = &keys.front();
	
		change_origin_rect = true;
	}
}

ComponentUIAnimation::~ComponentUIAnimation()
{

	parent->cmp_uiAnimation = nullptr;
}

void ComponentUIAnimation::Update()
{
	//float dt = App->timeManager->GetDt();

	if (change_origin_rect)
	{
		memcpy(init_rect, parent->cmp_rectTransform->GetRect(), sizeof(int) * 4);
		change_origin_rect = false;
	}
}

bool ComponentUIAnimation::IsRecording() const
{
	return recording;
}

uint ComponentUIAnimation::GetInternalSerializationBytes()
{
	uint keys_size = (!keys.empty()) ? (*keys.begin()).GetInternalSerializationBytes() * keys.size() : 0;
	return keys_size + sizeof(uint) + sizeof(versionSerialization);
}

void ComponentUIAnimation::OnInternalSave(char *& cursor)
{
	size_t bytes = sizeof(versionSerialization);
	memcpy(cursor, &version, bytes);
	cursor += bytes;

	bytes = sizeof(uint);
	uint keys_size = (!keys.empty()) ? keys.size() : 0;
	memcpy(cursor, &keys_size, bytes);
	cursor += bytes;

	if (keys_size > 0)
	{
		for (std::list<Key>::iterator it = keys.begin(); it != keys.end(); ++it)
			(*it).OnInternalSave(cursor);
	}
}

void ComponentUIAnimation::OnInternalLoad(char *& cursor)
{
	size_t bytes = sizeof(versionSerialization);
	memcpy(&version, cursor, bytes);
	cursor += bytes;

	switch (version)
	{
	case ComponentUIAnimation::v1:

		bytes = sizeof(uint);
		uint keys_size;
		memcpy(&keys_size, cursor, bytes);
		cursor += bytes;
		
		if (keys_size > 0)
		{
			for (uint i = 0; i < keys_size; i++)
			{
				Key nkey;
				nkey.OnInternalLoad(cursor);
				keys.push_back(nkey);
			}
		}
		break;
	}

	if (parent->includeModuleComponent)
	{
		change_origin_rect = true;
	}
}

void ComponentUIAnimation::OnUniqueEditor()
{
#ifndef GAMEMODE
	if (ImGui::CollapsingHeader("UI Animation", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text("UI Animation");

		if (!keys.empty()) {

			//TODO SELECT ANY KEY with a dropdown or smth

			if (current_key) {
				ImGui::Text("Key rect X: %i Y: %i W: %i H:%i", 
					current_key->diffRect[0], current_key->diffRect[1], current_key->diffRect[2], current_key->diffRect[3]);
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

		if(ImGui::Button((recording) ? "Stop recording" : "Start recording")) {
			recording = !recording;
		}
		

		if (ImGui::Button("Save Key"))
			this->AddKey();
	}
#endif
}

void ComponentUIAnimation::OnSystemEvent(System_Event event)
{
	switch (event.type)
	{
	case System_Event_Type::ScreenChanged:
	case System_Event_Type::CanvasChanged:
	case System_Event_Type::RectTransformUpdated:
		change_origin_rect = true;
		break;
	}
}

void ComponentUIAnimation::SetupInitPosition()
{
	if (keys.empty()) {
		Key tmp_key;
		memcpy(tmp_key.diffRect, parent->cmp_rectTransform->GetRect(), sizeof(int) * 4);
		tmp_key.time_to_key = 1000.0f;
		keys.push_back(tmp_key);

		current_key = &tmp_key;
	}
	else {

	}
	
}

void ComponentUIAnimation::AddKey()
{

}
