#ifndef __LIGHTS_H__
#define __LIGHTS_H__

#include <vector>

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

private:

	std::vector<class ComponentLight*> lights;

};

#endif
