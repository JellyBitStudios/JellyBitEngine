#include "ComponentUIAnimation.h"
#include "ComponentRectTransform.h"
#include "GameObject.h"
#include "Application.h"
#include "ModuleTimeManager.h"

#ifndef GAMEMODE
#include "imgui\imgui.h"
#endif

#define KEY_STR "Key "
#define NULL_STR "\0"
#define SIZE_STR_KEY 10

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
			AddKeyOnCombo();
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
	for (char* s : keys_strCombo)
		RELEASE_ARRAY(s);
	keys_strCombo.clear();

	parent->cmp_uiAnimation = nullptr;
}

void ComponentUIAnimation::Update()
{
	float dt = App->timeManager->GetDt();

	if (change_origin_rect)
	{
		memcpy(init_rect, parent->cmp_rectTransform->GetRect(), sizeof(int) * 4);
		calculate_keys_global = true;
		change_origin_rect = false;
	}

	if (calculate_keys_global)
	{
		for (std::list<Key*>::iterator it = keys.begin(); it != keys.end(); ++it)
		{
			(*it)->globalRect[0] = (*it)->diffRect[0] + init_rect[0];
			(*it)->globalRect[1] = (*it)->diffRect[1] + init_rect[1];
			(*it)->globalRect[2] = (*it)->diffRect[2] + init_rect[2];
			(*it)->globalRect[3] = (*it)->diffRect[3] + init_rect[3];
		}

		calculate_keys_global = false;
	}

	if (recalculate_times) 
	{

		for (std::list<Key*>::iterator it = keys.begin(); it != keys.end(); ++it)
		{
			(*it)->global_time = animation_time * (*it)->time_key;
		}

		recalculate_times = false;
	}

	// Interpolate
	if (!current_key)
		return;

	if (animation_timer > current_key->global_time) {
		std::list<Key*>::iterator it = std::find(keys.begin(), keys.end(), current_key);
		if (it++ != keys.end())
			current_key = (*it);
		else
			animation_state = UIAnimationState::PAUSED;
	}

	switch (animation_state)
	{
		case UIAnimationState::PLAYING: {
			animation_timer += dt;
			Interpolate(animation_timer);
		}
		break;

		case UIAnimationState::PAUSED:
		break;

		case UIAnimationState::STOPPED: {
			animation_timer = 0.0f;
		}
		break;
	}
}

void ComponentUIAnimation::Interpolate(float time)
{
	std::list<Key*>::iterator it = std::find(keys.begin(), keys.end(), current_key);

	if (it != keys.end()) {

		it++;
		if (it == keys.end()) {
			animation_state = UIAnimationState::PAUSED;
			return;
		}

		Key* next_key = (*it);

		int tmp_rect[4] = {
			time * next_key->globalRect[0] / next_key->time_key,
			time * next_key->globalRect[1] / next_key->time_key,
			time * next_key->globalRect[2] / next_key->time_key,
			time * next_key->globalRect[3] / next_key->time_key };

		parent->cmp_rectTransform->SetRect(tmp_rect, true);

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
				AddKeyOnCombo();
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

			ImGui::PushItemWidth(100.0f);
			if (ImGui::DragFloat("Total time", &animation_time))
				recalculate_times = true;

			ImGui::Text("Current timer: %f", animation_timer);

			int selectable_key = current_key_int;
			ImGui::PushItemWidth(75.0f);
			if (ImGui::Combo("Select", &selectable_key, keys_strCombo.data(), keys.size()));
			{
				current_key_int = selectable_key;
				uint count = 0u;
				for (std::list<Key*>::iterator it = keys.begin(); it != keys.end(); ++it, count++) {
					if (count == selectable_key) {
						current_key = (*it);
						break;
					}
				}
			}

			if (current_key) {
				ImGui::Separator();
				ImGui::Text("Current: "); ImGui::SameLine(); ImGui::Text(keys_strCombo[current_key_int]);
				ImGui::Text("DiffRect X: %i Y: %i W: %i H:%i", 
					current_key->diffRect[0], current_key->diffRect[1], current_key->diffRect[2], current_key->diffRect[3]);
				ImGui::Text("GlobalRect X: %i Y: %i W: %i H:%i",
					current_key->globalRect[0], current_key->globalRect[1], current_key->globalRect[2], current_key->globalRect[3]);

				if (ImGui::SliderFloat("Time", &current_key->time_key, 0.0f, 1.0f)) {
					current_key->global_time = this->animation_time * current_key->time_key;
				}

				ImGui::SameLine();

				ImGui::Text("| Global time: %f", current_key->global_time);
			}

			/*uint count = 1;
			for (std::list<Key*>::iterator it = keys.begin(); it != keys.end(); ++it, ++count)
			{
				ImGui::Separator();
				Key* k = *it;
				ImGui::Text("Key %u", count);
				ImGui::Text("DiffRect X: %i Y: %i W: %i H:%i",
					k->diffRect[0], k->diffRect[1], k->diffRect[2], k->diffRect[3]);
				ImGui::Text("GlobalRect X: %i Y: %i W: %i H:%i",
					k->globalRect[0], k->globalRect[1], k->globalRect[2], k->globalRect[3]);
				ImGui::Text("Time to key: %f", k->time_key);
			}*/
			ImGui::Separator();
			if (ImGui::Button("Play")) {//TODO PREPARE FOR SCRIPTING
				animation_state = UIAnimationState::PLAYING;
				current_key = keys.front();
			}
			ImGui::SameLine();

			if (ImGui::Button("Pause"))
				animation_state = UIAnimationState::PAUSED;

			ImGui::SameLine();

			if (ImGui::Button("Stop")) {//TODO PREPARE FOR SCRIPTING
				animation_state = UIAnimationState::STOPPED;
				parent->cmp_rectTransform->SetRect(init_rect, true);
			}

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

	memcpy((*tmp_key).globalRect, parent->cmp_rectTransform->GetRect(), sizeof(int) * 4);

	tmp_key->diffRect[0] = tmp_key->globalRect[0] - init_rect[0];
	tmp_key->diffRect[1] = tmp_key->globalRect[1] - init_rect[1];
	tmp_key->diffRect[2] = tmp_key->globalRect[2] - init_rect[2];
	tmp_key->diffRect[3] = tmp_key->globalRect[3] - init_rect[3];

	tmp_key->time_key = 0.0f;

	current_key_int = keys.size();
	keys.push_back(tmp_key);
	current_key = keys.back();

	AddKeyOnCombo();
}

void ComponentUIAnimation::AddKeyOnCombo()
{
	uint total_keys = keys.size();
	std::string total_keys_str = std::to_string(total_keys);
	uint number_size = (total_keys > 9) ? 2 : 1;

	char* key_str = new char[SIZE_STR_KEY];
	char* cursorkey = key_str;

	size_t bytes = sizeof(char) * 4;
	memcpy(cursorkey, KEY_STR, bytes);
	cursorkey += bytes;
	bytes = sizeof(char) * number_size;
	memcpy(cursorkey, total_keys_str.c_str(), bytes);
	cursorkey += bytes;
	bytes = sizeof(char) * 2;
	memcpy(cursorkey, NULL_STR, bytes);
	cursorkey += bytes;

	keys_strCombo.push_back(key_str);
}
