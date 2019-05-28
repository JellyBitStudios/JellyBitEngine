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

	/* ----- SCRIPTING CALLS ----- */
	/*Returns -1 if anything goes wrong*/
	int GetCurrentAnimationFrame()const;
	const char* GetCurrentAnimationName();
	bool PlayAnimation(const char* anim_name);
	bool CleanAnimations();
	bool AnimationFinished()const;
	bool UpdateAnimationSpeed(float new_speed);
	bool UpdateBlendTime(float new_blend);
	void SetAnimationLoop(bool loop);
	/* ----- SCRIPTING CALLS ----- */

	void OnInternalSave(char*& cursor);
	void OnInternalLoad(char*& cursor);
	void OnEditor();
	void OnUniqueEditor();

public:
	uint res = 0;
	uint res_avatar = 0;
	std::vector<uint> res_animations;

	GameObject* go_with_mesh = nullptr;

};

#endif // __COMPONENT_ANIMATOR_H__