#include "ComponentTrail.h"

#include "Application.h"
#include "ModuleInput.h"
#include "ModuleResourceManager.h"
#include "ModuleInternalResHandler.h"

#include "GameObject.h"
#include "ComponentTransform.h"
#include "ModuleTrails.h"

#include "glew/include/GL/glew.h"

#include "imgui\imgui.h"



ComponentTrail::ComponentTrail(GameObject * parent) : Component(parent, ComponentTypes::TrailComponent)
{
	App->trails->trails.push_back(this);
	timer.Start();

	App->res->SetAsUsed(App->resHandler->plane);
}

ComponentTrail::ComponentTrail(const ComponentTrail & componentTransform, GameObject * parent) : Component(parent, ComponentTypes::TrailComponent)
{
	timer.Start();

}

ComponentTrail::~ComponentTrail()
{
	for (std::list< TrailNode*>::iterator curr = trailVertex.begin(); curr != trailVertex.end(); ++curr)
	{
		delete *curr;
		*curr = nullptr;
	}

	trailVertex.clear();

	App->trails->trails.remove(this);
}

void ComponentTrail::Update() 
{

	if (create)
	{
		// Get the new trail vertex
		math::float3 originHigh = parent->boundingBox.FaceCenterPoint(5);
		math::float3 originLow = parent->boundingBox.FaceCenterPoint(4);

		// Check we already have a trail
		if (trailVertex.size() > 1)
		{
			TrailNode* last = trailVertex.back();

			// If the distance between the last vertex and the new vertex is
			// greater than the minimum distance, we add a new node to the trail
			if (last && originHigh.Distance(last->originHigh) > minDistance)
			{
				TrailNode* node = new TrailNode(originHigh, originLow);

				trailVertex.push_back(node);
			}

			// Otherwise we modify the last trail node
			else
			{
				last->originHigh = originHigh;
				last->originLow = originLow;

				last->createTime = SDL_GetTicks();
			}
		}

		// If we do not have a trail, we add the node directly
		else
		{
			TrailNode* node = new TrailNode(originHigh, originLow);

			trailVertex.push_back(node);
		}

		timer.Start();
	}

	// Remove old nodes
	Uint32 time = SDL_GetTicks();
	std::list< TrailNode*> toRemove;
	for (std::list< TrailNode*>::iterator curr = trailVertex.begin(); curr != trailVertex.end(); ++curr)
	{
		if (time - (*curr)->createTime > lifeTime)
		{
			toRemove.push_back(*curr);
		}
		else break;
	}

	for (std::list< TrailNode*>::iterator it = toRemove.begin(); it != toRemove.end(); ++it)
	{
		trailVertex.remove(*it);
		delete *it;
	}


	if (App->input->GetKey(SDL_SCANCODE_1) == KEY_DOWN)
	{
		trailVertex.clear();
	}
	if (App->input->GetKey(SDL_SCANCODE_2) == KEY_DOWN)
	{
		create = !create;
	}
}


void ComponentTrail::OnUniqueEditor()
{
#ifndef GAMEMODE

	if (ImGui::CollapsingHeader("Trail", ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::DragInt("Life Time", &lifeTime, 10.0f, 0, 10000);

		ImGui::DragFloat("Min Distance", &minDistance, 0.01f, 0.0f, 10.0f);

		if (ImGui::CollapsingHeader("Texture", ImGuiTreeNodeFlags_FramePadding))
		{
			const Resource* resource = App->res->GetResource(materialRes);
			std::string materialName = "No  Material";
			if (resource)
			materialName = resource->GetName();
			ImGui::PushID("material");
			ImGui::Button(materialName.data(), ImVec2(150.0f, 0.0f));
			ImGui::PopID();

			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::Text("%u", materialRes);
				ImGui::EndTooltip();
			}

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MATERIAL_INSPECTOR_SELECTOR"))
				{
					uint payload_n = *(uint*)payload->Data;
					SetMaterialRes(payload_n);
				}
				ImGui::EndDragDropTarget();
			}

			if (ImGui::SmallButton("Use default material"))
				SetMaterialRes(App->resHandler->defaultMaterial);
			ImGui::Separator();
		}
	}

#endif
}


void ComponentTrail::SetMaterialRes(uint materialUuid)
{
	if (materialRes > 0)
		App->res->SetAsUnused(materialRes);

	if (materialUuid > 0)
		App->res->SetAsUsed(materialUuid);

	materialRes = materialUuid;
}
