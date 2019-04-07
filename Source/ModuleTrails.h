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

	update_status PostUpdate();

	void Draw();
	void RearrangeVertex(std::list<ComponentTrail *>::iterator &trail, std::list<TrailNode *>::iterator &curr, std::list<TrailNode *>::iterator &next, float &currUV, float &nextUV, math::float3 &originHigh, math::float3 &originLow, math::float3 &destinationHigh, math::float3 &destinationLow);
	void GetOriginAndDest(std::list<ComponentTrail *>::iterator &trail, float &origin, std::list<TrailNode *>::iterator &curr, float &dest, std::list<TrailNode *>::iterator &next);
	void DebugDraw() const;

	void OnSystemEvent(System_Event event);

	void RemoveTrail(ComponentTrail* emitter);
public:

	std::list<ComponentTrail*> trails;

private:
	
};

#endif // !__MouduleTrails_H__
