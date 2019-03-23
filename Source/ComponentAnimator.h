#ifndef __COMPONENT_ANIMATOR_H__
#define __COMPONENT_ANIMATOR_H__

#include "Component.h"
#include <vector>

class ComponentAnimator : public Component
{
public:

	ComponentAnimator(GameObject* embedded_game_object);
	ComponentAnimator(GameObject* embedded_game_object, uint resource);
	ComponentAnimator(const ComponentAnimator& component_anim, GameObject* parent, bool include = true);
	~ComponentAnimator();

	uint GetInternalSerializationBytes();
	bool SetResourceAnimator(uint resource);
	bool SetResourceAvatar(uint resource);
	bool SetResourceAnimation(uint resource);

	virtual void Update();

	bool PlayAnimation(const char* anim_name);
	int GetCurrentAnimationFrame();
	const char* GetCurrentAnimationName();

	void OnInternalSave(char*& cursor);
	void OnInternalLoad(char*& cursor);
	void OnEditor();
	void OnUniqueEditor();

public:
	uint res = 0;
	uint res_avatar = 0;
	std::vector<uint> res_animations;

};

#endif // __COMPONENT_ANIMATOR_H__