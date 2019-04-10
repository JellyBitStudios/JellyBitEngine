#ifndef __COMPONENT_MATERIAL_H__
#define __COMPONENT_MATERIAL_H__

#include "Component.h"
#include <string>

class ComponentMaterial : public Component
{
public:

	ComponentMaterial(GameObject* parent);
	ComponentMaterial(const ComponentMaterial& componentMaterial, GameObject* parent);
	~ComponentMaterial();

	void OnUniqueEditor();

	uint GetInternalSerializationBytes();
	virtual void OnInternalSave(char*& cursor);
	virtual void OnInternalLoad(char*& cursor);

	// ----------------------------------------------------------------------------------------------------

	void SetResource(uint materialUuid);
	void SetResourceByName(std::string materialName);

public:

	uint res = 0;
	class ResourceMaterial* currentResource = 0;
};

#endif