#include "ComponentTransform.h"

#include "Application.h"
#include "ModuleTimeManager.h"
#include "ModuleCameraEditor.h"
#include "ModuleScene.h"
#include "ModuleInput.h"

#include "GameObject.h"
#include "ComponentCamera.h"
#include "ComponentProjector.h"
#include "ComponentRigidActor.h"
#include "ComponentCanvas.h"

#include "imgui\imgui.h"
#include "imgui\imgui_internal.h"

#include "Brofiler/Brofiler.h"
#include <list>

#include "MathGeoLib\include\Geometry\OBB.h"

ComponentTransform::ComponentTransform(GameObject* parent) : Component(parent, ComponentTypes::TransformComponent) 
{
	UpdateGlobal();
}

ComponentTransform::ComponentTransform(const ComponentTransform& componentTransform, GameObject* parent) : Component(parent, ComponentTypes::TransformComponent)
{
	position = componentTransform.position;
	rotation = componentTransform.rotation;
	scale = componentTransform.scale;

	UpdateGlobal();
}

ComponentTransform::~ComponentTransform()
{
	if (parent)
	{
		parent->transform = nullptr;
		parent->originalBoundingBox.SetNegativeInfinity();
		parent->boundingBox.SetNegativeInfinity();
	}
}

void ComponentTransform::Update() {}

// Redefined cause there is no way that a transform component could be erased or moved.
void ComponentTransform::OnEditor()
{
	if(!parent->cmp_canvas)
		OnUniqueEditor();
}

void ComponentTransform::OnUniqueEditor()
{
#ifndef GAMEMODE
	if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (parent)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			bool seenLastFrame = parent->seenLastFrame;
			ImGui::Checkbox("Seen last frame", &seenLastFrame);
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, false);
		}

		math::float3 lastPosition = position;
		math::Quat lastRotation = rotation;
		math::float3 lastScale = scale;

		if (ImGui::Button("Reset"))
		{
			position = math::float3::zero;
			rotation = math::Quat::identity;
			scale = math::float3::one;

			UpdateGlobal();
		}

		math::float4x4 matrix = GetMatrix();

		ImGui::Text("Position");
		if (ImGui::DragFloat3("##Pos", &position[0], 0.01f, 0.0f, 0.0f, "%.3f"))
		{
			SavePrevTransform(matrix);
		
			UpdateGlobal();
		}

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

			UpdateGlobal();
		}

		ImGui::Text("Scale");
		if (ImGui::DragFloat3("##Scale", &scale[0], 0.01f, 0.0f, 0.0f, "%.3f"))
		{
			SavePrevTransform(matrix);

			UpdateGlobal();
		}
		if (parent && (!position.Equals(lastPosition) || !rotation.Equals(lastRotation) || !scale.Equals(lastScale)))
		{
			// Transform updated: if the game object has a camera, update its frustum
			if (parent->cmp_camera != nullptr)
				parent->cmp_camera->UpdateTransform();

			// Transform updated: if the game object has a projector, update its frustum
			if (parent->cmp_projector != nullptr)
				parent->cmp_projector->UpdateTransform();

			// Transform updated: if the game object has a canvas, Update the rectTransforms
			if (parent->cmp_canvas != nullptr)
				parent->cmp_canvas->TransformUpdated();

#ifndef GAMEMODE
			// Transform updated: if the game object is selected, update the camera reference
			if (parent == App->scene->selectedObject.Get())
				App->camera->SetReference(position);
#endif

			//// Transform updated: recalculate bounding boxes -> // Bounding boxes are now automatically recalculated from ComponentTransform::UpdateGlobal()
			//System_Event newEvent;
			//newEvent.goEvent.gameObject = parent;
			//newEvent.type = System_Event_Type::RecalculateBBoxes;
			//App->PushSystemEvent(newEvent);

			if (parent->IsStatic())
			{
				// Bounding box changed: recreate quadtree
				System_Event newEvent;
				newEvent.type = System_Event_Type::RecreateQuadtree;
				App->PushSystemEvent(newEvent);
			}
		}
	}
	if ((App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_UP || App->input->GetKey(SDL_SCANCODE_KP_ENTER) == KEY_DOWN
		|| App->input->GetKey(SDL_SCANCODE_RETURN) == KEY_DOWN) && !dragTransform)
		dragTransform = true;
#endif // !GAMEMODE
}

void ComponentTransform::SavePrevTransform(const math::float4x4 & prevTransformMat)
{
#ifndef GAMEMODE
	if (dragTransform)
	{
		App->scene->SaveLastTransform(prevTransformMat);
		dragTransform = false;
	}
#endif // !GAMEMODE
}

math::float4x4& ComponentTransform::GetMatrix() const
{
	math::float4x4 matrix = math::float4x4::FromTRS(position, rotation, scale);
	return matrix;
}

void ComponentTransform::SetMatrixFromGlobal(math::float4x4& globalMatrix)
{
	if (parent)
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

		// Transform updated: if the game object has a camera, update its frustum
		if (parent->cmp_camera != nullptr)
			parent->cmp_camera->UpdateTransform();

		// Transform updated: if the game object has a projector, update its frustum
		if (parent->cmp_projector != nullptr)
			parent->cmp_projector->UpdateTransform();

		// Transform updated: if the game object has a canvas, Update the rectTransforms
		if (parent->cmp_canvas != nullptr)
			parent->cmp_canvas->TransformUpdated();

#ifndef GAMEMODE
		// Transform updated: if the game object is selected, update the camera reference
		if (parent == App->scene->selectedObject.Get())
			App->camera->SetReference(position);
#endif

		//// Transform updated: recalculate bounding boxes -> // Bounding boxes are now automatically recalculated from ComponentTransform::UpdateGlobal()
		//System_Event newEvent;
		//newEvent.goEvent.gameObject = parent;
		//newEvent.type = System_Event_Type::RecalculateBBoxes;
		//App->PushSystemEvent(newEvent);

		if (parent->IsStatic())
		{
			// Bounding box changed: recreate quadtree
			System_Event newEvent;
			newEvent.type = System_Event_Type::RecreateQuadtree;
			App->PushSystemEvent(newEvent);
		}
	}
	else
		globalMatrix.Decompose(position, rotation, scale);

	UpdateGlobal();
}

math::float4x4 ComponentTransform::GetGlobalMatrix() const
{
	return globalMatrix;
}

void ComponentTransform::UpdateGlobal()
{
	math::float4x4 local = GetMatrix();

	if (parent)
	{
		GameObject* goParent = parent->GetParent();
		if (goParent && goParent->transform)
 			globalMatrix = goParent->transform->GetGlobalMatrix().Mul(local);
		else
			globalMatrix = local;

		if (parent->cmp_rigidActor != nullptr)
			parent->cmp_rigidActor->UpdateTransform(globalMatrix);

		for (std::vector<GameObject*>::iterator childs = parent->children.begin(); childs != parent->children.end(); ++childs)
		{
			if ((*childs)->transform)
				(*childs)->transform->UpdateGlobal();
		}

		// Transform is updated, we have to recalculate the bounding box.
		// As we are already in a recursive method, all bouding boxes will be updated by recalculating the current.
		// Doing this we avoid calling another recusive method and improve performance.
		parent->RecalculateBoundingBox();
	}
	else
		globalMatrix = local;
}


void ComponentTransform::SetPosition(math::float3 newPos)
{
	this->position = newPos;

	UpdateGlobal();
}

void ComponentTransform::SetPosition(const float newPos[3])
{
	SetPosition(math::float3(newPos[0], newPos[1], newPos[2]));
}

void ComponentTransform::SetRotation(math::Quat newRot)
{
	this->rotation = newRot;

	UpdateGlobal();
}

void ComponentTransform::SetScale(math::float3 newScale)
{
	this->scale = newScale;

	UpdateGlobal();
}

void ComponentTransform::Move(math::float3 distance)
{
	SetPosition(this->position.Add(distance));
}

void ComponentTransform::Move(const float distance[3])
{
	Move(math::float3(distance[0], distance[1], distance[2]));
}

void ComponentTransform::Rotate(math::Quat rotation)
{
	SetRotation(rotation.Mul(this->rotation).Normalized());
}

void ComponentTransform::Scale(math::float3 scale)
{
	SetScale(this->scale.Mul(scale));
}
void ComponentTransform::Scale(float scale)
{
	SetScale(this->scale.Mul(scale));
}

math::float3 ComponentTransform::GetPosition() const
{
	return position;
}

math::Quat ComponentTransform::GetRotation() const
{
	return rotation;
}

math::float3 ComponentTransform::GetScale() const
{
	return scale;
}

uint ComponentTransform::GetInternalSerializationBytes()
{
	// position + scale + rotation
	return sizeof(math::float3) * 2 + sizeof(math::Quat);
}

void ComponentTransform::OnInternalSave(char*& cursor)
{
	size_t bytes = sizeof(math::float3);
	memcpy(cursor, &position, bytes);
	cursor += bytes;

	bytes = sizeof(math::Quat);
	memcpy(cursor, &rotation, bytes);
	cursor += bytes;

	bytes = sizeof(math::float3);
	memcpy(cursor, &scale, bytes);
	cursor += bytes;
}

void ComponentTransform::OnInternalLoad(char*& cursor)
{
	size_t bytes = sizeof(math::float3);
	memcpy(&position, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(math::Quat);
	memcpy(&rotation, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(math::float3);
	memcpy(&scale, cursor, bytes);
	cursor += bytes;

	UpdateGlobal();
}