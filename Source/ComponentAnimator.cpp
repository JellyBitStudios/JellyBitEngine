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
#ifndef GAMEMODE
#include "imgui\imgui.h"
#endif

ComponentAnimator::ComponentAnimator(GameObject * embedded_game_object) :
	Component(embedded_game_object, ComponentTypes::AnimatorComponent)
{
}

ComponentAnimator::ComponentAnimator(GameObject* embedded_game_object, uint resource) :
	Component(embedded_game_object, ComponentTypes::AnimatorComponent)
{
	this->SetResourceAnimator(resource);
}

ComponentAnimator::ComponentAnimator(const ComponentAnimator & component_anim, GameObject * parent, bool include) : Component(parent, ComponentTypes::AnimatorComponent)
{
	this->SetResourceAnimator(component_anim.res);
	this->SetResourceAvatar(component_anim.res_avatar);
	//App->animation->SetAnimationGos((ResourceAnimation*)App->res->GetResource(res));
}

ComponentAnimator::~ComponentAnimator()
{

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

	if (resource > 0) {
		App->res->SetAsUsed(resource);
		ResourceAnimator* animator = (ResourceAnimator*)App->res->GetResource(resource);
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

	if (resource > 0) {
		App->res->SetAsUsed(resource);
		res_animations.push_back(resource);
		ResourceAnimator* animator = (ResourceAnimator*)App->res->GetResource(res);
		animator->AddAnimationFromAnimationResource((ResourceAnimation*)App->res->GetResource(resource));
		animator->animator_data.animations_uuids.push_back(resource);
	}
	return true;
}

void ComponentAnimator::Update()
{
	if (res != 0) {
		ResourceAnimator* anim_res = (ResourceAnimator*)App->res->GetResource(res);

		if (anim_res) {
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

		std::string fileName = "Empty Animator";
		const ResourceAnimator* resource = (ResourceAnimator*)App->res->GetResource(res);
		if (resource != nullptr)
			fileName = resource->GetName();

		if (resource) {
			ImGui::Text("Animator name: %s", resource->animator_data.name.data());
			ImGui::Text("Animator resource UUID: %i", resource->GetUuid());
			ImGui::Text("Avatar UUID: %i", resource->animator_data.avatar_uuid);
			ImGui::Text("Animations size: %i", resource->animator_data.animations_uuids.size());
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
		const ResourceAnimation* resource_animation = (res_animations.size() == 0) ? nullptr : (ResourceAnimation*)App->res->GetResource(res_animations.at(0));
		if (resource_animation != nullptr)
			fileName = resource_animation->GetName();

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