#ifndef __COMPONENT_BONE_H__
#define __COMPONENT_BONE_H__

#include "Component.h"

class ComponentBone : public Component
{
public:

	ComponentBone(GameObject* parent);
	ComponentBone(const ComponentBone& componentBone, GameObject* parent);
	~ComponentBone();

	void OnUniqueEditor();

	uint GetInternalSerializationBytes();
	void OnInternalSave(char*& cursor);
	void OnInternalLoad(char*& cursor);

	// ----------------------------------------------------------------------------------------------------

	void SetResource(uint boneUuid);

public:

	uint res = 0;
};

#endif