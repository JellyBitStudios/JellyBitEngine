#ifndef __LIGHTS_H__
#define __LIGHTS_H__

#include <vector>

#include "MathGeoLib\include\Math\float3.h"

class Lights
{
public:

	Lights();
	~Lights();

	void AddLight(class ComponentLight* light);
	bool EraseLight(class ComponentLight* light);
	void UseLights(const unsigned int shaderProgram);
	void DebugDrawLights() const;

	// Fog
	void UpdateFogMaxDistUniform();
	void UpdateFogMinDistUniform();
	void UpdateFogColorUniform();

public:

	float ambientValue = 0.3f;

	// Fog
	float fogMaxDist = 0.0f;
	float fogMinDist = 0.0f;
	math::float3 fogColor = math::float3::zero;

private:

	std::vector<class ComponentLight*> lights;
};

#endif
