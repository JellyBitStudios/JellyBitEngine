#ifndef __COMPONENT_MATERIAL_H__
#define __COMPONENT_MATERIAL_H__

#include "Component.h"

#include <string>

#include "MathGeoLib\include\Math\float4.h"

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

	void SetColor(math::float4& color);
	math::float4 GetColor() const;

public:

	uint res = 0;
	class ResourceMaterial* currentResource = 0;

	bool useColor = false; // not save&load

private:

	math::float4 color = math::float4::zero; // not save&load
};

#endif