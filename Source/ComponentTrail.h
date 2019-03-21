#ifndef __COMPONENT_TRAIL_H__
#define __COMPONENT_TRAIL_H__

#include "Component.h"

#include "Globals.h"

#include "MathGeoLib/include/MathGeoLib.h"

struct TrailNode
{
	math::float3 originHigh = math::float3::zero;
	math::float3 originLow = math::float3::zero;
	math::float3 destHigh = math::float3::zero;
	math::float3 destLow = math::float3::zero;
};

class ComponentTrail : public Component
{
public:

	ComponentTrail(GameObject* parent);
	ComponentTrail(const ComponentTrail& componentTransform, GameObject* parent);
	~ComponentTrail();

	void Update();


	//Serialization
	uint GetInternalSerializationBytes() { return 0; }
	virtual void OnInternalSave(char*& cursor) {}
	virtual void OnInternalLoad(char*& cursor) {}

public:

	TrailNode test;
	TrailNode test2;

	bool ready1 = false;
	bool ready2 = false;

private:


};

#endif //! __COMPONENT_TRAIL_H__