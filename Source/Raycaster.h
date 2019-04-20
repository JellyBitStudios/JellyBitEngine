#ifndef __RAYCASTER_H__
#define __RAYCASTER_H__

#ifndef GAMEMODE

#include "MathGeoLib\include\Math\float3.h"

class GameObject;

class Raycaster
{
public:

	Raycaster();
	~Raycaster();

	void ScreenPointToRay(int posX, int posY, float& distance, math::float3& hitPoint, GameObject** hit) const;
	bool ScreenQuadToFrustum(int posX, int posY, int posW, int posH) const;

	math::Frustum CreateFrustum(math::Frustum cameraFrustum, math::float2 width_heith, math::float2 centerPos) const;
};

#endif

#endif
