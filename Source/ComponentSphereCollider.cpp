#include "ComponentSphereCollider.h"

#include "Application.h"
#include "ModulePhysics.h"
#include "GameObject.h"
#include "ComponentTransform.h"
#include "ModuleLayers.h"

#include "ComponentRigidActor.h"

#include "imgui\imgui.h"

#include "MathGeoLib\include\Math\float4x4.h"

ComponentSphereCollider::ComponentSphereCollider(GameObject* parent) : ComponentCollider(parent, ComponentTypes::SphereColliderComponent)
{
	EncloseGeometry();

	colliderType = ColliderTypes::SphereCollider;

	// -----

	physx::PxShapeFlags shapeFlags = gShape->getFlags();
	isTrigger = shapeFlags & physx::PxShapeFlag::Enum::eTRIGGER_SHAPE && !(shapeFlags & physx::PxShapeFlag::eSIMULATION_SHAPE);
	participateInContactTests = shapeFlags & physx::PxShapeFlag::Enum::eSIMULATION_SHAPE;
	participateInSceneQueries = shapeFlags & physx::PxShapeFlag::Enum::eSCENE_QUERY_SHAPE;
}

ComponentSphereCollider::ComponentSphereCollider(const ComponentSphereCollider& componentSphereCollider, GameObject* parent, bool include) : ComponentCollider(componentSphereCollider, parent, ComponentTypes::SphereColliderComponent, include)
{
	EncloseGeometry();

	colliderType = componentSphereCollider.colliderType;

	SetIsTrigger(componentSphereCollider.isTrigger);
	SetParticipateInContactTests(componentSphereCollider.participateInContactTests);
	SetParticipateInSceneQueries(componentSphereCollider.participateInSceneQueries);
	SetFiltering(componentSphereCollider.filterGroup, componentSphereCollider.filterMask);

	// -----

	SetRadius(componentSphereCollider.radius);

	SetCenter(componentSphereCollider.center);
}

ComponentSphereCollider::~ComponentSphereCollider() {}

// ----------------------------------------------------------------------------------------------------

void ComponentSphereCollider::OnUniqueEditor()
{
#ifndef GAMEMODE
	if (ImGui::CollapsingHeader("Sphere Collider", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text("Sphere Collider");
		ImGui::Spacing();

		ComponentCollider::OnUniqueEditor();

		ImGui::AlignTextToFramePadding();
		ImGui::Text("Radius"); ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
		if (ImGui::DragFloat("##SphereRadius", &radius, 0.01f, 0.01f, FLT_MAX, "%.2f", 1.0f))
			SetRadius(radius);
		ImGui::PopItemWidth();
	}
#endif
}

uint ComponentSphereCollider::GetInternalSerializationBytes()
{
	return ComponentCollider::GetInternalSerializationBytes() + 
		sizeof(float);
}

void ComponentSphereCollider::OnInternalSave(char*& cursor)
{
	ComponentCollider::OnInternalSave(cursor);

	size_t bytes = sizeof(float);
	memcpy(cursor, &radius, bytes);
	cursor += bytes;
}

void ComponentSphereCollider::OnInternalLoad(char*& cursor)
{
	ComponentCollider::OnInternalLoad(cursor);

	size_t bytes = sizeof(float);
	memcpy(&radius, cursor, bytes);
	cursor += bytes;

	// -----

	EncloseGeometry();
}

// ----------------------------------------------------------------------------------------------------

void ComponentSphereCollider::EncloseGeometry()
{
	math::float4x4 globalMatrix = parent->transform->GetGlobalMatrix();
	math::AABB boundingBox = parent->boundingBox;

	if (globalMatrix.IsFinite() && parent->boundingBox.IsFinite())
	{
		math::float3 pos = math::float3::zero;
		math::Quat rot = math::Quat::identity;
		math::float3 scale = math::float3::one;
		globalMatrix.Decompose(pos, rot, scale);

		center = parent->boundingBox.CenterPoint() - pos;
		math::float3 halfSize = parent->boundingBox.HalfSize();

		radius = halfSize.Length();
	}

	RecalculateShape();
}

void ComponentSphereCollider::RecalculateShape()
{
	ClearShape();

	physx::PxSphereGeometry gSphereGeometry(radius);
	gShape = App->physics->CreateShape(gSphereGeometry, *gMaterial);
	assert(gShape != nullptr);

	physx::PxTransform relativePose(physx::PxVec3(center.x, center.y, center.z));
	gShape->setLocalPose(relativePose);

	Layer* layer = App->layers->GetLayer(parent->GetLayer());
	SetFiltering(layer->GetFilterGroup(), layer->GetFilterMask());

	// -----

	if (parent->cmp_rigidActor != nullptr)
		parent->cmp_rigidActor->UpdateShape(gShape);
}

// ----------------------------------------------------------------------------------------------------

void ComponentSphereCollider::SetRadius(float radius)
{
	this->radius = radius;
	gShape->setGeometry(physx::PxSphereGeometry(radius));
}

// ----------------------------------------------------------------------------------------------------

physx::PxSphereGeometry ComponentSphereCollider::GetSphereGeometry() const
{
	physx::PxSphereGeometry sphereGeometry;
	gShape->getSphereGeometry(sphereGeometry);
	return sphereGeometry;
}