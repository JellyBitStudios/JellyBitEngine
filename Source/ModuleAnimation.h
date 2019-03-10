#ifndef __MODULE_ANIMATION_H__
#define __MODULE_ANIMATION_H__

#include "Module.h"
#include "Globals.h"
#include "MathGeoLib/include/MathGeoLib.h"
#include "ResourceAnimation.h"

#include <vector>
#include <map>
#include <list>

class GameObject;
class ComponentBone;

enum AnimationState
{
	NOT_DEF_STATE = -1,
	PLAYING,
	PAUSED,
	STOPPED,
	BLENDING
};

class ModuleAnimation : public Module
{
public:

	struct Animation {
		std::string name;
		std::vector<GameObject*> animable_gos;
		std::map<GameObject*, BoneTransformation*> animable_data_map;

		bool loop = true;
		bool interpolate = false;
		float anim_speed = 1.0f;

		float anim_timer = 0.0f;
		float duration = 0.0f;

		ResourceAnimationData anim_res_data;
	};

	std::vector<Animation*> animations;

public:

	ModuleAnimation();
	~ModuleAnimation();


	// Called before render is available
	bool Awake(JSON_Object* config = nullptr);

	// Called before the first frame
	bool Start();
	bool CleanUp();
	update_status Update();

	void SetAnimationGos(ResourceAnimation* res);

	float GetCurrentAnimationTime() const;
	const char* GetAnimationName(int index) const;
	uint GetAnimationsNumber() const;
	Animation* GetCurrentAnimation() const;

	void SetCurrentAnimationTime(float time);
	bool SetCurrentAnimation(const char* anim_name);


	void PlayAnimation();
	void PauseAnimation();
	void StopAnimation();
	void StepBackwards();
	void StepForward();


private:

	Animation* current_anim = nullptr;
	Animation* last_anim = nullptr;
	bool stop_all = false;

	float blend_timer = 0.0f;
	std::vector<ResourceAnimation*> available_animations;

public:
	AnimationState anim_state = AnimationState::PLAYING;

};

#endif // __ANIMATION_H__
