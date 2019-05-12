#ifndef __COMPONENT_UIANIMATOR_H__
#define __COMPONENT_UIANIMATOR_H__

#include "Component.h"

#include <list>
#include <vector>
#include <map>

class ComponentUIAnimation : public Component
{
public:
	enum key_editor {
		NONE,
		SwapedUp,
		SwapedDown,
		Deleted
	};
	//keys
	struct Key {
		Key() {}
		Key(const Key& fromKey) {
			memcpy(diffRect, fromKey.diffRect, sizeof(int) * 4);
			time_key = fromKey.time_key;
		}
		uint id = 0;
		int diffRect[4] = { 0, 0, 0, 0 };
		int globalRect[4] = { 0, 0, 0, 0 };
		float time_key = 0.0f;
		float global_time = 0.0f;

		Key* back_key = nullptr;
		Key* next_key = nullptr;

		key_editor OnEditor(float anim_time, bool isCurrent = false);
		void Swap(Key* to)
		{
			if (to == back_key)
			{
				id -= 1;
				to->id += 1;
				if (to->back_key != nullptr)
					to->back_key->next_key = this;
				if (next_key != nullptr)
					next_key->back_key = to;
				to->next_key = next_key;
				back_key = to->back_key;
				to->back_key = this;
				next_key = to;
			}
			else if (to == next_key)
			{
				id += 1;
				to->id -= 1;
				if (to->next_key != nullptr)
					to->next_key->back_key = this;
				if (back_key != nullptr)
					back_key->next_key = to;
				to->back_key = back_key;
				next_key = to->next_key;
				to->next_key = this;
				back_key = to;
			}
			float tmp_time = time_key;
			time_key = to->time_key;
			to->time_key = tmp_time;
		}

		void Delete() {
			if(back_key) back_key->next_key = next_key;
			if (next_key) next_key->back_key = back_key;
		}

		void CheckNextKeyTime() {
			if (next_key)
			{
				if (next_key->time_key < time_key)
					next_key->time_key = time_key;
				next_key->CheckNextKeyTime();
			}
		}

		static uint GetInternalSerializationBytes() {
			return sizeof(int) * 4 + sizeof(float);
		}
		void OnInternalSave(char*& cursor) {
			size_t bytes = sizeof(int) * 4;
			memcpy(cursor, diffRect, bytes);
			cursor += bytes;

			bytes = sizeof(float);
			memcpy(cursor, &time_key, bytes);
			cursor += bytes;
		}
		void OnInternalLoad(char*& cursor) {
			size_t bytes = sizeof(int) * 4;
			memcpy(diffRect, cursor, bytes);
			cursor += bytes;

			bytes = sizeof(float);
			memcpy(&time_key, cursor, bytes);
			cursor += bytes;
		}
	};
	enum UIAnimationState
	{
		NOT_DEF_STATE = -1,
		PLAYING,
		PAUSED,
		STOPPED
	};

public:
	ComponentUIAnimation(GameObject* embedded_game_object, bool includeComponents = true);
	ComponentUIAnimation(const ComponentUIAnimation& component_ui_anim, GameObject* parent, bool includeComponents = true);
	~ComponentUIAnimation();

	void Update();

	void Interpolate(float time);

	bool IsRecording()const;

	float GetAnimationTime()const;
	bool HasKeys()const;
	
	void ImGuiKeys();
	
private:
	//versions
	enum versionSerialization {
		v1 = 0,
	};
	virtual uint GetInternalSerializationBytes();
	virtual void OnInternalSave(char*& cursor);
	virtual void OnInternalLoad(char*& cursor);
	void OnUniqueEditor();

	void OnSystemEvent(System_Event event);


private:
	void AddKey();
	void AddKeyOnCombo();

private:
	//version
	versionSerialization version = v1; //always defined as last.

	// ----- Animation component -----
	UIAnimationState animation_state = UIAnimationState::STOPPED;

	std::map<uint, Key*> keys;

	bool usePanel = false;

	//curent
	Key* current_key = nullptr;

	//Animation values
	float animation_time = 0.0f;
	float animation_timer = 0.0f;
	bool repeat = false;

	//rect origin
	int init_rect[4];
	bool change_origin_rect = true;

	//recording mode (for rectTransform)
	bool recording = false;
	bool modifying_key = false;

	// ----- Key stuff -----
	bool recalculate_times = false;

	//calculate global pos of keys
	bool calculate_keys_global = false;
	
	//Combo values
	std::vector<char*> keys_strCombo;	
};

#endif