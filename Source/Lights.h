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

private:

	std::vector<const class ComponentLight*> lights;

};

#endif
