#include "ComponentMaterial.h"

#include "Application.h"
#include "ModuleResourceManager.h"
#include "ModuleInternalResHandler.h"
#include "Resource.h"
#include "ResourceMaterial.h"

#include "ModuleScene.h"

#ifndef GAMEMODE
#include "imgui\imgui.h"
#endif

#include <assert.h>

ComponentMaterial::ComponentMaterial(GameObject* parent) : Component(parent, ComponentTypes::MaterialComponent) 
{
	SetResource(App->resHandler->defaultMaterial);
}

ComponentMaterial::ComponentMaterial(const ComponentMaterial& componentMaterial, GameObject* parent) : Component(parent, ComponentTypes::MaterialComponent)
{
	if (App->res->GetResource(componentMaterial.res) != nullptr)
		SetResource(componentMaterial.res);
	else
		SetResource(App->resHandler->defaultMaterial);
}

ComponentMaterial::~ComponentMaterial() 
{
	SetResource(0);
}

void ComponentMaterial::OnUniqueEditor()
{
#ifndef GAMEMODE
	if (ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen))
	{
		const Resource* resource = currentResource;

		ImGui::PushID("material");
		ImGui::Button(resource->GetName(), ImVec2(150.0f, 0.0f));
		ImGui::PopID();

		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::Text("%u", res);
			ImGui::EndTooltip();
		}

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MATERIAL_INSPECTOR_SELECTOR"))
			{
				uint payload_n = *(uint*)payload->Data;
				SetResource(payload_n);
			}
			ImGui::EndDragDropTarget();
		}

		if (res != App->resHandler->defaultMaterial)
		{
			ImGui::SameLine();

			if (ImGui::Button("EDIT"))
				SELECT(App->res->GetResource(res));
		}

		if (ImGui::SmallButton("Use default material"))
			SetResource(App->resHandler->defaultMaterial);

		ImGui::Spacing();


	}
#endif
}

uint ComponentMaterial::GetInternalSerializationBytes()
{
	return sizeof(uint);
}

void ComponentMaterial::OnInternalSave(char*& cursor)
{
	size_t bytes = sizeof(uint);
	memcpy(cursor, &res, bytes);
	cursor += bytes;
}

void ComponentMaterial::OnInternalLoad(char*& cursor)
{
	size_t bytes = sizeof(uint);
	uint resource = 0;
	memcpy(&resource, cursor, bytes);

	if (App->res->GetResource(resource) != nullptr)
		SetResource(resource);
	else
		SetResource(App->resHandler->defaultMaterial);

	cursor += bytes;
}

// ----------------------------------------------------------------------------------------------------

void ComponentMaterial::SetResource(uint materialUuid)
{
	if (res > 0)
		App->res->SetAsUnused(res);

	if (materialUuid > 0)
		App->res->SetAsUsed(materialUuid);

	res = materialUuid;

	ResourceMaterial* mat = (ResourceMaterial*)App->res->GetResource(res);
	if (mat)
	{
		currentResource = mat;
		return;
	}
	currentResource = 0;
}

void ComponentMaterial::SetResourceByName(std::string materialName)
{
	if (res > 0)
		App->res->SetAsUnused(res);

	uint materialUuid = 0;

	std::vector<Resource*> materials = App->res->GetResourcesByType(ResourceTypes::MaterialResource);
	for (int i = 0; i < materials.size(); ++i)
	{
		ResourceMaterial* material = (ResourceMaterial*)materials[i];
		if (materialName == material->GetName())
		{
			materialUuid = material->GetUuid();
			break;
		}
	}

	if (materialUuid > 0)
		App->res->SetAsUsed(materialUuid);

	res = materialUuid;

	ResourceMaterial* mat = (ResourceMaterial*)App->res->GetResource(res);
	if (mat)
	{
		currentResource = mat;
		return;
	}
	currentResource = 0;
}

void ComponentMaterial::SetColor(math::float4& color)
{
	this->color = color;

	// r
	if (this->color.x < 0.0f)
		this->color.x = 0.0f;
	else if (this->color.x > 1.0f)
		this->color.x = 1.0f;

	// g
	if (this->color.y < 0.0f)
		this->color.y = 0.0f;
	else if (this->color.y > 1.0f)
		this->color.y = 1.0f;

	// b
	if (this->color.z < 0.0f)
		this->color.z = 0.0f;
	else if (this->color.z > 1.0f)
		this->color.z = 1.0f;

	// a
	if (this->color.w < 0.0f)
		this->color.w = 0.0f;
	else if (this->color.w > 1.0f)
		this->color.w = 1.0f;
}

math::float4 ComponentMaterial::GetColor() const
{
	return color;
}

void ComponentMaterial::SetPct(float pct)
{
	this->pct = pct;

	if (this->pct < 0.0f)
		this->pct = 0.0f;
	else if (this->pct > 1.0f)
		this->pct = 1.0f;
}

float ComponentMaterial::GetPct() const
{
	return pct;
}