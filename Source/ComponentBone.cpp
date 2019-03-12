#include "ComponentBone.h"

#include "Application.h"
#include "ModuleResourceManager.h"
#include "ResourceBone.h"

#ifndef GAMEMODE
#include "imgui\imgui.h"
#endif

ComponentBone::ComponentBone(GameObject* parent) : Component(parent, ComponentTypes::BoneComponent) {}

ComponentBone::ComponentBone(const ComponentBone& componentBone, GameObject* parent) : Component(parent, ComponentTypes::BoneComponent)
{
	if (App->res->GetResource(componentBone.res) != nullptr)
		SetResource(componentBone.res);
}

ComponentBone::~ComponentBone() 
{
	SetResource(0);
}

void ComponentBone::OnUniqueEditor()
{
#ifndef GAMEMODE
	if (ImGui::CollapsingHeader("Bone", ImGuiTreeNodeFlags_DefaultOpen))
	{
		const ResourceBone* boneResource = (const ResourceBone*)App->res->GetResource(res);

		ImGui::PushID("bone");
		ImGui::Button(boneResource != nullptr ? boneResource->boneData.name.data() : "Empty Bone", ImVec2(150.0f, 0.0f));
		ImGui::PopID();

		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::Text("%u", res);
			ImGui::EndTooltip();
		}
	}
#endif
}

uint ComponentBone::GetInternalSerializationBytes()
{
	return sizeof(uint);
}

void ComponentBone::OnInternalSave(char*& cursor)
{
	size_t bytes = sizeof(uint);
	memcpy(cursor, &res, bytes);
	cursor += bytes;
}

void ComponentBone::OnInternalLoad(char*& cursor)
{
	size_t bytes = sizeof(uint);
	uint resource = 0;
	memcpy(&resource, cursor, bytes);

	if (App->res->GetResource(resource) != nullptr)
		SetResource(resource);

	cursor += bytes;
}

// ----------------------------------------------------------------------------------------------------

void ComponentBone::SetResource(uint boneUuid)
{
	if (res > 0)
		App->res->SetAsUnused(res);

	if (boneUuid > 0)
		App->res->SetAsUsed(boneUuid);

	res = boneUuid;
}