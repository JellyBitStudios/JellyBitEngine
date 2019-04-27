#ifndef __COMPONENT_PROJECTOR_H__
#define __COMPONENT_PROJECTOR_H__

#include "Component.h"

#include "Globals.h"

#include "MathGeoLib\include\Geometry\Frustum.h"

/*
Texture:
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
*/

class ComponentProjector : public Component
{
public:

	ComponentProjector(GameObject* parent, bool include = true);
	ComponentProjector(const ComponentProjector& componentProjector, GameObject* parent, bool include = true);
	~ComponentProjector();

	void UpdateTransform();

	void OnUniqueEditor();

	uint GetInternalSerializationBytes();
	void OnInternalSave(char*& cursor);
	void OnInternalLoad(char*& cursor);

	// ----------------------------------------------------------------------------------------------------

	void Draw() const;

	// ----------------------------------------------------------------------------------------------------

	// Material
	void SetMaterialRes(uint materialUuid);
	void SetMaterialRes(std::string materialName);
	uint GetMaterialRes() const;
	std::string GetMaterialResName() const;

	void SetAlphaMultiplier(float alphaMultiplier);
	float GetAlphaMultiplier() const;

	// Mesh
	void SetMeshRes(uint meshUuid);
	uint GetMeshRes() const;

	// Filter mask
	void SetFilterMask(uint filterMask);
	uint GetFilterMask() const;

	// Frustum
	void SetFOV(float fov);
	float GetFOV() const;
	float GetNearPlaneDistance();
	void SetNearPlaneDistance(float nearPlane);
	float GetFarPlaneDistance();
	void SetFarPlaneDistance(float farPlane);

	math::Frustum GetFrustum() const;
	math::float4x4 GetOpenGLViewMatrix() const;
	math::float4x4 GetOpenGLProjectionMatrix() const;

private:

	uint materialRes = 0;
	uint meshRes = 0;

	float alphaMultiplier = 1.0f;

	uint filterMask = 0;

	math::Frustum frustum;
};

#endif
