#ifndef __COMPONENT_INTERPOLATION_H__
#define __COMPONENT_INTERPOLATION_H__

#include "Component.h"

#include "Globals.h"

#include "MathGeoLib/include/MathGeoLib.h"

#include "Timer.h"
#include <list>

struct TransNode
{
	std::string name = "No name";

	math::float3 position = math::float3::zero;
	math::Quat rotation = math::Quat::identity;
	math::float3 scale = math::float3::one;
};

class ComponentInterpolation : public Component
{
public:

	ComponentInterpolation(GameObject* parent);
	ComponentInterpolation(const ComponentInterpolation& componentTransform, GameObject* parent);
	~ComponentInterpolation();

	void Update();

	void OnUniqueEditor();

	//Serialization
	uint GetInternalSerializationBytes();
	virtual void OnInternalSave(char*& cursor);
	virtual void OnInternalLoad(char*& cursor);

public:

	std::list<TransNode*> nodes;

private:

};

#endif //! __COMPONENT_INTERPOLATION_H__