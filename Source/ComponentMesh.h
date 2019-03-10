#ifndef __COMPONENT_MESH_H__
#define __COMPONENT_MESH_H__

#include "Component.h"
#include "Globals.h"

#include <vector>

class ComponentMesh : public Component
{
public:

	ComponentMesh(GameObject* parent);
	ComponentMesh(const ComponentMesh& componentMesh, GameObject* parent, bool include = true);
	~ComponentMesh();

	void Update();

	void SetResource(uint res_uuid);

	void OnUniqueEditor();

	uint GetInternalSerializationBytes();
	virtual void OnInternalSave(char*& cursor);
	virtual void OnInternalLoad(char*& cursor);

public:

	uint res = 0;

	// Pathfinding
	bool nv_walkable = true;

	// Animation
	std::vector<uint> bonesUuids; // ComponentBone uuid
};

#endif