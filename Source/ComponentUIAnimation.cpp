#include "ComponentUIAnimation.h"

#include "Application.h"
#include "ModuleTimeManager.h"
#include "ModuleGui.h"

#include "PanelUIAnimation.h"

#include "GameObject.h"
#include "ComponentRectTransform.h"
#include "ComponentImage.h"

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
		if (!parent->cmp_image) parent->AddComponent(ComponentTypes::ImageComponent);

#ifndef GAMEMODE
		if (App->gui->panelUIAnimation->IsEnabled())
			usePanel = true;
#endif

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
		if (!parent->cmp_image) parent->AddComponent(ComponentTypes::ImageComponent);

		if (!keys.empty())
			current_key = keys.at(0);

#ifndef GAMEMODE
		if (App->gui->panelUIAnimation->IsEnabled())
			usePanel = true;
#endif
	
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

		if (parent->cmp_image) init_alpha = parent->cmp_image->GetAlpha();
	}

	if (calculate_keys_global)
	{
		for (std::map<uint, Key*>::iterator it = keys.begin(); it != keys.end(); ++it)
		{
			Key* k = it->second;
			k->globalRect[0] = k->diffRect[0] + ((k->back_key) ? k->back_key->globalRect[0] : init_rect[0]);
			k->globalRect[1] = k->diffRect[1] + ((k->back_key) ? k->back_key->globalRect[1] : init_rect[1]);
			k->globalRect[2] = k->diffRect[2] + ((k->back_key) ? k->back_key->globalRect[2] : init_rect[2]);
			k->globalRect[3] = k->diffRect[3] + ((k->back_key) ? k->back_key->globalRect[3] : init_rect[3]);
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
				if (current_key->next_key != nullptr) {
					current_key = current_key->next_key;
				}
				else
				{
					animation_state = UIAnimationState::PAUSED;

					if (App->GetEngineState() == engine_states::ENGINE_EDITOR && !repeat)
						Stop();
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
	float diff_time = (current_key->back_key) ? current_key->back_key->global_time : 0.0f;

	int rect_increment[4] = {
		(time - diff_time) * current_key->diffRect[0] / (current_key->global_time - diff_time),
		(time - diff_time) * current_key->diffRect[1] / (current_key->global_time - diff_time),
		(time - diff_time) * current_key->diffRect[2] / (current_key->global_time - diff_time),
		(time - diff_time) * current_key->diffRect[3] / (current_key->global_time - diff_time) };
	float alpha_increment = (time - diff_time) * (current_key->alpha - ((current_key->back_key) ? current_key->back_key->alpha : init_alpha)) / (current_key->global_time - diff_time);

	int next_rect[4] = { 
		(current_key->back_key) ? current_key->back_key->globalRect[0] + rect_increment[0] : init_rect[0] + rect_increment[0], 
		(current_key->back_key) ? current_key->back_key->globalRect[1] + rect_increment[1] : init_rect[1] + rect_increment[1],
		(current_key->back_key) ? current_key->back_key->globalRect[2] + rect_increment[2] : init_rect[2] + rect_increment[2],
		(current_key->back_key) ? current_key->back_key->globalRect[3] + rect_increment[3] : init_rect[3] + rect_increment[3] };
	float next_alpha = (current_key->back_key) ? current_key->back_key->alpha + alpha_increment : init_alpha + alpha_increment;
	
	parent->cmp_rectTransform->SetRect(next_rect, true);
	if (parent->cmp_image) parent->cmp_image->SetAlpha(next_alpha);
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
				last_key = nkey;
			}
		}
		break;
	}

	if (parent->includeModuleComponent)
	{
		if (!keys.empty())
			current_key = keys.at(0);
		change_origin_rect = true;
		recalculate_times = true;
	}
}

void ComponentUIAnimation::OnUniqueEditor()
{
#ifndef GAMEMODE
	if (ImGui::CollapsingHeader("UI Animation", ImGuiTreeNodeFlags_DefaultOpen))
	{
		bool popStyle = false;

		if (!keys.empty()) {

			ImGui::PushItemWidth(100.0f);
			if (ImGui::DragFloat("Total time", &animation_time, 0.5f, 0.0f))
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
					change_rect_pos = true;
				}

				if (change_rect_pos)
					(recording) ? DrawCurrent() : DrawInit();
			}

			if (current_key) {
				if (!recording)
				{
					ImGui::SameLine();
					if (popStyle = modifying_key)
					{
						ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Button, ImVec4(1.0, 0.0, 0.0, 0.5));
						ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_ButtonHovered, ImVec4(1.0, 0.0, 0.0, 0.7));
						ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_ButtonActive, ImVec4(1.0, 0.0, 0.0, 1.0));
					}
					if (ImGui::Button((modifying_key) ? "Disable edition" : "Enable edition"))
					{
						modifying_key = !modifying_key;
						(modifying_key) ? DrawCurrent() : DrawInit();
					}
					if (ImGui::IsItemHovered())
					{
						ImGui::BeginTooltip();
						ImGui::Text("REMEMBER DISABLE EDITION\nBEFORE EDIT OTHER ELEMENTS!");
						ImGui::EndTooltip();
					}
					if (popStyle) ImGui::PopStyleColor(3);
				}
				current_key->OnEditor(animation_time, true);
			}

			ImGui::Separator();
			if (ImGui::Button("Play")) {//TODO PREPARE FOR SCRIPTING
				Play();
			}
			ImGui::SameLine();

			if (ImGui::Button("Pause"))
				animation_state = UIAnimationState::PAUSED;

			ImGui::SameLine();

			if (ImGui::Button("Stop")) {//TODO PREPARE FOR SCRIPTING
				Stop();
			}

			ImGui::SameLine();

			if (ImGui::Button("Next Key")) {
				if (current_key->next_key != nullptr)
				{
					modifying_key = false;
					current_key = current_key->next_key;
					animation_timer = current_key->global_time;
					if (recording) DrawCurrent();
				}
			}

			ImGui::SameLine();

			if (ImGui::Button("Previous Key")) {
				if (current_key->back_key != nullptr)
				{
					modifying_key = false;
					current_key = current_key->back_key;
					animation_timer = current_key->global_time;
					if (recording) DrawCurrent();
				}
			}
		}
		else {
			ImGui::Text("There is no key for this UI GO ...");
			ImGuiDradDropCopyKeys();
		}

		if (popStyle = recording)
		{
			ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Button, ImVec4(1.0, 0.0, 0.0, 0.5));
			ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_ButtonHovered, ImVec4(1.0, 0.0, 0.0, 0.7));
			ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_ButtonActive, ImVec4(1.0, 0.0, 0.0, 1.0));
		}
		if(ImGui::Button((recording) ? "Stop recording" : "Start recording")) {
			recording = !recording;

			if(recording)
				App->gui->panelUIAnimation->SetOnOff(usePanel = true);

			if (!keys.empty())
			{
				if (recording){
					modifying_key = false;
					current_key = keys.at(keys.size() - 1);
					DrawCurrent();
				}
			}
			if (!recording) {
				current_key = (!keys.empty()) ? keys.at(0) : nullptr;
				DrawInit();
			}
		}
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::Text("REMEMBER STOP RECORDING\nBEFORE EDIT OTHER ELEMENTS!");
			ImGui::EndTooltip();
		}

		if (popStyle) ImGui::PopStyleColor(3);
		
		if (recording) {
			ImGui::SameLine();
			if (ImGui::Button("Save Key"))
				this->AddKey();
		}

		usePanel = (App->gui->panelUIAnimation->CheckItsMe(this) && App->gui->panelUIAnimation->IsEnabled()) ? true : false;
		if (ImGui::Button((usePanel) ? "Hide Panel" : "Show Panel"))
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

			current_key->diffRect[0] = current_key->globalRect[0] - ((current_key->back_key) ? current_key->back_key->globalRect[0] : init_rect[0]);
			current_key->diffRect[1] = current_key->globalRect[1] - ((current_key->back_key) ? current_key->back_key->globalRect[1] : init_rect[1]);
			current_key->diffRect[2] = current_key->globalRect[2] - ((current_key->back_key) ? current_key->back_key->globalRect[2] : init_rect[2]);
			current_key->diffRect[3] = current_key->globalRect[3] - ((current_key->back_key) ? current_key->back_key->globalRect[3] : init_rect[3]);
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
				modifying_key = false;
				current_key = keys.at(i);
				if (recording) DrawCurrent();
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
			case key_editor::Deleted:
			{
				if (current_key == keys.at(i))
				{
					current_key = (keys.size() > 1) ? keys.at(0) : nullptr;

					if (modifying_key)
					{
						modifying_key = false;
						DrawInit();
					}
				}
				RELEASE(keys.at(i));
				keys.erase(i);
				for (uint next_i = i + 1; next_i < keys.size() + 1; next_i++)
				{
					Key* next = keys.at(next_i);
					keys.erase(next_i);
					keys.insert(std::pair<uint, Key*>(next->id, next));
				}
				--i;
				RELEASE_ARRAY(keys_strCombo.back());
				keys_strCombo.pop_back();
				break;
			}
			}
		}
	}
	else
		ImGui::Text("Empty Keys");
#endif
}

void ComponentUIAnimation::Play()
{
	if (keys.size() > 1)
	{
		animation_state = UIAnimationState::PLAYING;
		current_key = keys.at(0);
		animation_timer = keys.at(0)->global_time;
		DrawCurrent();
	}
	else
	{
		CONSOLE_LOG(LogTypes::Warning, "You can't play with one key.");
	}
}

void ComponentUIAnimation::Stop()
{
	animation_state = UIAnimationState::STOPPED;
	current_key = keys.at(0);
	DrawInit();
}

void ComponentUIAnimation::SetInitAlpha(float alpha)
{
	init_alpha = alpha;
}

void ComponentUIAnimation::AddKey()
{
	Key* tmp_key = new Key();

	tmp_key->id = keys.size();

	memcpy((*tmp_key).globalRect, parent->cmp_rectTransform->GetRect(), sizeof(int) * 4);

	tmp_key->time_key = 0.0f;

	if (!keys.empty()) {
		tmp_key->back_key = keys.at(tmp_key->id - 1);
		tmp_key->back_key->next_key = tmp_key;
	}

	tmp_key->diffRect[0] = tmp_key->globalRect[0] - ((tmp_key->back_key) ? tmp_key->back_key->globalRect[0] : init_rect[0]);
	tmp_key->diffRect[1] = tmp_key->globalRect[1] - ((tmp_key->back_key) ? tmp_key->back_key->globalRect[1] : init_rect[1]);
	tmp_key->diffRect[2] = tmp_key->globalRect[2] - ((tmp_key->back_key) ? tmp_key->back_key->globalRect[2] : init_rect[2]);
	tmp_key->diffRect[3] = tmp_key->globalRect[3] - ((tmp_key->back_key) ? tmp_key->back_key->globalRect[3] : init_rect[3]);

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

void ComponentUIAnimation::DrawInit()
{
	parent->cmp_rectTransform->SetRect(init_rect, true);
	if (parent->cmp_image) parent->cmp_image->SetAlpha(init_alpha, true);
}

void ComponentUIAnimation::DrawCurrent()
{
	parent->cmp_rectTransform->SetRect(current_key->globalRect, true);
	if (parent->cmp_image) parent->cmp_image->SetAlpha(current_key->alpha, true);
}

void ComponentUIAnimation::ImGuiDradDropCopyKeys()
{
#ifndef GAMEMODE
	ImGui::Button("Copy keys from dragged GameObject", ImVec2(250.0f, 0.0f));
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GAMEOBJECTS_HIERARCHY"))
		{
			GameObject* fromCopy = *(GameObject**)payload->Data;
			if (fromCopy && fromCopy->cmp_uiAnimation)
			{
				uint key_size = fromCopy->cmp_uiAnimation->keys.size();
				if (key_size > 0u) {
					animation_time = fromCopy->cmp_uiAnimation->animation_time;
					std::map<uint, Key*> tmp_list = fromCopy->cmp_uiAnimation->keys;
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
				if (!keys.empty()) current_key = keys.at(0);
				change_origin_rect = true;
				recalculate_times = true;
			}
		}
		ImGui::EndDragDropTarget();
	}
#endif // !GAMEMODE
}

//OnEditor Key
ComponentUIAnimation::key_editor ComponentUIAnimation::Key::OnEditor(float anim_time, bool isCurrent)
{
	key_editor ret = key_editor::NONE;
#ifndef GAMEMODE
	ImGui::Separator();
	std::string id_str = std::to_string(id + 1);
	std::string tmp;
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
	ImGui::Text("Key: %i", id + 1); 
	if (!isCurrent)
	{
		ImGui::SameLine();
		tmp = "Delete Key "; tmp += id_str;
		if (ImGui::Button(tmp.c_str()))
		{
			Delete();
			ret = key_editor::Deleted;
		}
	}
	ImGui::Text("DiffRect X: %i Y: %i W: %i H:%i",
		diffRect[0], diffRect[1], diffRect[2], diffRect[3]);
	ImGui::Text("GlobalRect X: %i Y: %i W: %i H:%i",
		globalRect[0], globalRect[1], globalRect[2], globalRect[3]);
	tmp = "Alpha key "; tmp += id_str;
	ImGui::SliderFloat(tmp.c_str(), &alpha, 0.0f, 1.0f);
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
