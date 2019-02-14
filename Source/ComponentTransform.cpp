#include "ComponentTransform.h"

#include "Application.h"
#include "ModuleTimeManager.h"
#include "ModuleCameraEditor.h"
#include "ModuleScene.h"
#include "ModuleInput.h"
#include "GameObject.h"
#include "ComponentCamera.h"
#include "ComponentRigidActor.h"

#include "imgui\imgui.h"
#include "imgui\imgui_internal.h"

#include <list>

ComponentTransform::ComponentTransform(GameObject* parent) : Component(parent, ComponentTypes::TransformComponent) {}

ComponentTransform::ComponentTransform(const ComponentTransform& componentTransform) : Component(componentTransform.parent, ComponentTypes::TransformComponent)
{
	position = componentTransform.position;
	rotation = componentTransform.rotation;
	scale = componentTransform.scale;
}

ComponentTransform::~ComponentTransform()
{
	parent->transform = nullptr;
}

void ComponentTransform::Update() {}

// Redefined cause there is no way that a transform component could be erased or moved.
void ComponentTransform::OnEditor()
{
	OnUniqueEditor();
}

void ComponentTransform::OnUniqueEditor()
{
#ifndef GAMEMODE
	ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
	bool seenLastFrame = parent->GetSeenLastFrame();
	ImGui::Checkbox("Seen last frame", &seenLastFrame);
	ImGui::PushItemFlag(ImGuiItemFlags_Disabled, false);

	math::float3 lastPosition = position;
	math::Quat lastRotation = rotation;
	math::float3 lastScale = scale;

	if (ImGui::Button("Reset"))
	{
		position = math::float3::zero;
		rotation = math::Quat::identity;
		scale = math::float3::one;
	}

	math::float4x4 matrix = parent->transform->GetMatrix();
	ImGui::Text("Position");
	if (ImGui::DragFloat3("##Pos", &position[0], 0.01f, 0.0f, 0.0f, "%.3f"))
		SavePrevTransform(matrix);

	ImGui::Text("Rotation");
	math::float3 axis;
	float angle;
	rotation.ToAxisAngle(axis, angle);
	axis *= angle;
	axis *= RADTODEG;
	if (ImGui::DragFloat3("##Rot", &axis[0], 0.1f, 0.0f, 0.0f, "%.3f"))
	{
		SavePrevTransform(matrix);
		axis *= DEGTORAD;
		rotation.SetFromAxisAngle(axis.Normalized(), axis.Length());
	}

	ImGui::Text("Scale");
	if (ImGui::DragFloat3("##Scale", &scale[0], 0.01f, 0.0f, 0.0f, "%.3f"))
		SavePrevTransform(matrix);

	if (!position.Equals(lastPosition) || !rotation.Equals(lastRotation) || !scale.Equals(lastScale))
	{
		// Transform updated: if the game object has a rigid body, update its transform
		if (parent->rigidActor != nullptr)
		{
			math::float4x4 globalMatrix = GetGlobalMatrix();
			parent->rigidActor->UpdateTransform(globalMatrix);
		}

		// Transform updated: if the game object has a camera, update its frustum
		if (parent->camera != nullptr)
			parent->camera->UpdateTransform();

#ifndef GAMEMODE
		// Transform updated: if the game object is selected, update the camera reference
		if (parent == App->scene->selectedObject.Get())
			App->camera->SetReference(position);
#endif

		// Transform updated: recalculate bounding boxes
		System_Event newEvent;
		newEvent.goEvent.gameObject = parent;
		newEvent.type = System_Event_Type::RecalculateBBoxes;
		App->PushSystemEvent(newEvent);

		if (parent->IsStatic())
		{
			// Bounding box changed: recreate quadtree
			System_Event newEvent;
			newEvent.type = System_Event_Type::RecreateQuadtree;
			App->PushSystemEvent(newEvent);
		}

	}
	if ((App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_UP || App->input->GetKey(SDL_SCANCODE_KP_ENTER) == KEY_DOWN
	|| App->input->GetKey(SDL_SCANCODE_RETURN) == KEY_DOWN) && !dragTransform)
		dragTransform = true;

#endif // !GAMEMODE
}

void ComponentTransform::SavePrevTransform(const math::float4x4 &prevTransformMat)
{
	if (dragTransform)
	{
		App->scene->SaveLastTransform(prevTransformMat);
		dragTransform = false;
	}
}


math::float4x4& ComponentTransform::GetMatrix() const
{
	math::float4x4 matrix = math::float4x4::FromTRS(position, rotation, scale);
	return matrix;
}

math::float4x4& ComponentTransform::GetGlobalMatrix() const
{
	std::list<GameObject*> aux_list;

	GameObject* globalParent = this->GetParent()->GetParent();

	while (globalParent->GetParent() != nullptr)
	{
		aux_list.push_back(globalParent);
		globalParent = globalParent->GetParent();
	}
	
	math::float4x4 globalMatrix = math::float4x4::identity;

	for (std::list<GameObject*>::const_reverse_iterator it = aux_list.rbegin(); it != aux_list.rend(); it++)
	{
		math::float4x4 parentMatrix = (*it)->transform->GetMatrix();
		globalMatrix = globalMatrix * parentMatrix;
	}

	math::float4x4 localMatrix = GetMatrix();

	return globalMatrix * localMatrix;
}

void ComponentTransform::SetMatrixFromGlobal(math::float4x4& globalMatrix)
{
	if (parent->GetParent() == App->scene->root) 
		globalMatrix.Decompose(position, rotation, scale);
	else
	{
		math::float4x4 newMatrix = parent->GetParent()->transform->GetGlobalMatrix();
		newMatrix = newMatrix.Inverted();
		newMatrix = newMatrix * globalMatrix;

		newMatrix.Decompose(position, rotation, scale);
	}

	// Transform updated: if the game object has a rigid body, update its transform
	if (parent->rigidActor != nullptr)
	{
		math::float4x4 globalMatrix = GetGlobalMatrix();
		parent->rigidActor->UpdateTransform(globalMatrix);
	}

	// Transform updated: if the game object has a camera, update its frustum
	if (parent->camera != nullptr)
		parent->camera->UpdateTransform();

#ifndef GAMEMODE
	// Transform updated: if the game object is selected, update the camera reference
	if (parent == App->scene->selectedObject.Get())
		App->camera->SetReference(position);
#endif

	// Transform updated: recalculate bounding boxes
	System_Event newEvent;
	newEvent.goEvent.gameObject = parent;
	newEvent.type = System_Event_Type::RecalculateBBoxes;
	App->PushSystemEvent(newEvent);

	if (parent->IsStatic())
	{
		// Bounding box changed: recreate quadtree
		System_Event newEvent;
		newEvent.type = System_Event_Type::RecreateQuadtree;
		App->PushSystemEvent(newEvent);
	}
}

void ComponentTransform::OnInternalSave(JSON_Object* file)
{
	json_object_set_number(file, "PosX", position.x);
	json_object_set_number(file, "PosY", position.y);
	json_object_set_number(file, "PosZ", position.z);
	json_object_set_number(file, "RotX", rotation.x);
	json_object_set_number(file, "RotY", rotation.y);
	json_object_set_number(file, "RotZ", rotation.z);
	json_object_set_number(file, "RotW", rotation.w);
	json_object_set_number(file, "ScaleX", scale.x);
	json_object_set_number(file, "ScaleY", scale.y);
	json_object_set_number(file, "ScaleZ", scale.z);
}

void ComponentTransform::OnLoad(JSON_Object* file)
{
	position.x = json_object_get_number(file, "PosX");
	position.y = json_object_get_number(file, "PosY");
	position.z = json_object_get_number(file, "PosZ");
	rotation.x = json_object_get_number(file, "RotX");
	rotation.y = json_object_get_number(file, "RotY");
	rotation.z = json_object_get_number(file, "RotZ");
	rotation.w = json_object_get_number(file, "RotW");
	scale.x = json_object_get_number(file, "ScaleX");
	scale.y = json_object_get_number(file, "ScaleY");
	scale.z = json_object_get_number(file, "ScaleZ");
}