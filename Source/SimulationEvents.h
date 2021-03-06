#ifndef __SIMULATION_EVENTS_H__
#define __SIMULATION_EVENTS_H__

#include "physx\include\PxPhysicsAPI.h"

#include "MathGeoLib\include\Math\float3.h"

#include <vector>

class ModulePhysics;
class ComponentRigidActor;
class ComponentCollider;
class GameObject;

enum CollisionTypes
{
	OnCollisionEnter,
	OnCollisionStay,
	OnCollisionExit,
	OnTriggerEnter,
	OnTriggerStay,
	OnTriggerExit
};

class ContactPoint
{
public:

	ContactPoint();
	ContactPoint(math::float3& point, math::float3& normal, float separation, ComponentCollider* thisCollider, ComponentCollider* otherCollider);
	~ContactPoint();

	math::float3 GetPoint() const;
	math::float3 GetNormal() const;
	float GetSeparation() const;
	ComponentCollider* GetThisCollider() const;
	ComponentCollider* GetOtherCollider() const;

private:

	math::float3 point = math::float3::zero;// the point of contact
	math::float3 normal = math::float3::zero; // the normal of the contact point
	float separation = 0.0f; // the distance between the colliders at the contact point
	ComponentCollider* thisCollider = nullptr;
	ComponentCollider* otherCollider = nullptr;
};

// ----------------------------------------------------------------------------------------------------

class Collision
{
public:

	Collision();
	Collision(GameObject* gameObject, ComponentCollider* collider, ComponentRigidActor* actor, math::float3& impulse, math::float3& relativeVelocity, std::vector<ContactPoint>& contactPoints);
	~Collision();

	GameObject* GetGameObject() const;
	ComponentCollider* GetCollider() const;
	ComponentRigidActor* GetActor() const;
	math::float3 GetImpulse() const;
	math::float3 GetRelativeVelocity() const;
	std::vector<ContactPoint> GetContactPoints() const;

private:

	GameObject* gameObject = nullptr; // the game object we hit
	ComponentCollider* collider = nullptr; // the collider we hit
	ComponentRigidActor* actor = nullptr; // the actor we hit
	math::float3 impulse = math::float3::zero; // the total impulse applied to this contact pair to resolve the collision
	math::float3 relativeVelocity = math::float3::zero; // the relative linear velocity of the two colliding objects (A - B)
	std::vector<ContactPoint> contactPoints; // the contact points generated by the physics engine
};

// ----------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------

enum SimulationEventTypes
{
	OnWake,
	OnSleep
};

class SimulationEventCallback : public physx::PxSimulationEventCallback
{
public:

	SimulationEventCallback(ModulePhysics* callback);
	~SimulationEventCallback();

	// Happen before fetchResults() (swaps the buffers)
	void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count);
	void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs);
	void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count);

	void onAdvance(const physx::PxRigidBody*const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count) {}

	// Happen after fetchResults() (swaps the buffers)
	void onWake(physx::PxActor** actors, physx::PxU32 count);
	void onSleep(physx::PxActor** actors, physx::PxU32 count);

private:

	ModulePhysics* callback = nullptr;
};

#endif