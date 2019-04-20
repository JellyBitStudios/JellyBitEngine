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
	void ScreenQuadToFrustum(int posX, int posY, int posW, int posH) const;

	math::Frustum CreateFrustum(math::Frustum cameraFrustum, float width, float heith) const;
};

#endif

#endif
