#ifndef __COMPONENT_ANIMATOR_H__
#define __COMPONENT_ANIMATOR_H__

#include "Component.h"

class ComponentAnimator : public Component
{
public:

	ComponentAnimator(GameObject* embedded_game_object);
	ComponentAnimator(GameObject* embedded_game_object, uint resource);
	ComponentAnimator(const ComponentAnimator& component_anim, GameObject* parent, bool include = true);
	~ComponentAnimator();

	uint GetInternalSerializationBytes();
	bool SetResource(uint resource);

	bool PlayAnimation(const char* anim_name);

	void OnInternalSave(char*& cursor);
	void OnInternalLoad(char*& cursor);
	void OnEditor();
	void OnUniqueEditor();

public:
	uint res = 0;

};

#endif // __COMPONENT_ANIMATOR_H__