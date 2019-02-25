#ifndef __COMPONENT_ANIMATION_H__
#define __COMPONENT_ANIMATION_H__

#include "Component.h"

class ComponentAnimation : public Component
{
public:

	ComponentAnimation(GameObject* embedded_game_object);
	ComponentAnimation(GameObject* embedded_game_object, uint resource);
	~ComponentAnimation();

	uint GetInternalSerializationBytes();
	bool Save(JSON_Object* component_obj) const;
	bool Load(const JSON_Object* component_obj);

	bool SetResource(uint resource);

	void OnInternalSave(char*& cursor) {}
	void OnInternalLoad(char*& cursor) {}

public:
	uint res = 0;

};

#endif // __COMPONENT_ANIMATION_H__