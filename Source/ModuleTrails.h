#ifndef __MouduleTrails_H__
#define __MouduleTrails_H__

#include "Module.h"
#include "Application.h"

#include "ComponentTrail.h"

class ModuleTrails : public Module
{
public:
	ModuleTrails(bool start_enabled = true);
	~ModuleTrails();

	update_status Update();

	void Draw();
	void DebugDraw() const;

	void OnSystemEvent(System_Event event);

	void RemoveTrail(ComponentTrail* emitter);
public:

	std::list<ComponentTrail*> trails;

private:
	
};

#endif // !__MouduleTrails_H__
