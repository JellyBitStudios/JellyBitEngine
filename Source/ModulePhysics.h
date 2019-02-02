#ifndef __MODULE_PHYSICS_H__
#define __MODULE_PHYSICS_H__

#include "Module.h"

#include "physx/include/PxPhysicsAPI.h"
#include "physx/include/extensions/PxDefaultAllocator.h"
#include "physx/include/extensions/PxDefaultCpuDispatcher.h"

#include "MathGeoLib/include/Math/float3.h"

#include <vector>

class DefaultErrorCallback : public physx::PxErrorCallback
{
public:

	DefaultErrorCallback();
	~DefaultErrorCallback();

	void reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line);
};

// ----------------------------------------------------------------------------------------------------

class ModulePhysics;

enum SimulationEventTypes
{
	SimulationEventOnWake,
	SimulationEventOnSleep,
	SimulationEventOnContact,
	SimulationEventOnTrigger
};

class Collision
{
	Collision();
	~Collision();

private:

	physx::PxActor* gActor = nullptr; // actor
	physx::PxShape* gShape = nullptr; // collider

};

// Collision filtering example
/*
enum FilterGroup
{
	eSUBMARINE = (1 << 0),
	eMINE_HEAD = (1 << 1),
	eMINE_LINK = (1 << 2),
	eCRAB = (1 << 3),
	eHEIGHTFIELD = (1 << 4),
};
*/

class SimulationEventCallback : public physx::PxSimulationEventCallback
{
public:

	SimulationEventCallback(ModulePhysics* callback);
	~SimulationEventCallback();

	// Happen before fetchResults() (swaps the buffers)
	void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count) {}
	void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs);
	void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count);

	void onAdvance(const physx::PxRigidBody*const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count) {}

	// Happen after fetchResults() (swaps the buffers)
	void onWake(physx::PxActor** actors, physx::PxU32 count);
	void onSleep(physx::PxActor** actors, physx::PxU32 count);

private:

	ModulePhysics* callback = nullptr;
};

// ----------------------------------------------------------------------------------------------------

class ComponentRigidActor;
class ComponentCollider;
enum ComponentTypes;

class ModulePhysics : public Module
{
public:

	ModulePhysics(bool start_enabled = true);
	~ModulePhysics();

	bool Init(JSON_Object* jObject);
	bool Start();
	update_status PreUpdate();
	update_status Update();
	update_status PostUpdate();
	bool CleanUp();

	// Create elements
	physx::PxRigidStatic* CreateRigidStatic(const physx::PxTransform& transform, physx::PxShape& shape) const;
	physx::PxRigidDynamic* CreateRigidDynamic(const physx::PxTransform& transform, physx::PxShape& shape, float density, bool isKinematic = false) const;
	void RemoveActor(physx::PxActor& actor) const;

	physx::PxShape* CreateShape(const physx::PxGeometry& geometry, const physx::PxMaterial& material, bool isExclusive = true) const;

	// Create components
	ComponentRigidActor* CreateRigidActorComponent(GameObject* parent, ComponentTypes componentRigidActorType);
	bool AddRigidActorComponent(ComponentRigidActor* toAdd);
	bool EraseRigidActorComponent(ComponentRigidActor* toErase);

	ComponentCollider* CreateColliderComponent(GameObject* parent, ComponentTypes componentColliderType);
	bool AddColliderComponent(ComponentCollider* toAdd);
	bool EraseColliderComponent(ComponentCollider* toErase);

	// Callbacks
	void OnSimulationEvent(physx::PxActor* actorA, physx::PxActor* actorB, SimulationEventTypes simulationEventType) const;
	//void OnCollision(physx::)
	// ----------

	std::vector<ComponentRigidActor*> GetRigidActorComponents() const;
	ComponentRigidActor* GetRigidActorComponentFromActor(physx::PxActor* actor) const;
	std::vector<ComponentCollider*> GetColliderComponents() const;

	// General configuration values
	void SetGravity(math::float3 gravity);
	math::float3 GetGravity() const;

	void SetDefaultMaterial(physx::PxMaterial* material);
	physx::PxMaterial* GetDefaultMaterial() const;

private:

	physx::PxFoundation* gFoundation = nullptr;
	physx::PxPhysics* gPhysics = nullptr;
	physx::PxScene* gScene = nullptr;
	physx::PxDefaultCpuDispatcher* gDispatcher = nullptr;

	float gAccumulator = 0.0f;

	std::vector<ComponentRigidActor*> rigidActorComponents;
	std::vector<ComponentCollider*> colliderComponents;

	// General configuration values
	math::float3 gravity = math::float3::zero;
	physx::PxMaterial* defaultMaterial = nullptr;
};

#endif