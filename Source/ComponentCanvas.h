#ifndef __COMPONENT_CANVAS_H__
#define __COMPONENT_CANVAS_H__

#include "Component.h"

#include "MathGeoLib/include/Math/float4x4.h"

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
	ComponentCanvas(GameObject* parent, ComponentTypes componentType = ComponentTypes::CanvasComponent, bool includeComponents = true);
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
	math::float4x4 GetGlobal() const;

	void ScreenChanged();
	void TransformUpdated();

private:
	CanvasType type = CanvasType::SCREEN;
	bool needed_change = false;
	bool first_iterate = true;
	class GameObject* fakeGo = nullptr;
	class ComponentTransform* transform = nullptr;
};

#endif