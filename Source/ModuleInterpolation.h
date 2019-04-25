#ifndef __ModuleInterpolation_H__
#define __ModuleInterpolation_H__

#include "Module.h"
#include "Application.h"

#include "ComponentInterpolation.h"

class ModuleInterpolation : public Module
{
public:
	ModuleInterpolation(bool start_enabled = true);
	~ModuleInterpolation();

	update_status Update();

	void OnSystemEvent(System_Event event);

public:

	std::list<ComponentInterpolation*> interpolations;

private:
	
};

#endif // !__ModuleInterpolation_H__
