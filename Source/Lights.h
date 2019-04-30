#ifndef __LIGHTS_H__
#define __LIGHTS_H__

#include <vector>

#include "MathGeoLib\include\Math\float3.h"

struct Fog
{
	math::float3 color = math::float3(0.502f, 0.502f, 0.502f);
	//float minDist = 0.0f;
	//float maxDist = 300.0f;
	float density = 0.01f;
};

class Lights
{
public:

	Lights();
	~Lights();

	void AddLight(class ComponentLight* light);
	bool EraseLight(class ComponentLight* light);
	void UseLights(const unsigned int shaderProgram);
	void DebugDrawLights() const;

public:

	float ambientValue = 0.3f;
	Fog fog;

private:

	std::vector<class ComponentLight*> lights;
};

#endif
