#include "ComponentAnimator.h"

#include "ComponentMesh.h"
#include "Application.h"
#include "ModuleResourceManager.h"
#include "ModuleRenderer3D.h"
#include "ModuleFileSystem.h"
#include "SceneImporter.h"
#include "GameObject.h"
#include "Resource.h"
#include "ResourceMesh.h"
#include "ResourceAnimator.h"
#include "ResourceAnimation.h"
#include "AnimationImporter.h"
#include "ResourceAvatar.h"
#include "ModuleGOs.h"

#ifndef GAMEMODE
#include "imgui\imgui.h"
#endif

ComponentAnimator::ComponentAnimator(GameObject* embedded_game_object) :
	Component(embedded_game_object, ComponentTypes::AnimatorComponent)
{ 
	go_with_mesh = nullptr;
}

ComponentAnimator::ComponentAnimator(GameObject* embedded_game_object, uint resource) :
	Component(embedded_game_object, ComponentTypes::AnimatorComponent)
{
	SetResourceAnimator(resource);
	go_with_mesh = nullptr;
}

ComponentAnimator::ComponentAnimator(const ComponentAnimator & component_anim, GameObject * parent, bool include) : Component(parent, ComponentTypes::AnimatorComponent)
{
	if (include) 
	{
		// 1. Create and set new resource animator
		ResourceData genericAnimatorData;

		char animatorName[DEFAULT_BUF_SIZE];
		sprintf_s(animatorName, DEFAULT_BUF_SIZE, "BuiltinAnimator%u", App->res->GetResourcesByType(ResourceTypes::AnimatorResource).size());
		genericAnimatorData.name = animatorName;

		ResourceAnimatorData specificAnimatorData;

		std::string outputFileAnimator;
		this->SetResourceAnimator(App->res->ExportFile(ResourceTypes::AnimatorResource, genericAnimatorData, &specificAnimatorData, outputFileAnimator)->GetUuid());

		// 2. Create and set new resource avatar
		ResourceData genericAvatarData;

		char avatarName[DEFAULT_BUF_SIZE];
		sprintf_s(avatarName, DEFAULT_BUF_SIZE, "BuiltinAvatar%u", App->res->GetResourcesByType(ResourceTypes::AvatarResource).size());
		genericAvatarData.name = avatarName;

		ResourceAvatarData specificAvatarData;

		std::string outputFile;
		SetResourceAvatar(App->res->ExportFile(ResourceTypes::AvatarResource, genericAvatarData, &specificAvatarData, outputFile)->GetUuid());

		// 3. Add animations
		for (uint i = 0u; i < component_anim.res_animations.size(); i++)
		{
			this->SetResourceAnimation(component_anim.res_animations.at(i));
		}
	}
	go_with_mesh = nullptr;
	//App->animation->SetAnimationGos((ResourceAnimation*)App->res->GetResource(res));
}

ComponentAnimator::~ComponentAnimator()
{
	SetResourceAnimator(0);
	SetResourceAvatar(0);
	SetResourceAnimation(0);
}

uint ComponentAnimator::GetInternalSerializationBytes()
{
	return sizeof(uint) + sizeof(uint) + sizeof(uint) + sizeof(uint) * res_animations.size();
}

void ComponentAnimator::OnInternalSave(char*& cursor)
{
	size_t bytes = sizeof(uint);

	memcpy(cursor, &res, bytes); // resource animator
	cursor += bytes;

	bytes = sizeof(uint);
	memcpy(cursor, &res_avatar, bytes); // resource avatar
	cursor += bytes;

	bytes = sizeof(uint);
	uint anim_size = res_animations.size();
	memcpy(cursor, &anim_size, bytes); // anim size
	cursor += bytes;

	for (uint i = 0u; i < res_animations.size(); i++)
	{
		bytes = sizeof(uint);
		memcpy(cursor, &res_animations[i], bytes); // resource animation
		cursor += bytes;
	}
	go_with_mesh = nullptr;
}

void ComponentAnimator::OnInternalLoad(char*& cursor)
{
	uint loadedRes, loadedAva, animSize, loadedAni;
	size_t bytes = sizeof(uint);
	memcpy(&loadedRes, cursor, bytes);
	cursor += bytes;
	SetResourceAnimator(loadedRes);

	bytes = sizeof(uint);
	memcpy(&loadedAva, cursor, bytes);
	cursor += bytes;
	SetResourceAvatar(loadedAva);

	bytes = sizeof(uint);
	memcpy(&animSize, cursor, bytes);
	cursor += bytes;
	
	for (uint i = 0u; i < animSize; i++)
	{
		bytes = sizeof(uint);
		memcpy(&loadedAni, cursor, bytes);
		cursor += bytes;
		SetResourceAnimation(loadedAni);
	}
	go_with_mesh = nullptr;
}

bool ComponentAnimator::SetResourceAnimator(uint resource)
{
	if (res > 0)
		App->res->SetAsUnused(res);

	if (resource > 0) {
		App->res->SetAsUsed(resource);
		ResourceAnimator* anim_res = (ResourceAnimator*)App->res->GetResource(resource);
		if (anim_res)
			anim_res->InitAnimator();
	}

	res = resource;

	return true;
}

bool ComponentAnimator::SetResourceAvatar(uint resource)
{
	if (res_avatar > 0)
		App->res->SetAsUnused(res_avatar);

	if (resource > 0) 
	{
		// 1. Initialize avatar
		ResourceAvatar* avatar = (ResourceAvatar*)App->res->GetResource(resource);
		avatar->Initalize(parent->GetUUID());

		// 2. Set avatar as used
		App->res->SetAsUsed(resource);

		ResourceAnimator* animator = (ResourceAnimator*)App->res->GetResource(res);
		animator->animator_data.avatar_uuid = resource; // TODO_G : this is ugly and needs to be improved >:(
	}

	res_avatar = resource;

	return true;
}

bool ComponentAnimator::SetResourceAnimation(uint resource)
{
	for (uint i = 0u; i < res_animations.size(); i++)
	{
		if (res_animations[i] == resource) {
			App->res->SetAsUnused(res);
			break;
		}
	}

	if (resource > 0){
		App->res->SetAsUsed(resource);
		res_animations.push_back(resource);
		ResourceAnimator* animator = (ResourceAnimator*)App->res->GetResource(res);
		animator->AddAnimationFromAnimationResource((ResourceAnimation*)App->res->GetResource(resource));
		animator->animator_data.animations_uuids.push_back(resource);
	}


	return true;
}

bool ComponentAnimator::CleanAnimations() {

	res_animations.clear();
	ResourceAnimator* animator = (ResourceAnimator*)App->res->GetResource(res);

	animator->ClearAnimations();

	return true;
}

bool ComponentAnimator::AnimationFinished()const
{
	ResourceAnimator* animator_res = (ResourceAnimator*)App->res->GetResource(res);
	if (!animator_res)
		return false;

	ResourceAnimator::Animation* animation_res = (ResourceAnimator::Animation*)animator_res->GetCurrentAnimation();
	if (!animation_res)
		return false;

	return animation_res->finished;
}

bool ComponentAnimator::UpdateAnimationSpeed(float new_speed)
{
	ResourceAnimator* animator_res = (ResourceAnimator*)App->res->GetResource(res);
	if (!animator_res)
		return false;

	animator_res->SetAnimationSpeed(new_speed);
	
	return true;
}

bool ComponentAnimator::UpdateBlendTime(float new_blend)
{
	ResourceAnimator* animator_res = (ResourceAnimator*)App->res->GetResource(res);
	if (!animator_res)
		return false;

	animator_res->SetAnimationBlendTime(new_blend);

	return true;
}

void ComponentAnimator::SetAnimationLoop(bool loop)
{
	ResourceAnimator* animator_res = (ResourceAnimator*)App->res->GetResource(res);
	if (!animator_res)
		return;

	ResourceAnimator::Animation* animation_res = (ResourceAnimator::Animation*)animator_res->GetCurrentAnimation();
	if (!animation_res)
		return;

	animation_res->loop = loop;
}

void ComponentAnimator::Update()
{
	if (res != 0) {
		ResourceAnimator* anim_res = (ResourceAnimator*)App->res->GetResource(res);
		ResourceAvatar* avatar_res = (ResourceAvatar*)App->res->GetResource(res_avatar);
		if (anim_res && avatar_res) {
			avatar_res->SetIsAnimated(true);
			anim_res->Update();
		}
	}
}

bool ComponentAnimator::PlayAnimation(const char* anim_name)
{
	ResourceAnimator* anim_res = (ResourceAnimator*)App->res->GetResource(res);

	if (anim_res)
		return anim_res->SetCurrentAnimation(anim_name);
	else
		return false;
}

const char * ComponentAnimator::GetCurrentAnimationName()
{
	ResourceAnimator* anim_res = (ResourceAnimator*)App->res->GetResource(res);
	if (!anim_res)
		return nullptr;
	else
		return anim_res->GetCurrentAnimation()->name.data();
}

int ComponentAnimator::GetCurrentAnimationFrame()const
{
	ResourceAnimator* animator_res = (ResourceAnimator*)App->res->GetResource(res);
	if (!animator_res)
		return -1;

	ResourceAnimator::Animation* animation_res = (ResourceAnimator::Animation*)animator_res->GetCurrentAnimation();
	if (!animation_res)
		return -1;

	float current_animation_time = animator_res->GetCurrentAnimationTime();

	for (uint j = 0; j < animation_res->boneKeys[0].positions.count; ++j)
	{
		if (current_animation_time < animation_res->boneKeys[0].positions.time[j])
			return j - 1;
		else if (current_animation_time == animation_res->boneKeys[0].positions.time[j])
			return j;
	}

	for (uint j = 0; j < animation_res->boneKeys[0].scalings.count; ++j)
	{
		if (current_animation_time < animation_res->boneKeys[0].scalings.time[j])
			return j - 1;
		else if (current_animation_time == animation_res->boneKeys[0].scalings.time[j])
			return j;
	}

	for (uint j = 0; j < animation_res->boneKeys[0].rotations.count; ++j)
	{
		if (current_animation_time < animation_res->boneKeys[0].rotations.time[j])
			return j - 1;
		else if (current_animation_time == animation_res->boneKeys[0].rotations.time[j])
			return j;
	}
	
	return -1;
}

void ComponentAnimator::OnEditor()
{
	OnUniqueEditor();
}

void ComponentAnimator::OnUniqueEditor()
{
#ifndef GAMEMODE
	if (ImGui::CollapsingHeader("Animator", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text("Animator");
		ImGui::SameLine();
		ImGui::Text("Drag n drop in this order please");

		std::string fileName = "Empty Animator";
		ResourceAnimator* resource = (ResourceAnimator*)App->res->GetResource(res);
		if (resource != nullptr)
			fileName = resource->GetName();

		if (resource) {
			ImGui::Text("Animator name: %s", resource->animator_data.name.data());
			ImGui::Text("Animator resource UUID: %i", resource->GetUuid());
			ImGui::Text("Avatar UUID: %i", resource->animator_data.avatar_uuid);
			ImGui::Text("Meshes affected: %i", resource->animator_data.meshes_uuids.size());
			ImGui::Text("Animations size: %i", res_animations.size());

			if (ImGui::Button("PLAY"))
				resource->PlayAnimation();

			ImGui::SameLine();

			if (ImGui::Button("PAUSE"))
				resource->PauseAnimation();

			ImGui::SameLine();

			if (ImGui::Button("STOP"))
				resource->StopAnimation();

			ImGui::SameLine();

			if (ImGui::Button("FORWARD"))
				resource->StepForward();

			ImGui::SameLine();

			if (ImGui::Button("BACKWARDS"))
				resource->StepBackwards();

			if (ImGui::Button("CLEAN ANIMATIONS")) {
				this->CleanAnimations();
			}
				
		}

		

		ImGui::PushID("animator");
		ImGui::Button(fileName.data(), ImVec2(150.0f, 0.0f));
		ImGui::PopID();

		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::Text("%u", res);
			ImGui::EndTooltip();
		}

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ANIMATOR_INSPECTOR_SELECTOR"))
			{
				uint payload_n = *(uint*)payload->Data;
				
				SetResourceAnimator(payload_n);
			}
			ImGui::EndDragDropTarget();
		}

		fileName = "Empty Avatar";
		const ResourceAvatar* resource_avatar = (ResourceAvatar*)App->res->GetResource(res_avatar);
		if (resource_avatar != nullptr)
			fileName = resource_avatar->GetName();

		ImGui::PushID("avatar");
		ImGui::Button(fileName.data(), ImVec2(150.0f, 0.0f));
		ImGui::PopID();

		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::Text("%u", res_avatar);
			ImGui::EndTooltip();
		}

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("AVATAR_INSPECTOR_SELECTOR"))
			{
				uint payload_n = *(uint*)payload->Data;

				SetResourceAvatar(payload_n);
			}
			ImGui::EndDragDropTarget();
		}

		fileName = "Empty Animations";
		uint resource_animations = (res_animations.size() == 0) ? 0u : res_animations.size();
		if (resource_animations > 0u) {
			fileName = "Loaded animations: ";
			fileName.append(std::to_string(resource_animations));
		}

		ImGui::PushID("animation");
		ImGui::Button(fileName.data(), ImVec2(150.0f, 0.0f));
		ImGui::PopID();

		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::Text("%u", res_avatar);
			ImGui::EndTooltip();
		}

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ANIMATION_INSPECTOR_SELECTOR"))
			{
				uint payload_n = *(uint*)payload->Data;

				SetResourceAnimation(payload_n);
			}
			ImGui::EndDragDropTarget();
		}
	}
#endif
}