#ifndef __RAYCASTER_H__
#define __RAYCASTER_H__

#ifndef GAMEMODE

#include "MathGeoLib\include\Math\float3.h"
#include "MathGeoLib/include/Geometry/Frustum.h"

class GameObject;

class Raycaster
{
public:

	Raycaster();
	~Raycaster();

	void ScreenPointToRay(int posX, int posY, float& distance, math::float3& hitPoint, GameObject** hit) const;
	void GetGOFromFrustum(math::Frustum frustumSelecteds) const;

	void ScreenQuadToFrustum(int posX, int posY, int posW, int posH);
	math::Frustum CreateFrustum(math::Frustum cameraFrustum, math::float2 width_heith, math::float2 centerPos) const;

public:
	math::Frustum frustumSelection;
};

#endif

#endif
