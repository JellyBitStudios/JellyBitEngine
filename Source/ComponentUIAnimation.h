#ifndef __COMPONENT_UIANIMATOR_H__
#define __COMPONENT_UIANIMATOR_H__

#include "Component.h"

#include <list>
#include <vector>
#include <map>

class ComponentUIAnimation : public Component
{
public:
	//enum for return of key OnEditor (know what happened and apply changes)
	enum key_editor {
		NONE,
		SwapedUp,
		SwapedDown,
		Deleted
	};
	//keys
	struct Key {
		Key() {}
		Key(const Key& fromKey) { //copy contructor
			memcpy(diffRect, fromKey.diffRect, sizeof(int) * 4);
			time_key = fromKey.time_key;
			alpha = fromKey.alpha;
		}
		uint id = 0; //key id, position, order of play
		int diffRect[4] = { 0, 0, 0, 0 }; // difference from last key
		int globalRect[4] = { 0, 0, 0, 0 }; // global rect
		float time_key = 0.0f; // percentage time, 0 to 1
		float global_time = 0.0f; // global time, animation time * oercentage

		float alpha = 1.0f;

		Key* back_key = nullptr;
		Key* next_key = nullptr;

		key_editor OnEditor(float anim_time, bool isCurrent = false);
		void Swap(Key* to) //swap keys positions, mantain time and change pointers
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

		void Delete() { //delete key and modify if from nexts keys 
			CheckIDFromDeleted(id);
			if(back_key) back_key->next_key = next_key;
			if (next_key) next_key->back_key = back_key;
		}

		void CheckNextKeyTime() { // checks of nexts keys for not be lowered time
			if (next_key)
			{
				if (next_key->time_key < time_key)
					next_key->time_key = time_key;
				next_key->CheckNextKeyTime();
			}
		}

		void CheckIDFromDeleted(uint tmp_id) { //when delecting
			if (next_key)
			{
				uint tmp = next_key->id;
				next_key->id = tmp_id;
				next_key->CheckIDFromDeleted(tmp);
			}
		}

		static uint GetInternalSerializationBytes() {
			return sizeof(int) * 4 + sizeof(float) * 2;
			//diff rect			| percent time | alpha
		}
		void OnInternalSave(char*& cursor) {
			size_t bytes = sizeof(int) * 4;
			memcpy(cursor, diffRect, bytes);
			cursor += bytes;

			bytes = sizeof(float);
			memcpy(cursor, &time_key, bytes);
			cursor += bytes;

			bytes = sizeof(float);
			memcpy(cursor, &alpha, bytes);
			cursor += bytes;
		}
		void OnInternalLoad(char*& cursor) {
			size_t bytes = sizeof(int) * 4;
			memcpy(diffRect, cursor, bytes);
			cursor += bytes;

			bytes = sizeof(float);
			memcpy(&time_key, cursor, bytes);
			cursor += bytes;

			bytes = sizeof(float);
			memcpy(&alpha, cursor, bytes);
			cursor += bytes;
		}
	};
	enum UIAnimationState
	{
		NOT_DEF_STATE = -1,
		PLAYING,
		REWIND,
		PAUSED,
		STOPPED
	};

public:
	ComponentUIAnimation(GameObject* embedded_game_object, bool includeComponents = true);
	ComponentUIAnimation(const ComponentUIAnimation& component_ui_anim, GameObject* parent, bool includeComponents = true);
	~ComponentUIAnimation();

	void OnSystemEvent(System_Event event);

	void Update();

	void Interpolate(float time);

	//Getters info
	bool IsRecording()const; //cheker from rect transform, will be deleted. TODO. It's possible. same as modifying_key
	float GetAnimationTime()const;
	bool HasKeys()const;
	
	//Cal from comopnent image
	void SetInitAlpha(float alpha);

	//Call from panel
	void ImGuiKeys();
	
	//For scripting, needed to discurss to add more options. Needed.
	void Play(bool fromLoop = false);
	void Stop();
	bool IsFinished()const;
	inline bool GetLoop() const { return loop; }
	void SetLoop(bool loopable);
	void Rewind(bool fromLoop = false);

private:
	//versions
	enum versionSerialization {
		v1 = 0,
	};
	virtual uint GetInternalSerializationBytes();
	virtual void OnInternalSave(char*& cursor);
	virtual void OnInternalLoad(char*& cursor);
	void OnUniqueEditor();

private:
	//Imgui OnEitor
	void AddKey(); //When recording
	void AddKeyOnCombo(); //for imguiCombo
	void ImGuiDradDropCopyKeys(); //called OnEditor if keys is empty

	//calls to rect trnasform and image for set init or current values
	void DrawInit();
	void DrawCurrent();

private:
	//version
	versionSerialization version = v1; //always defined as last.

	// ----- Animation component -----
	UIAnimationState animation_state = UIAnimationState::STOPPED; //state

	std::map<uint, Key*> keys; //keys as map, id map == id keys and order reproduction

	bool usePanel = false; //value that cheks is observed by panel

	//curent
	Key* current_key = nullptr; // Playing to this key, then change to the next.

	//Animation values
	float animation_time = 0.0f; // Total time of animation
	float animation_timer = 0.0f; //Internal timer
	bool loop = false;
	bool is_finished = false;

	//Origin values
	int init_rect[4] = {0,0,100,100};
	float init_alpha = 1.0f;
	bool change_origin_rect = true; //for reset values. at update

	//recording mode (for rectTransform)
	bool recording = false; //recording, save keys
	bool modifying_key = false; // modify a selected key

	// ----- Key stuff -----
	//Calculate global time of keys
	bool recalculate_times = false;
	//calculate global pos of keys
	bool calculate_keys_global = false;
	
	//Combo values array position.
	std::vector<char*> keys_strCombo;	
};

#endif