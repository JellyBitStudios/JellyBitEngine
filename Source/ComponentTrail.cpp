#include "ComponentTrail.h"

#include "Application.h"
#include "ModuleInput.h"
#include "ModuleResourceManager.h"

#include "GameObject.h"
#include "ComponentTransform.h"

#include "glew/include/GL/glew.h"

#include "imgui\imgui.h"



ComponentTrail::ComponentTrail(GameObject * parent) : Component(parent, ComponentTypes::TrailComponent)
{
	timer.Start();
}

ComponentTrail::ComponentTrail(const ComponentTrail & componentTransform, GameObject * parent) : Component(parent, ComponentTypes::TrailComponent)
{
	timer.Start();

}

ComponentTrail::~ComponentTrail()
{
}

void ComponentTrail::Update() 
{

	if (create)
	{
		math::float3 originHigh = parent->boundingBox.FaceCenterPoint(5);
		math::float3 originLow = parent->boundingBox.FaceCenterPoint(4);

		if (test.size() > 1)
		{
			TrailNode* last = test.back();

			if (last && originHigh.Distance(last->originHigh) > 0.05f)
			{
				TrailNode* node = new TrailNode(originHigh, originLow);

				test.push_back(node);
			}

			else
			{
				last->originHigh = originHigh;
				last->originLow = originLow;

				last->createTime = SDL_GetTicks();
			}
		}
		else
		{
			TrailNode* node = new TrailNode(originHigh, originLow);

			test.push_back(node);
		}

		timer.Start();
	}

	Uint32 time = SDL_GetTicks();
	std::list< TrailNode*> toRemove;
	for (std::list< TrailNode*>::iterator curr = test.begin(); curr != test.end(); ++curr)
	{
		if (time - (*curr)->createTime > 1000)
		{
			toRemove.push_back(*curr);
		}
		else break;
	}
	for (std::list< TrailNode*>::iterator it = toRemove.begin(); it != toRemove.end(); ++it)
	{
		test.remove(*it);
		delete *it;
	}

	if (App->input->GetKey(SDL_SCANCODE_1) == KEY_DOWN)
	{
		test.clear();
	}
	if (App->input->GetKey(SDL_SCANCODE_2) == KEY_DOWN)
	{
		create = !create;
	}
}


void ComponentTrail::OnUniqueEditor()
{
	if (ImGui::CollapsingHeader("Particle Texture", ImGuiTreeNodeFlags_FramePadding))
	{
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MATERIAL_INSPECTOR_SELECTOR"))
			{
				uint payload_n = *(uint*)payload->Data;
				SetMaterialRes(payload_n);
			}
			ImGui::EndDragDropTarget();
		}
	}

}


void ComponentTrail::SetMaterialRes(uint materialUuid)
{
	if (materialRes > 0)
		App->res->SetAsUnused(materialRes);

	if (materialUuid > 0)
		App->res->SetAsUsed(materialUuid);

	materialRes = materialUuid;
}
