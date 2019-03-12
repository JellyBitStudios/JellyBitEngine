#ifndef __LIGHTS_H__
#define __LIGHTS_H__

#include <vector>

class Lights
{
public:

	Lights();
	~Lights();

	void AddLight(const class ComponentLight* light);
	bool EraseLight(const class ComponentLight* light);
	void UseLights(const unsigned int shaderProgram) const;
	void DebugDrawLights() const;

private:

	std::vector<const class ComponentLight*> lights;

};

#endif
