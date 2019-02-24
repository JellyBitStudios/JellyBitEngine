#include "ModuleAnimation.h"

#include "ResourceAnimation.h"
#include "GameObject.h"

#include "Globals.h"
#include "Application.h"
#include "ModuleScene.h"
#include "ModuleResourceManager.h"
#include "ModuleTimeManager.h"
#include "ModuleGOs.h"

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

bool ModuleAnimation::Update(float dt)
{
	if (current_anim == nullptr)
		return true;

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
		}
	}

	return true;
}

void ModuleAnimation::SetAnimationGos(ResourceAnimation * res)
{
	Animation* animation = new Animation();
	animation->name = res->name;
	for (uint i = 0; i < res->num_keys; ++i)
		RecursiveGetAnimableGO(App->scene->root, &res->bone_keys[i], animation);

	animation->duration = res->duration;

	animations.push_back(animation);
	current_anim = animations[0];
	current_anim->interpolate = true;
	current_anim->loop = true;
}

void ModuleAnimation::RecursiveGetAnimableGO(GameObject * go, ResourceAnimation::BoneTransformation* bone_transformation, Animation* anim)
{
	std::vector<GameObject*> all_gos;
	App->GOs->GetGameobjects(all_gos);

	for (uint i = 0u; i < all_gos.size(); i++)
	{
		GameObject* current_go = all_gos.at(i);

		if (bone_transformation->bone_name.compare(current_go->GetName()) == 0)
		{
			if (/*!go->to_destroy*/1 /* TODO_G */) {
				anim->animable_data_map.insert(std::pair<GameObject*, ResourceAnimation::BoneTransformation*>(current_go, bone_transformation));
				anim->animable_gos.push_back(current_go);
				return;
			}
		}
	}
	//for (std::list<GameObject*>::iterator it_childs = go->GetChild.begin(); it_childs != go->childs.end(); ++it_childs)
		//RecursiveGetAnimableGO((*it_childs), bone_transformation, anim);
}

void ModuleAnimation::MoveAnimationForward(float time, Animation* current_animation, float blend)
{

	for (uint i = 0; i < current_animation->animable_gos.size(); ++i)
	{
		ResourceAnimation::BoneTransformation* transform = current_animation->animable_data_map.find(current_animation->animable_gos[i])->second;

		if (transform)
		{
			math::float3 pos, scale;
			math::Quat rot;

			pos = current_animation->animable_gos[i]->transform->position;
			scale = current_animation->animable_gos[i]->transform->scale;
			rot = current_animation->animable_gos[i]->transform->rotation;

			float* prev_pos = nullptr;
			float* next_pos = nullptr;
			float time_pos_percentatge = 0.0f;

			float* prev_scale = nullptr;
			float* next_scale = nullptr;
			float time_scale_percentatge = 0.0f;

			float* prev_rot = nullptr;
			float* next_rot = nullptr;
			float time_rot_percentatge = 0.0f;

			float next_time = 0.0f;
			float prev_time = 0.0f;

			// -------- FINDING NEXT AND PREVIOUS TRANSFORMATIONS IN RELATION WITH THE GIVEN TIME (t) --------

			// Finding next and previous positions	
			if (transform->positions.count > i)
			{
				for (uint j = 0; j < transform->positions.count; ++j)
				{
					if (prev_pos != nullptr && next_pos != nullptr) // if prev and next postions have been found we stop
					{
						float time_interval = next_time - prev_time;
						time_pos_percentatge = (time - prev_time) / time_interval;
						break;
					}

					if (time == transform->positions.time[j]) // in this case interpolation won't be done
					{
						prev_pos = &transform->positions.value[j * 3];
						next_pos = prev_pos;
						break;
					}

					if (transform->positions.time[j] > time) // prev and next postions have been found
					{
						next_time = transform->positions.time[j];
						next_pos = &transform->positions.value[j * 3];

						prev_pos = &transform->positions.value[(j * 3) - 3];
						prev_time = transform->positions.time[j - 1];
					}
				}
			}

			// Finding next and previous scalings
			if (transform->scalings.count > i)
			{
				next_time = 0.0f;
				prev_time = 0.0f;

				for (uint j = 0; j < transform->scalings.count; ++j)
				{
					if (prev_scale != nullptr && next_scale != nullptr) // if prev and next scalings have been found we stop
					{
						float time_interval = next_time - prev_time;
						time_scale_percentatge = (time - prev_time) / time_interval;
						break;
					}

					if (time == transform->scalings.time[j]) // in this case interpolation won't be done
					{
						prev_scale = &transform->scalings.value[j * 3];
						next_scale = prev_scale;
						break;
					}

					if (transform->scalings.time[j] > time) // prev and next scalings have been found
					{
						next_time = transform->scalings.time[j];
						next_scale = &transform->scalings.value[j * 3];

						prev_scale = &transform->scalings.value[(j * 3) - 3];
						prev_time = transform->scalings.time[j - 1];
					}
				}
			}

			// Finding next and previous rotations
			if (transform->rotations.count > i)
			{
				next_time = 0.0f;
				prev_time = 0.0f;

				for (uint j = 0; j < transform->rotations.count; ++j)
				{
					if (prev_rot != nullptr && next_rot != nullptr) // if prev and next rotations have been found we stop
					{
						float time_interval = next_time - prev_time;
						time_rot_percentatge = (time - prev_time) / time_interval;
						break;
					}

					if (time == transform->rotations.time[j]) // in this case interpolation won't be done
					{
						prev_rot = &transform->rotations.value[j * 4];
						next_rot = prev_rot;
						break;
					}

					if (transform->rotations.time[j] > time) // prev and next rotations have been found
					{
						next_time = transform->rotations.time[j];
						next_rot = &transform->rotations.value[j * 4];

						prev_rot = &transform->rotations.value[(j * 4) - 4];
						prev_time = transform->rotations.time[j - 1];
					}
				}
			}

			// -------- INTERPOLATIONS CALCULATIONS --------

			// Interpolating positions
			if (current_animation->interpolate && prev_pos != nullptr && next_pos != nullptr && prev_pos != next_pos)
			{
				math::float3 prev_pos_lerp(prev_pos[0], prev_pos[1], prev_pos[2]);
				math::float3 next_pos_lerp(next_pos[0], next_pos[1], next_pos[2]);
				pos = math::float3::Lerp(prev_pos_lerp, next_pos_lerp, time_pos_percentatge);
			}
			else if (prev_pos != nullptr && (!current_animation->interpolate || prev_pos == next_pos))
				pos = math::float3(prev_pos[0], prev_pos[1], prev_pos[2]);

			// Interpolating scalings
			if (current_animation->interpolate && prev_scale != nullptr && next_scale != nullptr && prev_scale != next_scale)
			{
				math::float3 prev_scale_lerp(prev_scale[0], prev_scale[1], prev_scale[2]);
				math::float3 next_scale_lerp(next_scale[0], next_scale[1], next_scale[2]);
				scale = math::float3::Lerp(prev_scale_lerp, next_scale_lerp, time_scale_percentatge);
			}
			else if (prev_scale != nullptr && (!current_animation->interpolate || prev_scale == next_scale))
				scale = math::float3(prev_scale[0], prev_scale[1], prev_scale[2]);

			// Interpolating rotations
			if (current_animation->interpolate && prev_rot != nullptr && next_rot != nullptr && prev_rot != next_rot)
			{
				math::Quat prev_rot_lerp(prev_rot[0], prev_rot[1], prev_rot[2], prev_rot[3]);
				math::Quat next_rot_lerp(next_rot[0], next_rot[1], next_rot[2], next_rot[3]);
				rot = math::Quat::Slerp(prev_rot_lerp, next_rot_lerp, time_rot_percentatge);
			}
			else if (prev_rot != nullptr && (!current_animation->interpolate || prev_rot == next_rot))
				rot = math::Quat(prev_rot[0], prev_rot[1], prev_rot[2], prev_rot[3]);

			if (blend >= 1.f)
			{
				current_animation->animable_gos[i]->transform->position = pos;
				current_animation->animable_gos[i]->transform->scale = scale;
				current_animation->animable_gos[i]->transform->rotation = rot;
			}
			else
			{
				math::float3 pos2, scale2;
				math::Quat rot2;

				pos2 = current_animation->animable_gos[i]->transform->position;
				scale2 = current_animation->animable_gos[i]->transform->scale;
				rot2 = current_animation->animable_gos[i]->transform->rotation;

				math::float3 pos3, scale3;
				math::Quat rot3;

				pos3 = math::float3::Lerp(pos2, pos, blend);
				scale3 = math::float3::Lerp(scale2, scale, blend);
				rot3 = math::Quat::Slerp(rot2, rot, blend);
				
				current_animation->animable_gos[i]->transform->position = pos3;
				current_animation->animable_gos[i]->transform->scale = scale3;
				current_animation->animable_gos[i]->transform->rotation = rot3;

			}
		}

	}
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
	current_anim->anim_timer = time;
	MoveAnimationForward(current_anim->anim_timer, current_anim);
}

void ModuleAnimation::SetCurrentAnimation(int i)
{
	anim_state = BLENDING;
	blend_timer = 0.0f;
	last_anim = current_anim;
	current_anim = animations.at(i);
	SetCurrentAnimationTime(0.0f);
}

void ModuleAnimation::CleanAnimableGOS()
{
	for (uint i = 0; i < animations.size(); ++i)
	{
		animations.at(i)->animable_gos.clear();
		animations.at(i)->animable_data_map.clear();
	}

	for (std::vector<Animation*>::iterator it = animations.begin(); it != animations.end(); ++it)
		RELEASE(*it);

	animations.clear();
	current_anim = nullptr;
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
	ComponentMesh* mesh = component_bone->attached_mesh;
	//TODO_G: finish animation module
	/*if (mesh != nullptr)
	{
		const ResourceBone* rbone = (const ResourceBone*)component_bone->GetResource();
		const ResourceMesh* roriginal = (const ResourceMesh*)mesh->GetResource();
		ResourceMesh* tmp_mesh = (ResourceMesh*)mesh->GetResource();
		ResourceMesh* rmesh = (ResourceMesh*)tmp_mesh->deformable;

		float4x4 trans = component_bone->GetEmbeddedObject()->GetTransform()->GetMatrix();
		trans = trans * component_bone->attached_mesh->GetEmbeddedObject()->GetTransform()->GetLocal().Inverted();

		trans = trans * rbone->offset_matrix;

		for (uint i = 0; i < rbone->bone_weights_size; ++i)
		{
			uint index = rbone->bone_weights_indices[i];
			float3 original(&roriginal->vertices[index * 3]);

			float3 vertex = trans.TransformPos(original);

			rmesh->vertices[index * 3] += vertex.x * rbone->bone_weights[i] * SCALE;
			rmesh->vertices[index * 3 + 1] += vertex.y * rbone->bone_weights[i] * SCALE;
			rmesh->vertices[index * 3 + 2] += vertex.z * rbone->bone_weights[i] * SCALE;
		}
	}*/
}

void ModuleAnimation::ResetMesh(ComponentBone * component_bone)
{
	//TODO_G: finish animation module
	/*ResourceBone* rbone = (ResourceBone*)component_bone->GetResource();
	ResourceMesh* original = nullptr;
	if (rbone)
		original = (ResourceMesh*)App->resources->Get(rbone->mesh_uid);

	if (original)
		memset(original->deformable->vertices, 0, original->vertex_size * sizeof(float));*/
}