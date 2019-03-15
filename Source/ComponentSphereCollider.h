#ifndef __COMPONENT_SPHERE_COLLIDER_H__
#define __COMPONENT_SPHERE_COLLIDER_H__

#include "Component.h"
#include "ComponentCollider.h"

class ComponentSphereCollider : public ComponentCollider
{
public:

	ComponentSphereCollider(GameObject* parent, bool include = true);
	ComponentSphereCollider(const ComponentSphereCollider& componentSphereCollider, GameObject* parent, bool include = true);
	~ComponentSphereCollider();

	void OnUniqueEditor();

	uint GetInternalSerializationBytes();
	void OnInternalSave(char*& cursor);
	void OnInternalLoad(char*& cursor);

	// ----------------------------------------------------------------------------------------------------

	void EncloseGeometry();
	void RecalculateShape();

	// Sets
	void SetRadius(float radius);

	// Gets
	physx::PxSphereGeometry GetSphereGeometry() const;

private:

	float radius = 0.5f;
};

#endif