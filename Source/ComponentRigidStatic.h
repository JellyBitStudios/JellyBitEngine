#ifndef __COMPONENT_RIGID_STATIC_H__
#define __COMPONENT_RIGID_STATIC_H__

#include "Component.h"
#include "ComponentRigidActor.h"

class ComponentRigidStatic : public ComponentRigidActor
{
public:

	ComponentRigidStatic(GameObject* parent, bool include = true);
	ComponentRigidStatic(const ComponentRigidStatic& componentRigidStatic, GameObject* parent, bool include = true);
	~ComponentRigidStatic();

	void OnUniqueEditor();

	void Update();

	uint GetInternalSerializationBytes();
	void OnInternalSave(char*& cursor);
	void OnInternalLoad(char*& cursor);
};

#endif