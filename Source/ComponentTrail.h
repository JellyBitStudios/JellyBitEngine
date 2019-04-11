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

enum TrailVector
{
	X,
	Y,
	Z
};
class ComponentTrail : public Component
{
public:

	ComponentTrail(GameObject* parent);
	ComponentTrail(const ComponentTrail& componentTransform, GameObject* parent);
	~ComponentTrail();

	void Update();

	void OnUniqueEditor();

	void SetSpawnSize(math::float3 size);

	void SetMaterialRes(uint materialUuid);

	void Start();
	void Stop();
	void HardStop();

	void SetVector(TrailVector vec);
	inline TrailVector GetVector() { return vector; }

	void SetLifeTime(int lifeTime);
	inline int GetLifeTime() { return lifeTime; }

	void SetMinDistance(float minDistance);
	inline float GetMinDistance() { return minDistance; }

	void SetColor(math::float4 color);
	inline math::float4 GetColor() { return color; }

	//Serialization
	uint GetInternalSerializationBytes();
	virtual void OnInternalSave(char*& cursor);
	virtual void OnInternalLoad(char*& cursor);

public:
	std::list<TrailNode*> trailVertex;
	Timer timer;

	uint materialRes = 0;

	math::float4 color = math::float4::one;

	TrailVector vector = TrailVector::X;
	
	bool orient = true;

private:
	bool create = true;
	bool customSpawn = false;


	float minDistance = 0.05f;
	int lifeTime = 200;

	int hight = 5;
	int low = 4;

	math::OBB spawnBox;
	math::OBB _spawnBox;
	math::OBB originalSpawnBox;
};

#endif //! __COMPONENT_TRAIL_H__