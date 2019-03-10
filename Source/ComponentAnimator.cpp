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
	res = resource;
}

ComponentAnimator::ComponentAnimator(const ComponentAnimator & component_anim, GameObject * parent, bool include) : Component(parent, ComponentTypes::AnimatorComponent)
{
	this->SetResource(component_anim.res);
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
	SetResource(loadedRes);
}

bool ComponentAnimator::SetResource(uint resource) //check all this
{
	res = resource;

	return true;
}

void ComponentAnimator::Update()
{
	if (res != 0) {
		ResourceAnimator* anim_res = (ResourceAnimator*)App->res->GetResource(res);


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

		std::string fileName = "Empty Animation";
		const ResourceAnimator* resource = (ResourceAnimator*)App->res->GetResource(res);
		if (resource != nullptr)
			fileName = resource->GetName();

		ImGui::Text("Animator name: %s", resource->animator_data.name.data());
		ImGui::Text("Animator resource UUID: %i", resource->GetUuid());
		ImGui::Text("Avatar UUID: %i", resource->animator_data.avatar_uuid);
		ImGui::Text("Meshes affected: %i", resource->animator_data.meshes_uuids.size());
		ImGui::Text("Animations size: %i", resource->animator_data.animations_uuids.size());

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
				// TODO check res type and do things:

				SetResource(payload_n);
			}
			ImGui::EndDragDropTarget();
		}
	}
#endif
}