#include "ComponentPlaneCollider.h"

#include "Application.h"
#include "ModulePhysics.h"
#include "GameObject.h"
#include "ComponentTransform.h"
#include "ModuleLayers.h"

#include "ComponentRigidActor.h"

#include "imgui\imgui.h"

// Only for static actors

ComponentPlaneCollider::ComponentPlaneCollider(GameObject* parent, bool include) : ComponentCollider(parent, ComponentTypes::PlaneColliderComponent, include)
{
	if (include)
		EncloseGeometry();

	colliderType = ColliderTypes::PlaneCollider;

	// -----

	SetIsTrigger(isTrigger);
	SetParticipateInContactTests(participateInContactTests);
	SetParticipateInSceneQueries(participateInSceneQueries);
	SetFiltering(filterGroup, filterMask);

	// -----

	SetNormal(normal);
	SetDistance(distance);

	SetCenter(center);
}

ComponentPlaneCollider::ComponentPlaneCollider(const ComponentPlaneCollider& componentPlaneCollider, GameObject* parent, bool include) : ComponentCollider(componentPlaneCollider, parent, ComponentTypes::PlaneColliderComponent, include)
{
	if (include)
		EncloseGeometry();

	colliderType = componentPlaneCollider.colliderType;

	// -----

	SetIsTrigger(componentPlaneCollider.isTrigger);
	SetParticipateInContactTests(componentPlaneCollider.participateInContactTests);
	SetParticipateInSceneQueries(componentPlaneCollider.participateInSceneQueries);
	SetFiltering(componentPlaneCollider.filterGroup, componentPlaneCollider.filterMask);

	// -----

	SetNormal(componentPlaneCollider.normal);
	SetDistance(componentPlaneCollider.distance);

	SetCenter(componentPlaneCollider.center);
}

ComponentPlaneCollider::~ComponentPlaneCollider() {}

// ----------------------------------------------------------------------------------------------------

void ComponentPlaneCollider::OnUniqueEditor()
{
#ifndef GAMEMODE
	if (ImGui::CollapsingHeader("Plane Collider", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ComponentCollider::OnUniqueEditor();

		ImGui::Text("Normal"); ImGui::PushItemWidth(50.0f);
		if (ImGui::DragFloat("##NormalX", &normal.x, 0.01f, -FLT_MAX, FLT_MAX, "%.2f", 1.0f))
			SetNormal(normal);
		ImGui::PopItemWidth();
		ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
		if (ImGui::DragFloat("##NormalY", &normal.y, 0.01f, -FLT_MAX, FLT_MAX, "%.2f", 1.0f))
			SetNormal(normal);
		ImGui::PopItemWidth();
		ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
		if (ImGui::DragFloat("##NormalZ", &normal.z, 0.01f, -FLT_MAX, FLT_MAX, "%.2f", 1.0f))
			SetNormal(normal);
		ImGui::PopItemWidth();

		ImGui::AlignTextToFramePadding();
		ImGui::Text("Distance"); ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
		if (ImGui::DragFloat("##PlaneDistance", &distance, 0.01f, -FLT_MAX, FLT_MAX, "%.2f", 1.0f))
			SetDistance(distance);
		ImGui::PopItemWidth();
	}
#endif
}

uint ComponentPlaneCollider::GetInternalSerializationBytes()
{
	return ComponentCollider::GetInternalSerializationBytes() + 
		sizeof(math::float3) +
		sizeof(float);
}

void ComponentPlaneCollider::OnInternalSave(char*& cursor)
{
	ComponentCollider::OnInternalSave(cursor);

	size_t bytes = sizeof(math::float3);
	memcpy(cursor, &normal, bytes);
	cursor += bytes;

	bytes = sizeof(float);
	memcpy(cursor, &distance, bytes);
	cursor += bytes;
}

void ComponentPlaneCollider::OnInternalLoad(char*& cursor)
{
	ComponentCollider::OnInternalLoad(cursor);

	size_t bytes = sizeof(math::float3);
	memcpy(&normal, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(float);
	memcpy(&distance, cursor, bytes);
	cursor += bytes;

	// -----

	EncloseGeometry();
}

// ----------------------------------------------------------------------------------------------------

void ComponentPlaneCollider::EncloseGeometry()
{
	RecalculateShape();
}

void ComponentPlaneCollider::RecalculateShape()
{
	ClearShape();

	physx::PxPlaneGeometry gPlaneGeometry;
	gShape = App->physics->CreateShape(gPlaneGeometry, *gMaterial);
	assert(gShape != nullptr);

	physx::PxTransform relativePose = physx::PxTransformFromPlaneEquation(physx::PxPlane(normal.x, normal.y, normal.z, distance));
	gShape->setLocalPose(relativePose);

	Layer* layer = App->layers->GetLayer(parent->GetLayer());
	SetFiltering(layer->GetFilterGroup(), layer->GetFilterMask());

	// -----

	if (parent->cmp_rigidActor != nullptr)
		parent->cmp_rigidActor->UpdateShape(gShape);
}

// ----------------------------------------------------------------------------------------------------

void ComponentPlaneCollider::SetNormal(const math::float3& normal)
{
	assert(normal.IsFinite());

	if (normal.IsZero())
	{
		CONSOLE_LOG(LogTypes::Warning, "The plane transform cannot be updated since the normal is zero");
		return;
	}

	this->normal = normal;

	physx::PxTransform relativePose = physx::PxTransformFromPlaneEquation(physx::PxPlane(normal.x, normal.y, normal.z, distance));
	if (gShape != nullptr)
		gShape->setLocalPose(relativePose);
}

void ComponentPlaneCollider::SetDistance(float distance)
{
	this->distance = distance;

	if (normal.IsZero())
	{
		CONSOLE_LOG(LogTypes::Warning, "The plane transform cannot be updated since the normal is zero");
		return;
	}

	physx::PxTransform relativePose = physx::PxTransformFromPlaneEquation(physx::PxPlane(normal.x, normal.y, normal.z, distance));
	if (gShape != nullptr)
		gShape->setLocalPose(relativePose);
}

// ----------------------------------------------------------------------------------------------------

physx::PxPlaneGeometry ComponentPlaneCollider::GetPlaneGeometry() const
{
	physx::PxPlaneGeometry planeGeometry;
	if (gShape != nullptr)
		gShape->getPlaneGeometry(planeGeometry);
	return planeGeometry;
}