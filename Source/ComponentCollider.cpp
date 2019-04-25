#include "ComponentCollider.h"

#include "Application.h"
#include "ModulePhysics.h"
#include "GameObject.h"
#include "EventSystem.h"

#include "ComponentRigidActor.h"
#include "ComponentScript.h"

#include "imgui\imgui.h"

ComponentCollider::ComponentCollider(GameObject* parent, ComponentTypes componentColliderType, bool include) : Component(parent, componentColliderType)
{
	gMaterial = App->physics->GetDefaultMaterial();
	assert(gMaterial != nullptr);

	if (include)
		App->physics->AddColliderComponent(this);
}

ComponentCollider::ComponentCollider(const ComponentCollider& componentCollider, GameObject* parent, ComponentTypes componentColliderType, bool include) : Component(parent, componentColliderType)
{
	gMaterial = App->physics->GetDefaultMaterial();
	assert(gMaterial != nullptr);

	if (include)
		App->physics->AddColliderComponent(this);
}

ComponentCollider::~ComponentCollider()
{
	if (parent->cmp_rigidActor != nullptr)
		parent->cmp_rigidActor->UpdateShape(nullptr);
	ClearShape();

	gMaterial = nullptr;

	App->physics->EraseColliderComponent(this);
	parent->cmp_collider = nullptr;
}

// ----------------------------------------------------------------------------------------------------

void ComponentCollider::OnUniqueEditor()
{
#ifndef GAMEMODE
	if (ImGui::Checkbox("Is Trigger", &isTrigger))
		SetIsTrigger(isTrigger);
	if (ImGui::Checkbox("Contact Tests", &participateInContactTests))
		SetParticipateInContactTests(participateInContactTests);
	if (ImGui::Checkbox("Scene Queries", &participateInSceneQueries))
		SetParticipateInSceneQueries(participateInSceneQueries);

	// TODO: gMaterial (drag and drop)

	if (componentType != ComponentTypes::PlaneColliderComponent)
	{
		ImGui::Text("Center"); ImGui::PushItemWidth(50.0f);
		if (ImGui::DragFloat("##CenterX", &center.x, 0.01f, -FLT_MAX, FLT_MAX, "%.2f", 1.0f))
			SetCenter(center);
		ImGui::PopItemWidth();
		ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
		if (ImGui::DragFloat("##CenterY", &center.y, 0.01f, -FLT_MAX, FLT_MAX, "%.2f", 1.0f))
			SetCenter(center);
		ImGui::PopItemWidth();
		ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
		if (ImGui::DragFloat("##CenterZ", &center.z, 0.01f, -FLT_MAX, FLT_MAX, "%.2f", 1.0f))
			SetCenter(center);
		ImGui::PopItemWidth();
	}

	if (ImGui::Button("Enclose geometry"))
		EncloseGeometry();
#endif
}

void ComponentCollider::OnEnable()
{
	if (gShape != nullptr)
	{
		gShape->setFlag(physx::PxShapeFlag::Enum::eSCENE_QUERY_SHAPE, true);
		if (isTrigger)
		{
			gShape->setFlag(physx::PxShapeFlag::Enum::eTRIGGER_SHAPE, true);
			gShape->setFlag(physx::PxShapeFlag::Enum::eSIMULATION_SHAPE, false);
		}
		else
		{
			gShape->setFlag(physx::PxShapeFlag::Enum::eTRIGGER_SHAPE, false);
			gShape->setFlag(physx::PxShapeFlag::Enum::eSIMULATION_SHAPE, true);
		}
	}
}

void ComponentCollider::OnDisable()
{
	if (gShape != nullptr)
	{
		gShape->setFlag(physx::PxShapeFlag::Enum::eTRIGGER_SHAPE, false);
		gShape->setFlag(physx::PxShapeFlag::Enum::eSCENE_QUERY_SHAPE, false);
		gShape->setFlag(physx::PxShapeFlag::Enum::eSIMULATION_SHAPE, false);
	}
}

// ----------------------------------------------------------------------------------------------------

void ComponentCollider::Update()
{
	if (isTrigger)
	{
		if (triggerEnter && !triggerExit)
			OnTriggerStay(collision);
	}
}

uint ComponentCollider::GetInternalSerializationBytes()
{
	return sizeof(bool) +
		sizeof(bool) +
		sizeof(bool) +
		sizeof(math::float3) +
		sizeof(ColliderTypes) +
		sizeof(uint) +
		sizeof(uint);
}

void ComponentCollider::OnInternalSave(char*& cursor)
{
	size_t bytes = sizeof(bool);
	memcpy(cursor, &isTrigger, bytes);
	cursor += bytes;

	bytes = sizeof(bool);
	memcpy(cursor, &participateInContactTests, bytes);
	cursor += bytes;

	bytes = sizeof(bool);
	memcpy(cursor, &participateInSceneQueries, bytes);
	cursor += bytes;

	bytes = sizeof(math::float3);
	memcpy(cursor, &center, bytes);
	cursor += bytes;

	bytes = sizeof(ColliderTypes);
	memcpy(cursor, &colliderType, bytes);
	cursor += bytes;

	bytes = sizeof(uint);
	memcpy(cursor, &filterGroup, bytes);
	cursor += bytes;

	bytes = sizeof(uint);
	memcpy(cursor, &filterMask, bytes);
	cursor += bytes;
}

void ComponentCollider::OnInternalLoad(char*& cursor)
{
	size_t bytes = sizeof(bool);
	memcpy(&isTrigger, cursor, bytes);
	SetIsTrigger(isTrigger);
	cursor += bytes;

	bytes = sizeof(bool);
	memcpy(&participateInContactTests, cursor, bytes);
	SetParticipateInContactTests(participateInContactTests);
	cursor += bytes;

	bytes = sizeof(bool);
	memcpy(&participateInSceneQueries, cursor, bytes);
	SetParticipateInSceneQueries(participateInSceneQueries);
	cursor += bytes;

	bytes = sizeof(math::float3);
	memcpy(&center, cursor, bytes);
	SetCenter(center);
	cursor += bytes;

	bytes = sizeof(ColliderTypes);
	memcpy(&colliderType, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(uint);
	memcpy(&filterGroup, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(uint);
	memcpy(&filterMask, cursor, bytes);
	cursor += bytes;

	SetFiltering(filterGroup, filterMask);
}

// ----------------------------------------------------------------------------------------------------

void ComponentCollider::ClearShape()
{
	if (gShape != nullptr)
		gShape->release();
	gShape = nullptr;
}

void ComponentCollider::SetFiltering(physx::PxU32 filterGroup, physx::PxU32 filterMask)
{
	this->filterGroup = filterGroup;
	this->filterMask = filterMask;

	physx::PxFilterData filterData;
	filterData.word0 = filterGroup; // word 0 = own ID
	if (gShape != nullptr)
		gShape->setQueryFilterData(filterData);

	filterData.word1 = filterMask; // word 1 = ID mask to filter pairs that trigger a contact callback
	if (gShape != nullptr)
		gShape->setSimulationFilterData(filterData);
}

// ----------------------------------------------------------------------------------------------------

void ComponentCollider::SetIsTrigger(bool isTrigger)
{
	this->isTrigger = isTrigger;
	this->participateInContactTests = !isTrigger;

	if (IsTreeActive())
	{
		if (gShape != nullptr)
		{
			gShape->setFlag(physx::PxShapeFlag::Enum::eTRIGGER_SHAPE, isTrigger);
			gShape->setFlag(physx::PxShapeFlag::Enum::eSIMULATION_SHAPE, participateInContactTests);
		}
	}
}

void ComponentCollider::SetParticipateInContactTests(bool participateInContactTests)
{
	this->participateInContactTests = participateInContactTests;
	this->isTrigger = !participateInContactTests;

	if (IsTreeActive())
	{		
		if (gShape != nullptr)
		{
			gShape->setFlag(physx::PxShapeFlag::Enum::eTRIGGER_SHAPE, isTrigger);
			gShape->setFlag(physx::PxShapeFlag::Enum::eSIMULATION_SHAPE, participateInContactTests);
		}	
	}
}

void ComponentCollider::SetParticipateInSceneQueries(bool participateInSceneQueries)
{
	this->participateInSceneQueries = participateInSceneQueries;

	if (gShape != nullptr && IsTreeActive())
		gShape->setFlag(physx::PxShapeFlag::Enum::eSCENE_QUERY_SHAPE, participateInSceneQueries);
}

void ComponentCollider::SetCenter(const math::float3& center)
{
	assert(center.IsFinite());
	this->center = center;
	physx::PxTransform relativePose(physx::PxVec3(center.x, center.y, center.z));
	if (gShape != nullptr)
		gShape->setLocalPose(relativePose);
}

// ----------------------------------------------------------------------------------------------------

physx::PxShape* ComponentCollider::GetShape() const
{
	return gShape;
}

ColliderTypes ComponentCollider::GetColliderType() const
{
	return colliderType;
}

// ----------------------------------------------------------------------------------------------------

void ComponentCollider::OnCollisionEnter(Collision& collision)
{
	if (collision.GetGameObject() != nullptr)
	{
		//CONSOLE_LOG(LogTypes::Normal, "OnCollisionEnter with '%s'", collision.GetGameObject()->GetName());

		std::vector<Component*> scripts = parent->GetComponents(ComponentTypes::ScriptComponent);
		for (int i = 0; i < scripts.size(); ++i)
		{
			ComponentScript* script = (ComponentScript*)scripts[i];
			script->OnCollisionEnter(collision);
		}
	}		
}

void ComponentCollider::OnCollisionStay(Collision& collision)
{
	if (collision.GetGameObject() != nullptr)
	{
		//CONSOLE_LOG(LogTypes::Normal, "OnCollisionStay with '%s'", collision.GetGameObject()->GetName());

		std::vector<Component*> scripts = parent->GetComponents(ComponentTypes::ScriptComponent);
		for (int i = 0; i < scripts.size(); ++i)
		{
			ComponentScript* script = (ComponentScript*)scripts[i];
			script->OnCollisionStay(collision);
		}
	}		
}

void ComponentCollider::OnCollisionExit(Collision& collision)
{
	if (collision.GetGameObject() != nullptr)
	{
		//CONSOLE_LOG(LogTypes::Normal, "OnCollisionExit with '%s'", collision.GetGameObject()->GetName());

		std::vector<Component*> scripts = parent->GetComponents(ComponentTypes::ScriptComponent);
		for (int i = 0; i < scripts.size(); ++i)
		{
			ComponentScript* script = (ComponentScript*)scripts[i];
			script->OnCollisionExit(collision);
		}
	}
}

void ComponentCollider::OnTriggerEnter(Collision& collision)
{
	if (collision.GetGameObject() != nullptr)
	{
		//CONSOLE_LOG(LogTypes::Normal, "OnTriggerEnter with '%s'", collision.GetGameObject()->GetName());

		std::vector<Component*> scripts = parent->GetComponents(ComponentTypes::ScriptComponent);
		for (int i = 0; i < scripts.size(); ++i)
		{
			ComponentScript* script = (ComponentScript*)scripts[i];
			script->OnTriggerEnter(collision);
		}
	}

	triggerEnter = true;
	triggerExit = false;
	this->collision = collision;
}

void ComponentCollider::OnTriggerStay(Collision& collision)
{
	if (collision.GetGameObject() != nullptr)
	{
		//CONSOLE_LOG(LogTypes::Normal, "OnTriggerStay with '%s'", collision.GetGameObject()->GetName());

		std::vector<Component*> scripts = parent->GetComponents(ComponentTypes::ScriptComponent);
		for (int i = 0; i < scripts.size(); ++i)
		{
			ComponentScript* script = (ComponentScript*)scripts[i];
			script->OnTriggerStay(collision);
		}
	}
}

void ComponentCollider::OnTriggerExit(Collision& collision)
{
	if (collision.GetGameObject() != nullptr)
	{
		//CONSOLE_LOG(LogTypes::Normal, "OnTriggerExit with '%s'", collision.GetGameObject()->GetName());

		std::vector<Component*> scripts = parent->GetComponents(ComponentTypes::ScriptComponent);
		for (int i = 0; i < scripts.size(); ++i)
		{
			ComponentScript* script = (ComponentScript*)scripts[i];
			script->OnTriggerExit(collision);
		}
	}

	triggerExit = true;
	triggerEnter = false;
	this->collision = collision;
}

// ----------------------------------------------------------------------------------------------------

/// Transformed box, sphere, capsule or convex geometry
float ComponentCollider::GetPointToGeometryObjectDistance(const math::float3& point, const physx::PxGeometry& geometry, const physx::PxTransform& pose)
{
	assert(point.IsFinite() && pose.isFinite());
	return physx::PxGeometryQuery::pointDistance(physx::PxVec3(point.x, point.y, point.z), geometry, pose);
}

/// Transformed box, sphere, capsule or convex geometry
float ComponentCollider::GetPointToGeometryObjectDistance(const math::float3& point, const physx::PxGeometry& geometry, const physx::PxTransform& pose, math::float3& closestPoint)
{
	assert(point.IsFinite() && pose.isFinite());
	physx::PxVec3 gClosestPoint;
	float distance = physx::PxGeometryQuery::pointDistance(physx::PxVec3(point.x, point.y, point.z), geometry, pose, &gClosestPoint);
	if (gClosestPoint.isFinite())
		closestPoint = math::float3(gClosestPoint.x, gClosestPoint.y, gClosestPoint.z);
	else
		closestPoint = math::float3::zero;

	return distance;
}

physx::PxBounds3 ComponentCollider::GetGeometryObjectAABB(const physx::PxGeometry& geometry, const physx::PxTransform& pose, float inflation)
{
	physx::PxBounds3 gBounds = physx::PxGeometryQuery::getWorldBounds(geometry, pose, inflation);
	if (!gBounds.isValid() || !gBounds.isFinite())
		gBounds = physx::PxBounds3::empty();

	return gBounds;
}