#include "ComponentRigidStatic.h"

#include "Application.h"
#include "ModulePhysics.h"
#include "GameObject.h"
#include "ComponentTransform.h"

#include "ComponentCollider.h"
#include "ComponentBoxCollider.h"
#include "ComponentSphereCollider.h"
#include "ComponentCapsuleCollider.h"
#include "ComponentPlaneCollider.h"

#include "PhysicsConstants.h"

#include "imgui\imgui.h"

ComponentRigidStatic::ComponentRigidStatic(GameObject* parent, bool include) : ComponentRigidActor(parent, ComponentTypes::RigidStaticComponent, include)
{
	if (include)
	{
		physx::PxShape* gShape = nullptr;

		if (parent->cmp_collider != nullptr && parent->cmp_collider->GetShape())
			gShape = parent->cmp_collider->GetShape();
		else if (parent->boundingBox.IsFinite())
			gShape = App->physics->CreateShape(physx::PxBoxGeometry(parent->boundingBox.HalfSize().x, parent->boundingBox.HalfSize().y, parent->boundingBox.HalfSize().z), *App->physics->GetDefaultMaterial());
		else
			gShape = App->physics->CreateShape(physx::PxBoxGeometry(PhysicsConstants::GEOMETRY_HALF_SIZE, PhysicsConstants::GEOMETRY_HALF_SIZE, PhysicsConstants::GEOMETRY_HALF_SIZE), *App->physics->GetDefaultMaterial());
		assert(gShape != nullptr);

		gActor = App->physics->CreateRigidStatic(physx::PxTransform(physx::PxIDENTITY()), *gShape);
		assert(gActor != nullptr);
		App->physics->AddActor(*gActor);

		gActor->setActorFlag(physx::PxActorFlag::eSEND_SLEEP_NOTIFIES, true);

		math::float4x4 globalMatrix = parent->transform->GetGlobalMatrix();
		UpdateTransform(globalMatrix);
	}

	rigidActorType = RigidActorTypes::RigidStatic;

	// -----

	SetUseGravity(false);
}

ComponentRigidStatic::ComponentRigidStatic(const ComponentRigidStatic& componentRigidStatic, GameObject* parent, bool include) : ComponentRigidActor(componentRigidStatic, parent, ComponentTypes::RigidStaticComponent, include)
{
	if (include)
	{
		physx::PxShape* gShape = nullptr;

		if (parent->cmp_collider != nullptr)
			gShape = parent->cmp_collider->GetShape();
		else if (parent->boundingBox.IsFinite())
			gShape = App->physics->CreateShape(physx::PxBoxGeometry(parent->boundingBox.HalfSize().x, parent->boundingBox.HalfSize().y, parent->boundingBox.HalfSize().z), *App->physics->GetDefaultMaterial());
		else
			gShape = App->physics->CreateShape(physx::PxBoxGeometry(PhysicsConstants::GEOMETRY_HALF_SIZE, PhysicsConstants::GEOMETRY_HALF_SIZE, PhysicsConstants::GEOMETRY_HALF_SIZE), *App->physics->GetDefaultMaterial());
		assert(gShape != nullptr);

		gActor = App->physics->CreateRigidStatic(physx::PxTransform(physx::PxIDENTITY()), *gShape);
		assert(gActor != nullptr);
		App->physics->AddActor(*gActor);

		gActor->setActorFlag(physx::PxActorFlag::eSEND_SLEEP_NOTIFIES, true);

		math::float4x4 globalMatrix = parent->transform->GetGlobalMatrix();
		UpdateTransform(globalMatrix);
	}

	rigidActorType = componentRigidStatic.rigidActorType;

	// -----

	SetUseGravity(componentRigidStatic.useGravity);
}

ComponentRigidStatic::~ComponentRigidStatic() {}

// ----------------------------------------------------------------------------------------------------

void ComponentRigidStatic::OnUniqueEditor() 
{
#ifndef GAMEMODE
	if (ImGui::CollapsingHeader("Rigid Static", ImGuiTreeNodeFlags_DefaultOpen))
	{
		//ComponentRigidActor::OnUniqueEditor();
	}
#endif
}

// ----------------------------------------------------------------------------------------------------

void ComponentRigidStatic::Update()
{
	if (useGravity)
		UpdateGameObjectTransform();
}

uint ComponentRigidStatic::GetInternalSerializationBytes()
{
	return ComponentRigidActor::GetInternalSerializationBytes();
}

void ComponentRigidStatic::OnInternalSave(char*& cursor)
{
	ComponentRigidActor::OnInternalSave(cursor);
}

void ComponentRigidStatic::OnInternalLoad(char*& cursor)
{
	ComponentRigidActor::OnInternalLoad(cursor);
}

// ----------------------------------------------------------------------------------------------------