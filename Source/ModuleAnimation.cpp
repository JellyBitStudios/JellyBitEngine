#include "ModuleAnimation.h"

#include "ResourceAnimation.h"
#include "GameObject.h"

#include "Globals.h"
#include "Application.h"
#include "ModuleScene.h"
#include "ModuleResourceManager.h"
#include "ModuleTimeManager.h"
#include "ModuleGOs.h"
#include "ComponentTransform.h"
#include "ModuleEvents.h"
#include "EventSystem.h"
#include "ModuleInput.h"

#include "Brofiler/Brofiler.h"
//#include ".h" //TODO: delete this

#include "ComponentBone.h"
#include "ComponentMesh.h"

#include "ResourceBone.h"
#include "ResourceMesh.h"

#define SCALE 100 /// FBX/DAE exports set scale to 0.01
#define BLEND_TIME 1.0f

ModuleAnimation::ModuleAnimation()
{
	this->name = "ModuleAnimation";
}

ModuleAnimation::~ModuleAnimation()
{}

bool ModuleAnimation::Awake(JSON_Object* config)
{
	return true;
}

bool ModuleAnimation::Start()
{
	// Call here to attach bones and everytime that we reimport things
	//StartAttachingBones();

	if (current_anim) {
		current_anim->interpolate = true;
		current_anim->loop = true;
	}
	anim_state = AnimationState::PLAYING;

	return true;
}

// Called before quitting or switching levels
bool ModuleAnimation::CleanUp()
{
	DEPRECATED_LOG("Cleaning Animation");

	return true;
}

update_status ModuleAnimation::Update()
{
#ifndef GAMEMODE
	BROFILER_CATEGORY(__FUNCTION__, Profiler::Color::PapayaWhip);
#endif // GAMEMODE

	if (App->GetEngineState() != engine_states::ENGINE_PLAY)
		return update_status::UPDATE_CONTINUE;

	if (stop_all)
		return update_status::UPDATE_CONTINUE;
	if (current_anim == nullptr)
		return update_status::UPDATE_CONTINUE;

	float dt = 0.0f;
	dt = App->GetDt();
#ifdef GAMEMODE
	dt = App->timeManager->GetDt();
#endif // GAMEMODE


	if (current_anim->anim_timer >= current_anim->duration && current_anim->duration > 0.0f)
	{
		if (current_anim->loop)
			current_anim->anim_timer = 0.0f;
		else
			anim_state = AnimationState::STOPPED;
	}

	switch (anim_state)
	{
	case AnimationState::PLAYING:
		current_anim->anim_timer += dt * current_anim->anim_speed;
		MoveAnimationForward(current_anim->anim_timer, current_anim);
		break;

	case AnimationState::PAUSED:
		break;

	case AnimationState::STOPPED:
		current_anim->anim_timer = 0.0f;
		MoveAnimationForward(current_anim->anim_timer, current_anim);
		PauseAnimation();
		break;

	case AnimationState::BLENDING:
		last_anim->anim_timer += dt * last_anim->anim_speed;
		current_anim->anim_timer += dt * current_anim->anim_speed;
		blend_timer += dt;
		float blend_percentage = blend_timer / BLEND_TIME;
		MoveAnimationForward(last_anim->anim_timer, last_anim);
		MoveAnimationForward(current_anim->anim_timer, current_anim, blend_percentage);
		if (blend_percentage >= 1.0f) {
			anim_state = PLAYING;
		}
		break;
	}


	for (uint i = 0; i < current_anim->animable_gos.size(); ++i)
	{
		ComponentBone* tmp_bone = (ComponentBone*)current_anim->animable_gos.at(i)->GetComponent(ComponentTypes::BoneComponent);
		ResetMesh(tmp_bone);

	}

	for (uint i = 0; i < current_anim->animable_gos.size(); ++i)
	{
		ComponentBone* bone = (ComponentBone*)current_anim->animable_gos.at(i)->GetComponent(ComponentTypes::BoneComponent);
		
		if (bone && bone->attached_mesh)
		{
			DeformMesh(bone);
			ResourceMesh*res = (ResourceMesh*)App->res->GetResource(bone->attached_mesh->res);

			res->UnloadDeformableMeshFromMemory();
			res->GenerateAndBindDeformableMesh();
		}
	}

	return update_status::UPDATE_CONTINUE;
}

void ModuleAnimation::SetAnimationGos(ResourceAnimation * res)
{
	if (stop_all)
		return;
	Animation* animation = new Animation();
	animation->name = res->animationData.name;
	animation->anim_res_data = res->animationData;

#ifdef  GAMEMODE
	for (uint i = 0; i < res->animationData.numKeys; ++i)
		RecursiveGetAnimableGO(App->scene->root, &res->animationData.boneKeys[i], animation);
#endif //  GAMEMODE

	animation->duration = res->animationData.duration;

	animations.push_back(animation);
	current_anim = animations[0];
	current_anim->interpolate = true;
	current_anim->loop = true;
}

float ModuleAnimation::GetCurrentAnimationTime() const
{
	return current_anim->anim_timer;
}

const char* ModuleAnimation::GetAnimationName(int index) const
{
	return animations[index]->name.c_str();
}

uint ModuleAnimation::GetAnimationsNumber() const
{
	return (uint)animations.size();
}

ModuleAnimation::Animation* ModuleAnimation::GetCurrentAnimation() const
{
	return current_anim;
}

void ModuleAnimation::SetCurrentAnimationTime(float time)
{
	if (stop_all)
		return;
	current_anim->anim_timer = time;
	MoveAnimationForward(current_anim->anim_timer, current_anim);
}

bool ModuleAnimation::SetCurrentAnimation(const char* anim_name)
{
	if (stop_all)
		return true;
	for (uint i = 0u; i < animations.size(); i++)
	{
		Animation* it_anim = animations[i];
		if (strcmp(it_anim->name.c_str(), anim_name) == 0) {
			anim_state = BLENDING;
			blend_timer = 0.0f;
			last_anim = current_anim;
			current_anim = it_anim;
			SetCurrentAnimationTime(0.0f);
			return true;
		}
	}

	return false;
}

void ModuleAnimation::PlayAnimation()
{
	anim_state = AnimationState::PLAYING;
}

void ModuleAnimation::PauseAnimation()
{
	anim_state = AnimationState::PAUSED;
}

void ModuleAnimation::StopAnimation()
{
	anim_state = AnimationState::STOPPED;
}

void ModuleAnimation::StepBackwards()
{
	if (current_anim->anim_timer > 0.0f)
	{
		current_anim->anim_timer -= App->timeManager->GetRealDt() * current_anim->anim_speed;

		if (current_anim->anim_timer < 0.0f)
			current_anim->anim_timer = 0.0f;
		else
			MoveAnimationForward(current_anim->anim_timer, current_anim);

		PauseAnimation();
	}
}

void ModuleAnimation::StepForward()
{
	if (current_anim->anim_timer < current_anim->duration)
	{
		current_anim->anim_timer += App->timeManager->GetRealDt() * current_anim->anim_speed;

		if (current_anim->anim_timer > current_anim->duration)
			current_anim->anim_timer = 0.0f;
		else
			MoveAnimationForward(current_anim->anim_timer, current_anim);

		PauseAnimation();
	}
}

void ModuleAnimation::DeformMesh(ComponentBone* component_bone)
{
	ComponentMesh* mesh_co = component_bone->attached_mesh;

	if (mesh_co != nullptr)
	{
		ResourceBone* rbone = (ResourceBone*)App->res->GetResource(component_bone->res);
		ResourceMesh* mesh = (ResourceMesh*)App->res->GetResource(mesh_co->res);

		math::float4x4 trans = component_bone->GetParent()->transform->GetGlobalMatrix();
		trans = trans * component_bone->attached_mesh->GetParent()->transform->GetGlobalMatrix().Inverse();

		trans = trans * rbone->boneData.offsetMatrix;

		for (uint i = 0; i < rbone->boneData.bone_weights_size; ++i)
		{
			uint index = rbone->boneData.bone_weights_indices[i];
			math::float3 original(mesh->GetSpecificData().vertices[index].position);

			math::float3 vertex = trans.TransformPos(original);

			mesh->deformableMeshData.vertices[index].position[0] += vertex.x * rbone->boneData.bone_weights[i] * SCALE;
			mesh->deformableMeshData.vertices[index].position[1] += vertex.y * rbone->boneData.bone_weights[i] * SCALE;
			mesh->deformableMeshData.vertices[index].position[2] += vertex.z * rbone->boneData.bone_weights[i] * SCALE;
		}
	}
}