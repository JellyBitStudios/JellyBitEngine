#ifndef __COMPONENT_INTERPOLATION_H__
#define __COMPONENT_INTERPOLATION_H__

#include "Component.h"

#include "Globals.h"

#include "MathGeoLib/include/MathGeoLib.h"

#include "Timer.h"
#include <list>

struct TransNode
{
	TransNode() {}
	TransNode(TransNode* node)
	{
		name = node->name;
		position = node->position;
		rotation = node->rotation;
		scale = node->scale;
		distance = node->distance;
	}

	TransNode(char*& cursor)
	{
		size_t bytes = sizeof(uint);
		uint nameLenght;
		memcpy(&nameLenght, cursor, bytes);
		cursor += bytes;
		
		bytes = nameLenght * sizeof(char);
		name.resize(nameLenght);
		memcpy((void*)name.c_str(), cursor, bytes);
		name.resize(nameLenght);
		cursor += bytes;

		bytes = sizeof(position);
		memcpy(&position, cursor, bytes);
		cursor += bytes;

		bytes = sizeof(rotation);
		memcpy(&rotation, cursor, bytes);
		cursor += bytes;

		bytes = sizeof(scale);
		memcpy(&scale, cursor, bytes);
		cursor += bytes;

		bytes = sizeof(distance);
		memcpy(&distance, cursor, bytes);
		cursor += bytes;
	}

	std::string name = "No name";

	math::float3 position = math::float3::zero;
	math::Quat rotation = math::Quat::identity;
	math::float3 scale = math::float3::one;

	float distance = 0;

	uint Size()
	{
		return name.size() * sizeof(char) + sizeof(uint) + sizeof(position) + sizeof(rotation) + sizeof(scale) + sizeof(distance);
	}

	void Save(char* &cursor)
	{
		size_t bytes = sizeof(uint);
		uint nameLenght = name.size();
		memcpy(cursor, &nameLenght, bytes);
		cursor += bytes;
		
		bytes = nameLenght * sizeof(char);
		memcpy(cursor, name.c_str(), bytes);
		cursor += bytes;

		bytes = sizeof(position);
		memcpy(cursor, &position, bytes);
		cursor += bytes;

		bytes = sizeof(rotation);
		memcpy(cursor, &rotation, bytes);
		cursor += bytes;

		bytes = sizeof(scale);
		memcpy(cursor, &scale, bytes);
		cursor += bytes;

		bytes = sizeof(distance);
		memcpy(cursor, &distance, bytes);
		cursor += bytes;
	}
};

class ComponentInterpolation : public Component
{
public:

	ComponentInterpolation(GameObject* parent);
	ComponentInterpolation(const ComponentInterpolation& componentTransform, GameObject* parent, bool includeComponents);
	~ComponentInterpolation();

	void Update();

	void OnUniqueEditor();

	void StartInterpolation(char* nodeName, bool goBack, float time = 0.0f);
	void GoBack();


	//Serialization
	uint GetNodesBytes();
	void SaveNodes(char *& cursor);
	void LoadNodes(char *& cursor, int size);

	uint GetInternalSerializationBytes();
	virtual void OnInternalSave(char*& cursor);
	virtual void OnInternalLoad(char*& cursor);

public:

	std::list<TransNode*> nodes;

	float speed = 10.0f;
	float minDist = 1.0f;
private:

	bool move = false;
	TransNode currentNode;
	TransNode startPoint;
	float currTime = 0;

	bool goBack = false;
	float waitTime = 0;

	Timer goBackTime;

	bool finished = false;
	bool goingBack = false;
};

#endif //! __COMPONENT_INTERPOLATION_H__