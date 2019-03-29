#include "SimulationEvents.h"

#include <assert.h>

#include "ModulePhysics.h"
#include "ComponentRigidActor.h"
#include "ComponentRigidDynamic.h"

ContactPoint::ContactPoint() {}

ContactPoint::ContactPoint(math::float3& point, math::float3& normal, float separation, ComponentCollider* thisCollider, ComponentCollider* otherCollider) :
	point(point), normal(normal), separation(separation), thisCollider(thisCollider), otherCollider(otherCollider) {}

ContactPoint::~ContactPoint() {}

math::float3 ContactPoint::GetPoint() const
{
	return point;
}

math::float3 ContactPoint::GetNormal() const
{
	return normal;
}

float ContactPoint::GetSeparation() const
{
	return separation;
}

ComponentCollider* ContactPoint::GetThisCollider() const
{
	return thisCollider;
}

ComponentCollider* ContactPoint::GetOtherCollider() const
{
	return otherCollider;
}

// ----------------------------------------------------------------------------------------------------

Collision::Collision() {}

Collision::Collision(GameObject* gameObject, ComponentCollider* collider, ComponentRigidActor* actor, math::float3& impulse, math::float3& relativeVelocity, std::vector<ContactPoint>& contactPoints) :
	gameObject(gameObject), collider(collider), actor(actor), impulse(impulse), relativeVelocity(relativeVelocity), contactPoints(contactPoints) {}

Collision::~Collision() {}

GameObject* Collision::GetGameObject() const
{
	return gameObject;
}

ComponentCollider* Collision::GetCollider() const
{
	return collider;
}

ComponentRigidActor* Collision::GetActor() const
{
	return actor;
}

math::float3 Collision::GetImpulse() const
{
	return impulse;
}

math::float3 Collision::GetRelativeVelocity() const
{
	return relativeVelocity;
}

std::vector<ContactPoint> Collision::GetContactPoints() const
{
	return contactPoints;
}

// ----------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------

SimulationEventCallback::SimulationEventCallback(ModulePhysics* callback)
{
	assert(callback != nullptr);
	this->callback = callback;
}

SimulationEventCallback::~SimulationEventCallback() {}

void SimulationEventCallback::onConstraintBreak(physx::PxConstraintInfo * constraints, physx::PxU32 count)
{
	// The joint no longer partakes in simulation, altough it remains attached to its actors until it is deleted
	// TODO:

	for (physx::PxU32 i = 0; i < count; ++i)
	{
		physx::PxJoint* joint = reinterpret_cast<physx::PxJoint*>(constraints[i].externalReference);

	}
}

void SimulationEventCallback::onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs)
{
	for (physx::PxU32 i = 0; i < nbPairs; ++i)
	{
		const physx::PxContactPair& contactPair = pairs[i];

		if (contactPair.flags
			& (physx::PxContactPairFlag::eREMOVED_SHAPE_0
				| physx::PxContactPairFlag::eREMOVED_SHAPE_1))
			continue;
		else
		{
			// Collision A
			ComponentCollider* colliderA = callback->FindColliderComponentByShape(contactPair.shapes[1]);
			ComponentRigidActor* actorA = callback->FindRigidActorComponentByActor(pairHeader.actors[1]);
			GameObject* gameObjectA = actorA != nullptr ? actorA->GetParent() : nullptr;
			ComponentCollider* thisColliderA = callback->FindColliderComponentByShape(contactPair.shapes[0]);

			// Collision B
			ComponentCollider* colliderB = callback->FindColliderComponentByShape(contactPair.shapes[0]);
			ComponentRigidActor* actorB = callback->FindRigidActorComponentByActor(pairHeader.actors[0]);
			GameObject* gameObjectB = actorB != nullptr ? actorB->GetParent() : nullptr;
			ComponentCollider* thisColliderB = callback->FindColliderComponentByShape(contactPair.shapes[1]);

			math::float3 totalImpulse = math::float3::zero;
			std::vector<ContactPoint> contactPointsA;
			std::vector<ContactPoint> contactPointsB;
			if (contactPair.contactCount > 0)
			{
				std::vector<physx::PxContactPairPoint> contactPairPoints;
				contactPairPoints.resize(contactPair.contactCount);
				contactPair.extractContacts(&contactPairPoints[0], contactPair.contactCount);

				for (physx::PxU32 j = 0; j < contactPair.contactCount; ++j)
				{
					math::float3 point = math::float3(contactPairPoints[i].position.x, contactPairPoints[i].position.y, contactPairPoints[i].position.z);
					math::float3 normal = math::float3(contactPairPoints[i].normal.x, contactPairPoints[i].normal.y, contactPairPoints[i].normal.z);
					math::float3 impulse = math::float3(contactPairPoints[i].impulse.x, contactPairPoints[i].impulse.y, contactPairPoints[i].impulse.z);
					totalImpulse += impulse;

					ContactPoint contactPointA(point, normal, contactPairPoints[i].separation, colliderA, colliderB);
					contactPointsA.push_back(contactPointA);

					ContactPoint contactPointB(point, normal, contactPairPoints[i].separation, colliderB, colliderA);
					contactPointsB.push_back(contactPointB);
				}
			}

			// Collision A
			math::float3 relativeVelocityA = math::float3::zero;
			if (actorA != nullptr && actorB != nullptr
				&& actorA->GetRigidActorType() == RigidActorTypes::RigidDynamic)
			{
				relativeVelocityA = ((ComponentRigidDynamic*)actorA)->GetLinearVelocity();
			}
			if (actorB != nullptr && actorA != nullptr
				&& actorB->GetRigidActorType() == RigidActorTypes::RigidDynamic)
			{
				relativeVelocityA -= ((ComponentRigidDynamic*)actorB)->GetLinearVelocity();
			}
			Collision collisionA(gameObjectA, colliderA, actorA, totalImpulse, relativeVelocityA, contactPointsA);

			// Collision B
			math::float3 relativeVelocityB = math::float3::zero;
			if (actorA != nullptr && actorB != nullptr
				&& actorB->GetRigidActorType() == RigidActorTypes::RigidDynamic)
			{
				relativeVelocityB = ((ComponentRigidDynamic*)actorB)->GetLinearVelocity();
			}
			if (actorB != nullptr && actorA != nullptr
				&& actorA->GetRigidActorType() == RigidActorTypes::RigidDynamic)
			{
				relativeVelocityB -= ((ComponentRigidDynamic*)actorA)->GetLinearVelocity();
			}
			Collision collisionB(gameObjectB, colliderB, actorB, totalImpulse, relativeVelocityB, contactPointsB);

			if (contactPair.events & physx::PxPairFlag::eNOTIFY_TOUCH_FOUND)
			{
				callback->OnCollision(thisColliderA, collisionA, CollisionTypes::OnCollisionEnter);
				callback->OnCollision(thisColliderB, collisionB, CollisionTypes::OnCollisionEnter);
			}
			else if (contactPair.events & physx::PxPairFlag::eNOTIFY_TOUCH_PERSISTS)
			{
				callback->OnCollision(thisColliderA, collisionA, CollisionTypes::OnCollisionStay);
				callback->OnCollision(thisColliderB, collisionB, CollisionTypes::OnCollisionStay);
			}
			else if (contactPair.events & physx::PxPairFlag::eNOTIFY_TOUCH_LOST)
			{
				callback->OnCollision(thisColliderA, collisionA, CollisionTypes::OnCollisionExit);
				callback->OnCollision(thisColliderB, collisionB, CollisionTypes::OnCollisionExit);
			}
		}
	}
}

void SimulationEventCallback::onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count)
{
	for (physx::PxU32 i = 0; i < count; ++i)
	{
		const physx::PxTriggerPair& triggerPair = pairs[i];

		if (triggerPair.flags &
			(physx::PxTriggerPairFlag::eREMOVED_SHAPE_TRIGGER
				| physx::PxTriggerPairFlag::eREMOVED_SHAPE_OTHER))
			continue;
		else
		{
			// Collision A
			ComponentCollider* colliderA = callback->FindColliderComponentByShape(triggerPair.triggerShape);
			ComponentRigidActor* actorA = callback->FindRigidActorComponentByActor(triggerPair.triggerActor);
			GameObject* gameObjectA = actorA != nullptr ? actorA->GetParent() : nullptr;
			Collision collisionA(gameObjectA, colliderA, actorA, math::float3(), math::float3(), std::vector<ContactPoint>());
			ComponentCollider* thisColliderA = callback->FindColliderComponentByShape(triggerPair.otherShape);

			// Collision B
			ComponentCollider* colliderB = callback->FindColliderComponentByShape(triggerPair.otherShape);
			ComponentRigidActor* actorB = callback->FindRigidActorComponentByActor(triggerPair.otherActor);
			GameObject* gameObjectB = actorB != nullptr ? actorB->GetParent() : nullptr;
			Collision collisionB(gameObjectB, colliderB, actorB, math::float3(), math::float3(), std::vector<ContactPoint>());
			ComponentCollider* thisColliderB = callback->FindColliderComponentByShape(triggerPair.triggerShape);

			if (triggerPair.status & physx::PxPairFlag::eNOTIFY_TOUCH_FOUND)
			{
				callback->OnCollision(thisColliderA, collisionA, CollisionTypes::OnTriggerEnter);
				callback->OnCollision(thisColliderB, collisionB, CollisionTypes::OnTriggerEnter);
			}
			else if (triggerPair.status & physx::PxPairFlag::eNOTIFY_TOUCH_LOST)
			{
				callback->OnCollision(thisColliderA, collisionA, CollisionTypes::OnTriggerExit);
				callback->OnCollision(thisColliderB, collisionB, CollisionTypes::OnTriggerExit);
			}
		}
	}
}

void SimulationEventCallback::onWake(physx::PxActor** actors, physx::PxU32 count)
{
	for (physx::PxActor** actor = actors; *actor != nullptr; ++actor)
		callback->OnSimulationEvent(callback->FindRigidActorComponentByActor(*actor), SimulationEventTypes::OnWake);
}

void SimulationEventCallback::onSleep(physx::PxActor** actors, physx::PxU32 count)
{
	for (physx::PxActor** actor = actors; *actor != nullptr; ++actor)
		callback->OnSimulationEvent(callback->FindRigidActorComponentByActor(*actor), SimulationEventTypes::OnSleep);
}