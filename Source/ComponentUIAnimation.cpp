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

#ifndef GAMEMODE //checks if panel is active
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
	loop = component_ui_anim.loop;

	uint key_size = component_ui_anim.keys.size();
	if (key_size > 0u) {
		std::map<uint, Key*> tmp_list = component_ui_anim.keys;
		Key* last_key = nullptr;
		for (std::map<uint, Key*>::iterator it = tmp_list.begin(); it != tmp_list.end(); ++it) {

			Key* tmp_key = new Key(*it->second); //copy constructor of key

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
	
	if (includeComponents)
	{
		if (!parent->cmp_image) parent->AddComponent(ComponentTypes::ImageComponent);

		if (!keys.empty())
			current_key = keys.at(0);

#ifndef GAMEMODE //checks if panel is active
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
	if (change_origin_rect) // catch init info of transform when it's no editing with animation
	{
		memcpy(init_rect, parent->cmp_rectTransform->GetRect(), sizeof(int) * 4);
		calculate_keys_global = true;
		change_origin_rect = false;

		if (parent->cmp_image) init_alpha = parent->cmp_image->GetAlpha();
	}

	if (calculate_keys_global) //calculate global keys
	{
		for (std::map<uint, Key*>::iterator it = keys.begin(); it != keys.end(); ++it)
		{
			Key* k = it->second;
			//know the global position by the key diff and the globalrect of back key
			k->globalRect[0] = k->diffRect[0] + ((k->back_key) ? k->back_key->globalRect[0] : init_rect[0]);
			k->globalRect[1] = k->diffRect[1] + ((k->back_key) ? k->back_key->globalRect[1] : init_rect[1]);
			k->globalRect[2] = k->diffRect[2] + ((k->back_key) ? k->back_key->globalRect[2] : init_rect[2]);
			k->globalRect[3] = k->diffRect[3] + ((k->back_key) ? k->back_key->globalRect[3] : init_rect[3]);
		}

		calculate_keys_global = false;
	}

	if (recalculate_times) //calculate global times
	{
		for (std::map<uint, Key*>::iterator it = keys.begin(); it != keys.end(); ++it)
			it->second->global_time = animation_time * it->second->time_key; //global time =  animation time * key percentage time

		recalculate_times = false;
	}

	// Interpolate
	if (keys.size() <= 1)
		return; //exits when keys is 1 or empty

	//when is in engine mode use app dt on gamemode of time mamager
	float dt = (App->GetEngineState() == engine_states::ENGINE_PLAY) ? App->timeManager->GetDt() : App->GetDt();

	switch (animation_state)
	{
		case UIAnimationState::PLAYING: {

			if (animation_timer >= current_key->global_time) { //if time surpass the time of key, chenge to the next
				if (current_key->next_key != nullptr) {
					current_key = current_key->next_key;
				}
				else
				{
					//if not next, pause the animation. And if engine editor return to the init position (Stop)
					is_finished = true;

					if (!loop)
					{
						animation_state = UIAnimationState::STOPPED;
						if (wasStatic)
						{
							parent->ToggleChildrenAndThisStatic(true);
							wasStatic = false;
						}
					}
					else
						Play(true);

					if (App->GetEngineState() == engine_states::ENGINE_EDITOR && !loop)
						Stop();

					break;
				}
			}
			else
				is_finished = false;

			Interpolate(animation_timer);
			animation_timer += dt;
		}
		break;

		case UIAnimationState::REWIND:
		{
			if (animation_timer <= 0.0f) { //if time surpass the time of key, chenge to the next
				if (current_key->back_key != nullptr) {
					current_key = current_key->back_key;
				}
				else
				{
					//if not next, pause the animation. And if engine editor return to the init position (Stop)
					is_finished = true;

					if (!loop)
					{
						animation_state = UIAnimationState::STOPPED;
						if (wasStatic)
						{
							parent->ToggleChildrenAndThisStatic(true);
							wasStatic = false;
						}
					}
					else
						Rewind(true);

					if (App->GetEngineState() == engine_states::ENGINE_EDITOR && !loop)
						Stop();

					break;
				}
			}
			else
				is_finished = false;

			Interpolate(animation_timer);
			animation_timer -= dt;
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
	if (parent->cmp_image) parent->cmp_image->SetAlpha(next_alpha, true);
}

//cheks if recording for rectTransform
bool ComponentUIAnimation::IsRecording() const
{
	return recording;
}

uint ComponentUIAnimation::GetInternalSerializationBytes()
{
	uint keys_size = (!keys.empty()) ? keys.at(0)->GetInternalSerializationBytes() * keys.size() : 0;
	return keys_size + sizeof(float) + sizeof(bool) + sizeof(uint) + sizeof(versionSerialization);
	//	total keys size				||animation repeat||size keys || version of serialization
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
	memcpy(cursor, &loop, bytes);
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
		memcpy(&loop, cursor, bytes);
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
				case UIAnimationState::REWIND:
				case UIAnimationState::PLAYING:
					ImGui::Text("Playing");
				break;
				case UIAnimationState::STOPPED:
					ImGui::Text("Stopped");
				break;
			}
			ImGui::Text("Current timer: %f", animation_timer);

			bool change_rect_pos = false;
			int selectable_key = (current_key) ? current_key->id : 0; //ImGui combo knows position by key id.
			ImGui::PushItemWidth(75.0f);
			if (ImGui::Combo("Current", &selectable_key, keys_strCombo.data(), keys.size())) //Drop list of keys
			{
				modifying_key = false;
				current_key = keys.at(selectable_key);
				change_rect_pos = true;
			}

			//Move left or right of currents
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
			}
			// -----

			if (change_rect_pos) //sets draw of current or init if recording
				(recording) ? DrawCurrent() : DrawInit();

			// if not recording and exists current key, appears a button that enables individual modification of rect from key
			if (current_key) {
				if (!recording)
				{
					ImGui::SameLine();
					if (popStyle = modifying_key) //red button when is enabled
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
			if (ImGui::Button("Play")) {
				Play();
			}
			ImGui::SameLine();

			if (ImGui::Button("Rewind")) {
				Rewind();
			}
			ImGui::SameLine();

			if (ImGui::Button("Pause"))
				animation_state = UIAnimationState::PAUSED;

			ImGui::SameLine();

			if (ImGui::Button("Stop")) {
				Stop();
			}

			ImGui::SameLine();
			ImGui::Checkbox((loop) ? "Quit Loop" : "Enable Loop", &loop);

			//change keys when on play or current
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
			// -----
		}
		else {
			ImGui::Text("There is no key for this UI GO ...");
			ImGuiDradDropCopyKeys();
		}

		if (popStyle = recording) //red button when is enabled
		{
			ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Button, ImVec4(1.0, 0.0, 0.0, 0.5));
			ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_ButtonHovered, ImVec4(1.0, 0.0, 0.0, 0.7));
			ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_ButtonActive, ImVec4(1.0, 0.0, 0.0, 1.0));
		}
		//Enables recording, appears a panel that shows all info os keys, and you can move the rect and save positions
		if(ImGui::Button((recording) ? "Stop recording" : "Start recording")) {
			recording = !recording;

			if(recording)
				App->gui->panelUIAnimation->SetOnOff(usePanel = true);

			if (!keys.empty())
			{
				if (recording){ //if jeys and recording, ser current key at last and draw
					modifying_key = false;
					current_key = keys.at(keys.size() - 1);
					DrawCurrent();
				}
			}
			if (!recording) { //if stop recording, return at init position
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

		//bool know if is showing this component at panel
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
		if (modifying_key) // cheks event of rect for modify current key if not change init rect and recalculate all elements
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

void ComponentUIAnimation::ImGuiKeys() //Call from panel for draw all  keys
{
#ifndef GAMEMODE
	if (!keys.empty())
	{
		for (uint i = 0; i < keys.size(); i++)
		{
			// needs differents sttrings for imgui, detects as id
			std::string t = "Set key "; t += std::to_string(i + 1); t += " to current"; 
			if (ImGui::Button(t.c_str()))
			{
				modifying_key = false;
				current_key = keys.at(i);
				if (recording) DrawCurrent();
			}
			ImGui::SameLine();
			ComponentUIAnimation::key_editor returned = keys.at(i)->OnEditor(animation_time);
			switch (returned) //Editor of key returns a value for know if something happened and apply to main map of animation
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
			case key_editor::Deleted: //when delecting all map needed to be realocated of after elements
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

void ComponentUIAnimation::Play(bool fromLoop)
{
	if (keys.size() > 1) //needed to be more than one key to play
	{
		animation_state = UIAnimationState::PLAYING;
		current_key = keys.at(0);
		animation_timer = keys.at(0)->global_time;
		DrawCurrent();

		if (parent->IsStatic() && !fromLoop)
		{
			parent->ToggleChildrenAndThisStatic(false);
			wasStatic = true;
		}

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

	if (!parent->IsStatic())
		parent->ToggleChildrenAndThisStatic(true);
}

bool ComponentUIAnimation::IsFinished() const
{
	return is_finished;
}

void ComponentUIAnimation::SetLoop(bool loopable)
{
	loop = loopable;
}

void ComponentUIAnimation::Rewind(bool fromLoop)
{
	if (keys.size() > 1) //needed to be more than one key to play
	{
		animation_state = UIAnimationState::REWIND;
		current_key = keys.at(keys.size() - 1);
		animation_timer = animation_time;
		DrawCurrent();

		if (parent->IsStatic() && !fromLoop)
		{
			parent->ToggleChildrenAndThisStatic(false);
			wasStatic = true;
		}
	}
	else
	{
		CONSOLE_LOG(LogTypes::Warning, "You can't rewind with one key.");
	}
}

void ComponentUIAnimation::SetInitAlpha(float alpha) //called from cmp image
{
	init_alpha = alpha;
}

//from OnEditor adds a default key on current position of rect when recording
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

	//Save the difference from last
	tmp_key->diffRect[0] = tmp_key->globalRect[0] - ((tmp_key->back_key) ? tmp_key->back_key->globalRect[0] : init_rect[0]);
	tmp_key->diffRect[1] = tmp_key->globalRect[1] - ((tmp_key->back_key) ? tmp_key->back_key->globalRect[1] : init_rect[1]);
	tmp_key->diffRect[2] = tmp_key->globalRect[2] - ((tmp_key->back_key) ? tmp_key->back_key->globalRect[2] : init_rect[2]);
	tmp_key->diffRect[3] = tmp_key->globalRect[3] - ((tmp_key->back_key) ? tmp_key->back_key->globalRect[3] : init_rect[3]);

	keys.insert(std::pair<uint, Key*>(tmp_key->id, tmp_key));
	current_key = tmp_key;

	AddKeyOnCombo();
}

//When add key, a string of name as id, needs to be added to another vector. For ImGui combo
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
 //Draws to init (original go values) or current key.
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
// -----
// -- Copy all keys from another game object that contains keys
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

// ------ OnEditor Key
// value isCurrent - if call is from OnEditor of Component, is the current key and it isn't interactuable to other keys or map. 
ComponentUIAnimation::key_editor ComponentUIAnimation::Key::OnEditor(float anim_time, bool isCurrent)
{
	key_editor ret = key_editor::NONE;
#ifndef GAMEMODE
	ImGui::Separator();
	std::string id_str = std::to_string(id + 1);
	std::string tmp;
	if (!isCurrent) 
	{
		// Move key of map (of keys pointers, after at map by event from this method), change ids and mantain time.
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
		// ------
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
	//Show rects values
	ImGui::Text("DiffRect X: %i Y: %i W: %i H:%i",
		diffRect[0], diffRect[1], diffRect[2], diffRect[3]);
	ImGui::Text("GlobalRect X: %i Y: %i W: %i H:%i",
		globalRect[0], globalRect[1], globalRect[2], globalRect[3]);

	//Modifiable values, alpha and time(for time cheks the befor key time and the next key times)
	// One key time can't be lower than the back key.
	tmp = "Alpha key "; tmp += id_str;
	ImGui::SliderFloat(tmp.c_str(), &alpha, 0.0f, 1.0f);
	tmp = "Time key "; tmp += id_str; // Modifiable time by slider
	if (ImGui::SliderFloat(tmp.c_str(), &time_key, 0.0f, 1.0f)) {
		(back_key != nullptr && time_key < back_key->time_key) ? time_key = back_key->time_key : time_key;
		global_time = anim_time * time_key;
		CheckNextKeyTime();
	}
	//Show global time
	ImGui::Text("Global time: %f", global_time);
#endif
	return ret;
}
