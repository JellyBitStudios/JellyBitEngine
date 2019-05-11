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
		char buffer[sizeof(int) * 4 + sizeof(float)];
		char* cursor = buffer;
		std::list<Key*> tmp_list = component_ui_anim.keys;
		Key* last_key = nullptr;
		for (std::list<Key*>::iterator it = tmp_list.begin(); it != tmp_list.end(); ++it) {
			Key* tmp_key = new Key();
			(*it)->OnInternalSave(cursor);
			cursor = buffer;
			tmp_key->OnInternalLoad(cursor);
			cursor = buffer;

			tmp_key->id = keys.size();

			if (last_key)
			{
				last_key->next_key = tmp_key;
				tmp_key->back_key = last_key;
			}

			last_key = tmp_key;
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
		recalculate_times = true;
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
	uint keys_size = (!keys.empty()) ? (*keys.begin())->GetInternalSerializationBytes() * keys.size() : 0;
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
		for (std::list<Key*>::iterator it = keys.begin(); it != keys.end(); ++it)
			(*it)->OnInternalSave(cursor);
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
				
				keys.push_back(nkey);
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
		ImGui::Text("UI Animation");

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

			int selectable_key = (current_key) ? current_key->id : 0;
			ImGui::PushItemWidth(75.0f);
			if (ImGui::Combo("Select", &selectable_key, keys_strCombo.data(), keys.size()));
			{
				uint count = 0u;
				for (std::list<Key*>::iterator it = keys.begin(); it != keys.end(); ++it, count++) {
					if (count == selectable_key) {
						current_key = (*it);
						break;
					}
				}
			}

			if (current_key) {
				ImGui::Text("Current"); ImGui::SameLine();
				current_key->OnEditor(animation_time);
			}

			ImGui::Separator();
			if (ImGui::Button("Play")) {//TODO PREPARE FOR SCRIPTING
				if (keys.size() > 1)
				{
					animation_state = UIAnimationState::PLAYING;
					current_key = keys.front();
					animation_timer = 0;
					parent->cmp_rectTransform->SetRect(init_rect, true);
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
				current_key = keys.front();
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

		if(ImGui::Button((recording) ? "Stop recording" : "Start recording")) {
			recording = !recording;

			if (recording)
			{
				usePanel = true;
				App->gui->panelUIAnimation->SetCmp(this);
			}

			if (!recording) {
				usePanel = false;
				parent->cmp_rectTransform->SetRect(init_rect, true);
				App->gui->panelUIAnimation->ClearCmp();
			}
		}
		
		if (recording) {
			ImGui::SameLine();
			if (ImGui::Button("Save Key"))
				this->AddKey();
		}

		if (ImGui::Button((usePanel) ? "Hide panel" : "Show Panel"))
		{
			usePanel = !usePanel;

			if (usePanel)
				App->gui->panelUIAnimation->SetCmp(this);
			else
				App->gui->panelUIAnimation->ClearCmp();
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

std::list<ComponentUIAnimation::Key*>* ComponentUIAnimation::GetKeys()
{
	return &keys;
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
		for (std::list<ComponentUIAnimation::Key*>::iterator it = keys.begin(); it != keys.end(); ++it)
		{
			std::string t = "Set key "; t += std::to_string((*it)->id); t += " to current";
			if (ImGui::Button(t.c_str()))
				current_key = *it;
			ImGui::SameLine();
			(*it)->OnEditor(animation_time);
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
		tmp_key->back_key = keys.back();
		keys.back()->next_key = tmp_key;
	}

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

//OnEditor Key
void ComponentUIAnimation::Key::OnEditor(float anim_time)
{
#ifndef GAMEMODE
	ImGui::Separator();
	ImGui::Text("Key: "); ImGui::SameLine(); ImGui::Text("%i", id);
	ImGui::Text("DiffRect X: %i Y: %i W: %i H:%i",
		diffRect[0], diffRect[1], diffRect[2], diffRect[3]);
	ImGui::Text("GlobalRect X: %i Y: %i W: %i H:%i",
		globalRect[0], globalRect[1], globalRect[2], globalRect[3]);
	std::string t = "Time key"; t += std::to_string(id);
	if (ImGui::SliderFloat(t.c_str(), &time_key, 0.0f, 1.0f)) {
		global_time = anim_time * time_key;
	}

	ImGui::SameLine();

	ImGui::Text("| Global time: %f", global_time);
#endif
}
