#ifndef __COMPONENT_UIANIMATOR_H__
#define __COMPONENT_UIANIMATOR_H__

#include "Component.h"

#include <list>

class ComponentUIAnimation : public Component
{
public:
	ComponentUIAnimation(GameObject* embedded_game_object, bool includeComponents = true);
	ComponentUIAnimation(const ComponentUIAnimation& component_ui_anim, GameObject* parent, bool includeComponents = true);
	~ComponentUIAnimation();

	void Update();

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

private:
	//version
	versionSerialization version = v1;
	//keys
	struct Key {
		Key() {}
		int diffRect[4] = { 0, 0, 0, 0};
		int globalRect[4] = { 0, 0, 0, 0 };
		float time_to_key = 0.0;

		static uint GetInternalSerializationBytes() {
			return sizeof(int) * 4 + sizeof(float);
		}
		void OnInternalSave(char*& cursor) {
			size_t bytes = sizeof(int) * 4;
			memcpy(cursor, diffRect, bytes);
			cursor += bytes;

			bytes = sizeof(float);
			memcpy(cursor, &time_to_key, bytes);
			cursor += bytes;
		}
		void OnInternalLoad(char*& cursor) {
			size_t bytes = sizeof(int) * 4;
			memcpy(diffRect, cursor, bytes);
			cursor += bytes;

			bytes = sizeof(float);
			memcpy(&time_to_key, cursor, bytes);
			cursor += bytes;
		}
	};

	std::list<Key*> keys;

	//curent
	Key* current_key = nullptr;
	float timer;

	//rect origin
	int init_rect[4];
	bool change_origin_rect = true;

	//recording mode (for rectTransform)
	bool recording = false;

	//calculate global pos of keys
	bool calculate_keys_global_pos = false;
};

#endif