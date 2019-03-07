#include "Lights.h"
#include "ComponentLight.h"

#include <algorithm>

Lights::Lights()
{
}

Lights::~Lights()
{
}

void Lights::AddLight(const ComponentLight* light)
{
	lights.push_back(light);
}

bool Lights::EraseLight(const ComponentLight* light)
{
	bool ret = false;

	if (lights.size() <= 0)
		return false;

	std::vector<const ComponentLight*>::const_iterator it = std::find(lights.begin(), lights.end(), light);
	ret = it != lights.end();

	if (ret)
		lights.erase(it);

	return ret;
}