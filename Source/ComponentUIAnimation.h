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

private:
	virtual uint GetInternalSerializationBytes();
	virtual void OnInternalSave(char*& cursor);
	virtual void OnInternalLoad(char*& cursor);
	void OnUniqueEditor();

	void OnSystemEvent(System_Event event);


private:
	struct Key {
		int rect[4];
		float time_to_key;

		uint GetInternalSerializationBytes() {

		}
		void OnInternalSave(char*& cursor) {

		}
		void OnInternalLoad(char*& cursor) {

		}
	};

	std::list<Key> keys;

	Key* current_key = nullptr;
};

#endif