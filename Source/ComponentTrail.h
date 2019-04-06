#ifndef __COMPONENT_TRAIL_H__
#define __COMPONENT_TRAIL_H__

#include "Component.h"

#include "Globals.h"

#include "MathGeoLib/include/MathGeoLib.h"

#include "Timer.h"
#include <list>


#define MAX_TRAIL_NODE 500

struct TrailNode
{
	math::float3 originHigh = math::float3::zero;
	math::float3 originLow = math::float3::zero;
	
	Uint32 createTime = 0u;

	TrailNode(math::float3 high, math::float3 low)
	{
		originHigh = high;
		originLow = low;

		createTime = SDL_GetTicks();
	}
};

class ComponentTrail : public Component
{
public:

	ComponentTrail(GameObject* parent);
	ComponentTrail(const ComponentTrail& componentTransform, GameObject* parent);
	~ComponentTrail();

	void Update();

	void OnUniqueEditor();

	void SetMaterialRes(uint materialUuid);


	//Serialization
	uint GetInternalSerializationBytes();
	virtual void OnInternalSave(char*& cursor);
	virtual void OnInternalLoad(char*& cursor);

public:

	std::list<TrailNode*> trailVertex;
	Timer timer;

	bool create = true;

	uint materialRes = 0;

private:

	float minDistance = 0.05f;

	int lifeTime = 200;


};

#endif //! __COMPONENT_TRAIL_H__