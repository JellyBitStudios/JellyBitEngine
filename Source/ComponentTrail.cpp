#include "ComponentTrail.h"

#include "Application.h"
#include "ModuleInput.h"
#include "ModuleResourceManager.h"
#include "ModuleInternalResHandler.h"

#include "GameObject.h"
#include "ComponentTransform.h"
#include "ModuleTrails.h"

#include "Brofiler/Brofiler.h"

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
#ifndef GAMEMODE
	BROFILER_CATEGORY(__FUNCTION__, Profiler::Color::PapayaWhip);
#endif // !GAMEMODE

	if (create)
	{
		// Get the new trail vertex
		math::float3 originHigh = parent->rotationBB.FaceCenterPoint(hight);
		math::float3 originLow = parent->rotationBB.FaceCenterPoint(low);

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
		ImGui::Text("Vector of creation");
		ImGui::SameLine();
		ImGui::ShowHelpMarker("Select the direction in which the trail will be created, this direction must be the direction of movement of the gameobject.\nYou shoud not use the Y vector.");

		/*Index of the OBB face to generate the point at. The valid range is [0, 5].
		This index corresponds to the planes in the order (-X, +X, -Y, +Y, -Z, +Z).*/
		if (ImGui::RadioButton("X", vector == TrailVector::X))
		{
			low = 4; hight = 5;
			vector = TrailVector::X;
		} ImGui::SameLine();
		if (ImGui::RadioButton("Y", vector == TrailVector::Y))
		{
			low = 2; hight = 3;
			vector = TrailVector::Y;
		} ImGui::SameLine();
		if (ImGui::RadioButton("Z", vector == TrailVector::Z))
		{
			low = 0; hight = 1;
			vector = TrailVector::Z;
		}

		ImGui::DragInt("Life Time", &lifeTime, 10.0f, 0, 10000);
		ImGui::SameLine();
		ImGui::ShowHelpMarker("Time in milliseconds that a trail plane will last in the world.");

		ImGui::DragFloat("Min Distance", &minDistance, 0.01f, 0.0f, 10.0f);
		ImGui::SameLine();
		ImGui::ShowHelpMarker("Minimum distance before a new plane is created in the trail.If the minimum distance is not exceeded, the last plane will be placed in the new position.");

		if (ImGui::CollapsingHeader("Trail Material", ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_DefaultOpen))
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

			ImGui::ColorEdit4("Trail color", color.ptr(), ImGuiColorEditFlags_None);

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

inline void ComponentTrail::Start()
{
	create = true;
}

inline void ComponentTrail::Stop()
{
	create = false;
}

inline void ComponentTrail::HardStop()
{
	create = false;

	for (std::list< TrailNode*>::iterator curr = trailVertex.begin(); curr != trailVertex.end(); ++curr)
	{
		delete *curr;
		*curr = nullptr;
	}

	trailVertex.clear();
}

uint ComponentTrail::GetInternalSerializationBytes()
{
	return sizeof(materialRes) + sizeof(minDistance) + sizeof(lifeTime) + sizeof(create);
}

void ComponentTrail::OnInternalSave(char *& cursor)
{
	size_t bytes = sizeof(materialRes);
	memcpy(cursor, &materialRes, bytes);
	cursor += bytes;

	bytes = sizeof(minDistance);
	memcpy(cursor, &minDistance, bytes);
	cursor += bytes;

	bytes = sizeof(lifeTime);
	memcpy(cursor, &lifeTime, bytes);
	cursor += bytes;

	bytes = sizeof(create);
	memcpy(cursor, &create, bytes);
	cursor += bytes;
}

void ComponentTrail::OnInternalLoad(char *& cursor)
{
	size_t bytes = sizeof(materialRes);
	memcpy(&materialRes, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(minDistance);
	memcpy(&minDistance, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(lifeTime);
	memcpy(&lifeTime, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(create);
	memcpy(&create, cursor, bytes);
	cursor += bytes;
}