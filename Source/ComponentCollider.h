#ifndef __COMPONENT_COLLIDER_H__
#define __COMPONENT_COLLIDER_H__

#include "Component.h"

#include "physx/include/PxPhysicsAPI.h"
#include "MathGeoLib/include/Math/float3.h"

#include "ModulePhysics.h"

class ComponentCollider : public Component
{
public:

	ComponentCollider(GameObject* parent, ComponentTypes componentColliderType);
	//ComponentCollider(const ComponentRigidActor& componentRigidActor);
	virtual ~ComponentCollider();

	virtual void OnUniqueEditor();

	virtual void Update();

	virtual void ClearShape();
	virtual void RecalculateShape() = 0;
	void SetFiltering(physx::PxU32 filterGroup, physx::PxU32 filterMask);

	void SetIsTrigger(bool isTrigger);
	void ParticipateInContactTests(bool participateInContactTests);
	void ParticipateInSceneQueries(bool participateInSceneQueries);

	physx::PxShape* GetShape() const;

	void OnCollisionEnter(Collision& collision);
	void OnCollisionStay(Collision& collision);
	void OnCollisionExit(Collision& collision);
	void OnTriggerEnter(Collision& collision);
	void OnTriggerStay(Collision& collision);
	void OnTriggerExit(Collision& collision);

	//void OnInternalSave(JSON_Object* file);
	//void OnLoad(JSON_Object* file);

protected:

	bool isTrigger = false;
	bool participateInContactTests = true;
	bool participateInSceneQueries = true;
	physx::PxMaterial* gMaterial = nullptr;
	math::float3 center = math::float3::zero;

	physx::PxShape* gShape = nullptr;

private:

	bool triggerEnter = false;
	bool triggerExit = false;
	Collision collision;
};

#endif