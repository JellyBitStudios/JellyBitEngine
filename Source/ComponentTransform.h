#ifndef __COMPONENT_TRANSFORM_H__
#define __COMPONENT_TRANSFORM_H__

#include "Component.h"

#include "Globals.h"

#include "MathGeoLib\include\Math\float3.h"
#include "MathGeoLib\include\Math\Quat.h"
#include "MathGeoLib\include\Math\float4x4.h"

#define TRANSFORMINPUTSWIDTH 50.0f

class ComponentTransform : public Component
{
public:

	ComponentTransform(GameObject* parent);
	ComponentTransform(const ComponentTransform& componentTransform, GameObject* parent);
	~ComponentTransform();

	void Update();

	void OnEditor();

	void OnUniqueEditor();

	void SavePrevTransform(const math::float4x4 & prevTransformMat);

	math::float4x4& GetMatrix() const;
	void SetMatrixFromGlobal(math::float4x4& globalMatrix, bool fromUI = false);

	math::float4x4 GetGlobalMatrix() const;

	void UpdateGlobal();

	void SetPosition(math::float3 newPos);
	void SetPosition(const float newPos[3]);
	void SetRotation(math::Quat newRot);
	void SetScale(math::float3 newScale);

	void Move(math::float3 distance);
	void Move(const float distance[3]);
	void Rotate(math::Quat rotation);
	void Scale(math::float3 scale);
	void Scale(float scale);

	math::float3 GetPosition() const;
	math::Quat GetRotation() const;
	math::float3 GetScale() const;



	uint GetInternalSerializationBytes();
	virtual void OnInternalSave(char*& cursor);
	virtual void OnInternalLoad(char*& cursor);

public:
	bool dragTransform = false;

private:
	math::float3 position = math::float3::zero;
	math::Quat rotation = math::Quat::identity;
	math::float3 scale = math::float3::one;

	math::float4x4 globalMatrix = math::float4x4::identity;

};

#endif