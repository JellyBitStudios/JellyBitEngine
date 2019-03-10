#ifndef __RESOURCE_ANIMATOR_H__
#define __RESOURCE_ANIMATOR_H__

#include "Resource.h"
#include <vector>

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

struct ResourceAnimatorData
{
	std::string name;
	uint avatar_uuid = 0u;
	std::vector<uint> meshes_uuids;
	std::vector<uint> animations_uuids;
};

class ResourceAnimator : public Resource
{


public:

	ResourceAnimator(ResourceTypes type, uint uuid, ResourceData data, ResourceAnimatorData animationData);
	~ResourceAnimator();

	bool LoadInMemory();
	bool UnloadFromMemory();

	void OnPanelAssets();

	static bool ImportFile(const char* file, std::string& name, std::string& outputFile);
	static bool ExportFile(ResourceData& data, ResourceAnimatorData& prefabData, std::string& outputFile, bool overwrite = false);
	static uint CreateMeta(const char* file, uint prefab_uuid, std::string& name, std::string& outputMetaFile);
	static bool ReadMeta(const char* metaFile, int64_t& lastModTime, uint& prefab_uuid, std::string& name);
	static bool LoadFile(const char* file, ResourceAnimatorData& prefab_data_output);



	bool GenerateLibraryFiles() const;
	static uint SetNameToMeta(const char* metaFile, const std::string& name);

	/*---------------------------------------------------------------------------------------------------------------*/

	

public:
	ResourceAnimatorData animator_data;




	/*---------------------------------------- ANIMATOR MODULE STUFF ----------------------------------------*/
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
	void InitAnimator();
	bool Update();

	void AddAnimationFromAnimationResource(ResourceAnimation* res);

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

#endif // __RESOURCE_ANIMATION_H__