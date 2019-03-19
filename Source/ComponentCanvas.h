#ifndef __COMPONENT_CANVAS_H__
#define __COMPONENT_CANVAS_H__

#include "Component.h"

class ComponentCanvas : public Component
{
public:
	enum CanvasType
	{
		SCREEN,
		WORLD_SCREEN,
		WORLD
	};

public:
	ComponentCanvas(GameObject* parent, ComponentTypes componentType = ComponentTypes::CanvasComponent);
	ComponentCanvas(const ComponentCanvas& componentCanvas, GameObject* parent, bool includeComponents = true);
	~ComponentCanvas();

	void Update();

private:
	virtual uint GetInternalSerializationBytes();
	virtual void OnInternalSave(char*& cursor);
	virtual void OnInternalLoad(char*& cursor);

	virtual void OnEditor();

public: //Custom
	CanvasType GetType()const;

private:
	CanvasType type = CanvasType::SCREEN;
	bool needed_change = false;

	class GameObject* fakeGo = nullptr;
	class ComponentTransform* transform = nullptr;
};

#endif