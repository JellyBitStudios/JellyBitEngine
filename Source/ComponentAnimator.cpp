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
	return sizeof(uint);
}

void ComponentAnimator::OnInternalSave(char*& cursor)
{
	size_t bytes = sizeof(uint);
	memcpy(cursor, &res, bytes);
	cursor += bytes;
}

void ComponentAnimator::OnInternalLoad(char*& cursor)
{
	uint loadedRes;
	size_t bytes = sizeof(uint);
	memcpy(&loadedRes, cursor, bytes);
	cursor += bytes;
	SetResourceAnimator(loadedRes);
}

bool ComponentAnimator::SetResourceAnimator(uint resource)
{
	if (res > 0)
		App->res->SetAsUnused(res);

	if (resource > 0) {
		App->res->SetAsUsed(resource);
		ResourceAnimator* anim_res = (ResourceAnimator*)App->res->GetResource(res);
		if (anim_res)
			anim_res->InitAnimator();
	}

	res = resource;

	return true;
}

bool ComponentAnimator::SetResourceAvatar(uint resource)
{
	if (res > 0)
		App->res->SetAsUnused(res);

	if (resource > 0)
		App->res->SetAsUsed(resource);

	res_avatar = resource;

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
			ImGui::Text("Meshes affected: %i", resource->animator_data.meshes_uuids.size());
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
	}
#endif
}