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

	uint key_size = component_ui_anim.keys.size();
	if (key_size > 0u) {
		char buffer[sizeof(int) * 4 + sizeof(float)];
		char* cursor = buffer;

		std::list<Key*> tmp_list = component_ui_anim.keys;
		for (std::list<Key*>::iterator it = tmp_list.begin(); it != tmp_list.end(); ++it) {
			Key* tmp_key = new Key();
			(*it)->OnInternalSave(cursor);
			cursor = buffer;
			tmp_key->OnInternalLoad(cursor);
			cursor = buffer;

			keys.push_back(tmp_key);
		}
	}
	

	// TODO end this
	if (includeComponents)
	{
		if (!keys.empty())
			current_key = keys.front();
	
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

	if (calculate_keys_global_pos)
	{
		int rParent[4];
		memcpy(rParent, parent->cmp_rectTransform->GetRect(), sizeof(int) * 4);

		for (std::list<Key*>::iterator it = keys.begin(); it != keys.end(); ++it)
		{
			(*it)->globalRect[0] = (*it)->diffRect[0] + rParent[0];
			(*it)->globalRect[1] = (*it)->diffRect[1] + rParent[1];
			(*it)->globalRect[2] = (*it)->diffRect[2] + rParent[2];
			(*it)->globalRect[3] = (*it)->diffRect[3] + rParent[3];
		}

		calculate_keys_global_pos = false;
	}
}

bool ComponentUIAnimation::IsRecording() const
{
	return recording;
}

uint ComponentUIAnimation::GetInternalSerializationBytes()
{
	uint keys_size = (!keys.empty()) ? (*keys.begin())->GetInternalSerializationBytes() * keys.size() : 0;
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
		for (std::list<Key*>::iterator it = keys.begin(); it != keys.end(); ++it)
			(*it)->OnInternalSave(cursor);
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
				Key* nkey = new Key();
				nkey->OnInternalLoad(cursor);
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

			if (!recording) {
				parent->cmp_rectTransform->SetRect(init_rect, true);
			}
		}
		
		if (recording) {
			if (ImGui::Button("Save Key"))
				this->AddKey();
		}
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

void ComponentUIAnimation::AddKey()
{
	Key* tmp_key = new Key();

	memcpy((*tmp_key).diffRect, parent->cmp_rectTransform->GetRect(), sizeof(int) * 4);

	tmp_key->diffRect[0] -= init_rect[0];
	tmp_key->diffRect[1] -= init_rect[1];
	tmp_key->diffRect[2] -= init_rect[2];
	tmp_key->diffRect[3] -= init_rect[3];

	tmp_key->time_to_key = 1000.0f;
	keys.push_back(tmp_key);

	current_key = keys.back();
}
