#include "Lights.h"
#include "ComponentLight.h"

#include "glew\include\GL\glew.h"

#include "GameObject.h"
#include "ComponentTransform.h"

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

void Lights::UseLights(const unsigned int shaderID) const
{
	for (int i = 0; i < lights.size(); ++i)
	{
		char str[20];
		sprintf(str, "lights[%i].Dir", i);
		math::float3 dir = lights[i]->GetParent()->transform->GetGlobalMatrix().WorldZ();
		glUniform3fv(glGetUniformLocation(shaderID, str), 1, dir.ptr());
		sprintf(str, "lights[%i].Color", i);
		glUniform3fv(glGetUniformLocation(shaderID, str), 1, lights[i]->color);
	}
}
