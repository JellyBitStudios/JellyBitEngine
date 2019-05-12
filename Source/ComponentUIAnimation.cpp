#include "ComponentUIAnimation.h"

#include "Application.h"
#include "ModuleTimeManager.h"
#include "ModuleGui.h"

#include "PanelUIAnimation.h"

#include "GameObject.h"
#include "ComponentRectTransform.h"

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
		if (App->gui->panelUIAnimation->IsEnabled())
			usePanel = true;

		change_origin_rect = true;
	}
}

ComponentUIAnimation::ComponentUIAnimation(const ComponentUIAnimation & component_ui_anim, GameObject * parent, bool includeComponents) :
	Component(parent, ComponentTypes::UIAnimationComponent)
{
	animation_time = component_ui_anim.animation_time;
	repeat = component_ui_anim.repeat;

	uint key_size = component_ui_anim.keys.size();
	if (key_size > 0u) {
		std::map<uint, Key*> tmp_list = component_ui_anim.keys;
		Key* last_key = nullptr;
		for (std::map<uint, Key*>::iterator it = tmp_list.begin(); it != tmp_list.end(); ++it) {

			Key* tmp_key = new Key(*it->second);

			tmp_key->id = keys.size();
			if (last_key)
			{
				last_key->next_key = tmp_key;
				tmp_key->back_key = last_key;
			}

			last_key = tmp_key;
			keys.insert(std::pair<uint, Key*>(tmp_key->id, tmp_key));
			AddKeyOnCombo();
		}
	}
	

	// TODO end this
	if (includeComponents)
	{
		if (!keys.empty())
			current_key = keys.at(0);

		if (App->gui->panelUIAnimation->IsEnabled())
			usePanel = true;
	
		change_origin_rect = true;
		recalculate_times = true;
	}
}

ComponentUIAnimation::~ComponentUIAnimation()
{
	for (char* s : keys_strCombo)
		RELEASE_ARRAY(s);
	keys_strCombo.clear();

	for (std::map<uint, Key*>::iterator it = keys.begin(); it != keys.end(); ++it)
		RELEASE(it->second);
	keys.clear();

	parent->cmp_uiAnimation = nullptr;
}

void ComponentUIAnimation::Update()
{
	if (change_origin_rect)
	{
		memcpy(init_rect, parent->cmp_rectTransform->GetRect(), sizeof(int) * 4);
		calculate_keys_global = true;
		change_origin_rect = false;
	}

	if (calculate_keys_global)
	{
		for (std::map<uint, Key*>::iterator it = keys.begin(); it != keys.end(); ++it)
		{
			Key* k = it->second;
			k->globalRect[0] = k->diffRect[0] + init_rect[0];
			k->globalRect[1] = k->diffRect[1] + init_rect[1];
			k->globalRect[2] = k->diffRect[2] + init_rect[2];
			k->globalRect[3] = k->diffRect[3] + init_rect[3];
		}

		calculate_keys_global = false;
	}

	if (recalculate_times) 
	{

		for (std::map<uint, Key*>::iterator it = keys.begin(); it != keys.end(); ++it)
		{
			it->second->global_time = animation_time * it->second->time_key;
		}

		recalculate_times = false;
	}

	// Interpolate
	if (keys.empty() || keys.size() == 1)
		return;

	float dt = (App->GetEngineState() == engine_states::ENGINE_PLAY) ? App->timeManager->GetDt() : App->GetDt();

	switch (animation_state)
	{
		case UIAnimationState::PLAYING: {

			if (animation_timer >= current_key->global_time) {
				if (current_key->next_key != nullptr)
					current_key = current_key->next_key;
				else
				{
					animation_state = UIAnimationState::PAUSED;
					break;
				}
			}

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
	int tmp_rect[4] = {
		time * current_key->globalRect[0] / current_key->time_key,
		time * current_key->globalRect[1] / current_key->time_key,
		time * current_key->globalRect[2] / current_key->time_key,
		time * current_key->globalRect[3] / current_key->time_key };
	parent->cmp_rectTransform->SetRect(tmp_rect, true);
}

bool ComponentUIAnimation::IsRecording() const
{
	return recording;
}

uint ComponentUIAnimation::GetInternalSerializationBytes()
{
	uint keys_size = (!keys.empty()) ? keys.at(0)->GetInternalSerializationBytes() * keys.size() : 0;
	return keys_size + sizeof(float) + sizeof(bool) + sizeof(uint) + sizeof(versionSerialization);
}

void ComponentUIAnimation::OnInternalSave(char *& cursor)
{
	size_t bytes = sizeof(versionSerialization);
	memcpy(cursor, &version, bytes);
	cursor += bytes;

	bytes = sizeof(float);
	memcpy(cursor, &animation_time, bytes);
	cursor += bytes;

	bytes = sizeof(bool);
	memcpy(cursor, &repeat, bytes);
	cursor += bytes;

	bytes = sizeof(uint);
	uint keys_size = (!keys.empty()) ? keys.size() : 0;
	memcpy(cursor, &keys_size, bytes);
	cursor += bytes;

	if (keys_size > 0)
	{
		for (std::map<uint, Key*>::iterator it = keys.begin(); it != keys.end(); ++it)
			it->second->OnInternalSave(cursor);
	}
}

void ComponentUIAnimation::OnInternalLoad(char *& cursor)
{
	size_t bytes = sizeof(versionSerialization);
	versionSerialization loadVersion;
	memcpy(&loadVersion, cursor, bytes);
	cursor += bytes;

	switch (loadVersion)
	{
	case ComponentUIAnimation::v1:

		bytes = sizeof(float);
		memcpy(&animation_time, cursor, bytes);
		cursor += bytes;

		bytes = sizeof(bool);
		memcpy(&repeat, cursor, bytes);
		cursor += bytes;

		bytes = sizeof(uint);
		uint keys_size;
		memcpy(&keys_size, cursor, bytes);
		cursor += bytes;
		
		if (keys_size > 0)
		{
			Key* last_key = nullptr;
			for (uint i = 0; i < keys_size; i++)
			{
				Key* nkey = new Key();
				nkey->OnInternalLoad(cursor);

				nkey->id = keys.size();

				if (last_key)
				{
					last_key->next_key = nkey;
					nkey->back_key = last_key;
				}
				
				keys.insert(std::pair<uint, Key*>(nkey->id, nkey));
				AddKeyOnCombo();
			}
		}
		break;
	}

	if (parent->includeModuleComponent)
	{
		change_origin_rect = true;
		recalculate_times = true;
	}
}

void ComponentUIAnimation::OnUniqueEditor()
{
#ifndef GAMEMODE
	if (ImGui::CollapsingHeader("UI Animation", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (!keys.empty()) {

			ImGui::PushItemWidth(100.0f);
			if (ImGui::DragFloat("Total time", &animation_time))
				recalculate_times = true;

			switch (animation_state)
			{
			case UIAnimationState::PAUSED:
					ImGui::Text("Paused");
				break;
				case UIAnimationState::PLAYING:
					ImGui::Text("Playing");
				break;
				case UIAnimationState::STOPPED:
					ImGui::Text("Stopped");
				break;
			}
			ImGui::Text("Current timer: %f", animation_timer);

			bool change_rect_pos = false;
			int selectable_key = (current_key) ? current_key->id : 0;
			ImGui::PushItemWidth(75.0f);
			if (ImGui::Combo("Current", &selectable_key, keys_strCombo.data(), keys.size()))
			{
				modifying_key = false;
				parent->cmp_rectTransform->SetRect(init_rect, true);
				current_key = keys.at(selectable_key);
				change_rect_pos = true;
			}

			if (current_key->back_key != nullptr)
			{
				ImGui::SameLine();
				if (ImGui::ArrowButton("ToLeft", ImGuiDir_::ImGuiDir_Left))
				{
					modifying_key = false;
					current_key = current_key->back_key;
					parent->cmp_rectTransform->SetRect(init_rect, true);
					change_rect_pos = true;
				}
			}

			if (current_key->next_key != nullptr)
			{
				ImGui::SameLine();
				if (ImGui::ArrowButton("ToRight", ImGuiDir_::ImGuiDir_Right))
				{
					modifying_key = false;
					current_key = current_key->next_key;
					parent->cmp_rectTransform->SetRect(init_rect, true);
					change_rect_pos = true;
				}

				if (recording && change_rect_pos) parent->cmp_rectTransform->SetRect(current_key->globalRect, true);
			}

			if (current_key) {
				if (!recording)
				{
					ImGui::SameLine();

					if (ImGui::Button((modifying_key) ? "Disable edition" : "Enable edition"))
					{
						modifying_key = !modifying_key;
						parent->cmp_rectTransform->SetRect((modifying_key) ? current_key->globalRect : init_rect, true);
					}
				}
				current_key->OnEditor(animation_time, true);
			}

			ImGui::Separator();
			if (ImGui::Button("Play")) {//TODO PREPARE FOR SCRIPTING
				if (keys.size() > 1)
				{
					animation_state = UIAnimationState::PLAYING;
					current_key = keys.at(0);
					animation_timer = keys.at(0)->global_time;
					parent->cmp_rectTransform->SetRect(keys.at(0)->globalRect, true);
				}
				else
				{
					CONSOLE_LOG(LogTypes::Warning, "You can't play with one key.");
				}
			}
			ImGui::SameLine();

			if (ImGui::Button("Pause"))
				animation_state = UIAnimationState::PAUSED;

			ImGui::SameLine();

			if (ImGui::Button("Stop")) {//TODO PREPARE FOR SCRIPTING
				animation_state = UIAnimationState::STOPPED;
				current_key = keys.at(0);
				parent->cmp_rectTransform->SetRect(init_rect, true);
			}

			ImGui::SameLine();

			if (ImGui::Button("Next Key")) {
				if (current_key->next_key != nullptr)
				{
					current_key = current_key->next_key;
					animation_timer = current_key->global_time;
					parent->cmp_rectTransform->SetRect(current_key->globalRect, true);
				}
			}

			ImGui::SameLine();

			if (ImGui::Button("Previous Key")) {
				if (current_key->back_key != nullptr)
				{
					current_key = current_key->back_key;
					animation_timer = current_key->global_time;
					parent->cmp_rectTransform->SetRect(current_key->globalRect, true);
				}
			}
		}
		else {
			ImGui::Text("There is no key for this UI GO ...");
		}

		bool popStyle = false;
		if (popStyle = recording) ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Button, ImVec4(1.0, 0.0, 0.0, 0.8));
		if(ImGui::Button((recording) ? "Stop recording" : "Start recording")) {
			recording = !recording;

			if(recording)
				App->gui->panelUIAnimation->SetOnOff(usePanel = true);

			if (!keys.empty())
			{
				if (recording){
					modifying_key = false;
					current_key = keys.at(keys.size() - 1);
					parent->cmp_rectTransform->SetRect(current_key->globalRect, true);
				}

				if (!recording) {
					current_key = keys.at(0);
					parent->cmp_rectTransform->SetRect(init_rect, true);
				}
			}
		}
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::Text("REMEMBER STOP RECORDING\nBEFORE EDIT OTHER ELEMENTS!");
			ImGui::EndTooltip();
		}

		if (popStyle) ImGui::PopStyleColor();
		
		if (recording) {
			ImGui::SameLine();
			if (ImGui::Button("Save Key"))
				this->AddKey();
		}

		usePanel = (App->gui->panelUIAnimation->CheckItsMe(this) && App->gui->panelUIAnimation->IsEnabled()) ? true : false;
		if (ImGui::Button((usePanel) ? "Hide panel" : "Show Panel"))
			App->gui->panelUIAnimation->SetOnOff(usePanel = !usePanel);

		ImGui::PopItemWidth();
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
		if (modifying_key)
		{
			memcpy(current_key->globalRect, parent->cmp_rectTransform->GetRect(), sizeof(int) * 4);

			current_key->diffRect[0] = current_key->globalRect[0] - init_rect[0];
			current_key->diffRect[1] = current_key->globalRect[1] - init_rect[1];
			current_key->diffRect[2] = current_key->globalRect[2] - init_rect[2];
			current_key->diffRect[3] = current_key->globalRect[3] - init_rect[3];
		}
		else
			change_origin_rect = true;
		break;
	}
}

float ComponentUIAnimation::GetAnimationTime() const
{
	return animation_time;
}

bool ComponentUIAnimation::HasKeys() const
{
	return !keys.empty();
}

void ComponentUIAnimation::ImGuiKeys()
{
#ifndef GAMEMODE
	if (!keys.empty())
	{
		for (uint i = 0; i < keys.size(); i++)
		{
			std::string t = "Set key "; t += std::to_string(i + 1); t += " to current";
			if (ImGui::Button(t.c_str()))
			{
				current_key = keys.at(i);
			
				if (recording) parent->cmp_rectTransform->SetRect(current_key->globalRect, true);
			}
			ImGui::SameLine();
			ComponentUIAnimation::key_editor returned = keys.at(i)->OnEditor(animation_time);
			switch (returned)
			{
			case key_editor::SwapedUp:
			{
				Key* k1 = keys.at(i);
				Key* k2 = k1->next_key;

				keys.erase(i);
				keys.erase(i - 1);

				keys.insert(std::pair<uint, Key*>(k1->id, k1));
				keys.insert(std::pair<uint, Key*>(k2->id, k2));

				break;
			}
			case key_editor::SwapedDown:
			{
				Key* k1 = keys.at(i);
				Key* k2 = k1->back_key;

				keys.erase(i);
				keys.erase(i + 1);

				keys.insert(std::pair<uint, Key*>(k1->id, k1));
				keys.insert(std::pair<uint, Key*>(k2->id, k2));

				break;
			}

			}
		}
	}
	else
		ImGui::Text("Empty Keys");
#endif
}

void ComponentUIAnimation::AddKey()
{
	Key* tmp_key = new Key();

	tmp_key->id = keys.size();

	memcpy((*tmp_key).globalRect, parent->cmp_rectTransform->GetRect(), sizeof(int) * 4);

	tmp_key->diffRect[0] = tmp_key->globalRect[0] - init_rect[0];
	tmp_key->diffRect[1] = tmp_key->globalRect[1] - init_rect[1];
	tmp_key->diffRect[2] = tmp_key->globalRect[2] - init_rect[2];
	tmp_key->diffRect[3] = tmp_key->globalRect[3] - init_rect[3];

	tmp_key->time_key = 0.0f;

	if (!keys.empty()) {
		tmp_key->back_key = keys.at(tmp_key->id - 1);
		tmp_key->back_key->next_key = tmp_key;
	}

	keys.insert(std::pair<uint, Key*>(tmp_key->id, tmp_key));
	current_key = tmp_key;

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

//OnEditor Key
ComponentUIAnimation::key_editor ComponentUIAnimation::Key::OnEditor(float anim_time, bool isCurrent)
{
	key_editor ret = key_editor::NONE;
#ifndef GAMEMODE
	ImGui::Separator();
	std::string id_str = std::to_string(id + 1);
	std::string tmp;
	ImGui::Text("Key: %i", id + 1); 
	if (!isCurrent)
	{
		if (back_key != nullptr)
		{
			ImGui::SameLine();
			tmp = "AU"; tmp += id_str;
			if (ImGui::ArrowButton(tmp.c_str(), ImGuiDir_::ImGuiDir_Up))
			{
				Swap(back_key);
				global_time = anim_time * time_key;
				ret = key_editor::SwapedUp;
			}
		}
		if (next_key != nullptr)
		{
			ImGui::SameLine();
			tmp = "AD"; tmp += id_str;
			if (ImGui::ArrowButton(tmp.c_str(), ImGuiDir_::ImGuiDir_Down))
			{
				Swap(next_key);
				global_time = anim_time * time_key;
				ret = key_editor::SwapedDown;
			}
		}
	}
	ImGui::Text("DiffRect X: %i Y: %i W: %i H:%i",
		diffRect[0], diffRect[1], diffRect[2], diffRect[3]);
	ImGui::Text("GlobalRect X: %i Y: %i W: %i H:%i",
		globalRect[0], globalRect[1], globalRect[2], globalRect[3]);
	tmp = "Time key "; tmp += id_str;
	if (ImGui::SliderFloat(tmp.c_str(), &time_key, 0.0f, 1.0f)) {
		(back_key != nullptr && time_key < back_key->time_key) ? time_key = back_key->time_key : time_key;
		global_time = anim_time * time_key;
		CheckNextKeyTime();
	}
	ImGui::Text("Global time: %f", global_time);
#endif
	return ret;
}
