#ifndef __COMPONENT_UIANIMATOR_H__
#define __COMPONENT_UIANIMATOR_H__

#include "Component.h"

#include <list>
#include <vector>

class ComponentUIAnimation : public Component
{
public:
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
	versionSerialization version = v1;
	//keys
	struct Key {
		Key() {}
		uint id = 0;
		int diffRect[4] = { 0, 0, 0, 0};
		int globalRect[4] = { 0, 0, 0, 0 };
		float time_key = 0.0f;
		float global_time = 0.0f;

		Key* back_key = nullptr;
		Key* next_key = nullptr;

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

	

	// ----- Animation component -----

	UIAnimationState animation_state = UIAnimationState::STOPPED;

	std::list<Key*> keys;

	//curent
	Key* current_key = nullptr;

	float animation_time = 0.0f;

	float animation_timer = 0.0f;

	//rect origin
	int init_rect[4];
	bool change_origin_rect = true;

	//recording mode (for rectTransform)
	bool recording = false;


	// ----- Key stuff -----

	bool recalculate_times = false;

	//calculate global pos of keys
	bool calculate_keys_global = false;
	
	//Combo values
	std::vector<char*> keys_strCombo;	
};

#endif