#ifndef __COMPONENT_LIGHT_H__
#define __COMPONENT_LIGHT_H__

#include "Component.h"

enum LightTypes { DirectionalLight = 1, PointLight, SpotLight };

class ComponentLight : public Component
{
public:

	ComponentLight(GameObject* parent);
	ComponentLight(const ComponentLight& componentCamera, GameObject* parent);
	~ComponentLight();

	void OnUniqueEditor();

	uint GetInternalSerializationBytes();
	virtual void OnInternalSave(char*& cursor);
	virtual void OnInternalLoad(char*& cursor);

public:

	LightTypes lightType = LightTypes::DirectionalLight;
	float color[3];
	// constant = 1.0f; constant is always 1, so we dont need to store it
	float linear = 0.09f;
	float quadratic = 0.032f;

};

#endif