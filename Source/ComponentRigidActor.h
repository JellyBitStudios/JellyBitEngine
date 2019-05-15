#ifndef __COMPONENT_RIGID_ACTOR_H__
#define __COMPONENT_RIGID_ACTOR_H__

#include "Component.h"

#include "physx\include\PxPhysicsAPI.h"

#include "MathGeoLib\include\Math\float4x4.h"

enum RigidActorTypes
{
	NoRigidActor,
	RigidStatic,
	RigidDynamic
};

class ComponentRigidActor : public Component
{
public:

	ComponentRigidActor(GameObject* parent, ComponentTypes componentRigidActorType, bool include = true);
	ComponentRigidActor(const ComponentRigidActor& componentRigidActor, GameObject* parent, ComponentTypes componentRigidActorType, bool include = true);
	virtual ~ComponentRigidActor();

	virtual void OnUniqueEditor();

	virtual void Update();

	virtual uint GetInternalSerializationBytes();
	virtual void OnInternalSave(char*& cursor);
	virtual void OnInternalLoad(char*& cursor);

	// ----------------------------------------------------------------------------------------------------

	void UpdateShape(physx::PxShape* shape) const;
	void ClearActor();
	void UpdateTransform(math::float4x4& globalMatrix) const;
	void UpdateGameObjectTransform() const;

	// Sets
	void SetUseGravity(bool useGravity);

	// Gets
	inline bool GetUseGravity() { return useGravity; }
	physx::PxRigidActor* GetActor() const;
	RigidActorTypes GetRigidActorType() const;

	// Callbacks
	void OnWake();
	void OnSleep();

protected:

	bool useGravity = true;

	// -----

	physx::PxRigidActor* gActor = nullptr;
	RigidActorTypes rigidActorType = RigidActorTypes::NoRigidActor;
};

#endif