#ifndef __COMPONENT_LIGHT_H__
#define __COMPONENT_LIGHT_H__

#include "Component.h"

#include <vector>

enum LightTypes { DirectionalLight = 1, PointLight, SpotLight };

class ComponentLight : public Component
{
public:

	ComponentLight(GameObject* parent);
	ComponentLight(const ComponentLight& componentCamera, GameObject* parent);
	~ComponentLight();

	void OnUniqueEditor();
	void Update();

	uint GetInternalSerializationBytes();
	virtual void OnInternalSave(char*& cursor);
	virtual void OnInternalLoad(char*& cursor);

public:

	LightTypes lightType = LightTypes::DirectionalLight;
	float color[3];
	// constant = 1.0f; constant is always 1, so we dont need to store it
	float linear = 0.09f;
	float quadratic = 0.032f;

	bool behaviour = false;
	bool enabled = true;

	struct LightBehaviourBlock
	{
		float color[3];
		float interval = 1.0f;
		float elpasedInterval = 0.0f;
		float linear = 0.09f;
		float quadratic = 0.032f;
		bool  enabled = true;
	};

	float timer = 0.0f;
	float nextInterval = 0.0f;
	int currentLightBB = 0;
	std::vector<LightBehaviourBlock> lightBB;
};

#endif