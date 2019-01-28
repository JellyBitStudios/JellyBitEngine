#ifndef __COMPONENT_RIGID_DYNAMIC_H__
#define __COMPONENT_RIGID_DYNAMIC_H__

#include "Component.h"
#include "ComponentRigidBody.h"

class ComponentRigidDynamic : public ComponentRigidBody
{
public:

	ComponentRigidDynamic(GameObject* parent);
	//ComponentRigidBody(const ComponentRigidBody& componentRigidBody);
	~ComponentRigidDynamic();

	void Update();

	void OnUniqueEditor();

	void ToggleKinematic() const;

	//void OnInternalSave(JSON_Object* file);
	//void OnLoad(JSON_Object* file);

private:

	bool isKinematic = false;
};

#endif