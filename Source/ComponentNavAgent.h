#ifndef __COMPONENT_NAV_AGENT_H__
#define __COMPONENT_NAV_AGENT_H__

#include "Component.h"

class ComponentNavAgent : public Component
{
public:

	ComponentNavAgent(GameObject* parent);
	ComponentNavAgent(const ComponentNavAgent& componentTransform, GameObject* parent, bool include = true);
	~ComponentNavAgent();

	void Update();

	void OnUniqueEditor();

	void AddAgent();
	void RemoveAgent();
	int  GetIndex() const { return index; }
	bool UpdateParams() const;
	void SetDestination(const float* pos) const;
	bool IsWalking() const;
	void RequestMoveVelocity(float* dir) const;
	void ResetMoveTarget() const;

	uint GetInternalSerializationBytes();
	virtual void OnInternalSave(char*& cursor);
	virtual void OnInternalLoad(char*& cursor);

public:
	float radius = 1.0f;
	float height = 1.0f;
	float maxAcceleration = 8.0f;
	float maxSpeed = 3.5f;
	unsigned int params = 0;
	float separationWeight = 2.0f;
	int avoidanceQuality = 3;
	float stopAtLength = 0.3f;

	int index = -1;
};

#endif