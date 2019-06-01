#ifndef __COMPONENT_CANVAS_H__
#define __COMPONENT_CANVAS_H__

#include "Component.h"

#include "MathGeoLib/include/Math/float4x4.h"

class ComponentCanvas : public Component
{
public:
	enum CanvasType
	{
		CNULL = -1,
		SCREEN,
		WORLD = 2,
	};

public:
	ComponentCanvas(GameObject* parent, ComponentTypes componentType = ComponentTypes::CanvasComponent, bool includeComponents = true);
	ComponentCanvas(const ComponentCanvas& componentCanvas, GameObject* parent, bool includeComponents = true);
	~ComponentCanvas();

	//NOTE: If you override this method, make sure to call the base class method. 
	//(Component::OnSystemEvent(event); at start)
	void OnSystemEvent(System_Event event);

	void Update();

private:
	virtual uint GetInternalSerializationBytes();
	virtual void OnInternalSave(char*& cursor);
	virtual void OnInternalLoad(char*& cursor);

	virtual void OnEditor();

public: //Custom
	void Change(CanvasType to);
	CanvasType GetType()const;
	math::float4x4 GetGlobal() const;
	float GetZ(GameObject* go, ComponentTypes type);
private:
	CanvasType type = CanvasType::SCREEN;
	bool needed_change = false;
	bool first_iterate = true;
};

#endif