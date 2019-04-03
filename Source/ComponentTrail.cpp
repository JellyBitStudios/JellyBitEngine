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

	if (timer.Read() > 500 && create)
	{
		TrailNode* node = new TrailNode();

		node->originHigh = parent->boundingBox.FaceCenterPoint(5);
		node->originLow = parent->boundingBox.FaceCenterPoint(4);

		node->rotaion = parent->transform->GetRotation();

		if (!test.empty())
		{
			//math::Plane plane = math::Plane(test.back()->destHigh, node->originHigh, node->originLow);
			//node->direction = plane.normal;
		}

		test.push_back(node);

		timer.Start();
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
