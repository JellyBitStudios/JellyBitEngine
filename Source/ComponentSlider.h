#ifndef __COMPONENT_SLIDER_H__
#define __COMPONENT_SLIDER_H__

#include "GLCache.h"

#include "Component.h"

#include "MathGeoLib/include/Math/float4.h"

class ComponentSlider : public Component
{
public:
	enum SliderType {
		BarMask,
	};
public:
	ComponentSlider(GameObject * parent, bool includeComponents = true);
	ComponentSlider(const ComponentSlider & ComponentSlider, GameObject* parent, bool includeComponents = true);

	~ComponentSlider();

	//NOTE: If you override this method, make sure to call the base class method. 
//(Component::OnSystemEvent(event); at start)
	void OnSystemEvent(System_Event event);

	void Update();

public:
	void CalculateFrontSizeByPercentage();
	void CalculatePercentageByFrontSize();
	void CalculateCorners();

	//Buffer
	int GetBackBufferIndex()const;
	int GetFrontBufferIndex()const;
	void SetBufferRangeAndFIll(uint offs, int index);

	math::float3* GetCorners();
	uint GetBackTexture()const;
	uint GetFrontTexture()const;

	void SetFromInvadilateResource(uint uuid);

	//Scripting
	float GetPercentage()const;
	void SetPercentage(float i);

	//ignoreMouse
	void SetIgnoreMouse(bool ignore);
	bool GetIgnoreMouse()const;

private:
	virtual uint GetInternalSerializationBytes();
	virtual void OnInternalSave(char*& cursor);
	virtual void OnInternalLoad(char*& cursor);
	void OnUniqueEditor();

private:
	void FillBuffers();
	void CalculateCornersFromScreen();
	void CalculateCornersFromWorld();

private:
	SliderType sType = SliderType::BarMask;
	//calcs
	float percentage = 1;
	int actualSize = 0;
	int referenceSize = 0;
	math::float3 corners[8];

	//Booleans
	bool ignoreMouse = false;
	bool currentOnClick = false;

	//Textures
	uint backTexture = 0u;
	uint frontTexture = 0u;

	//Buffer BackTexture
	int indexB = -1;
	uint offsetB = 0;
	int indexF = -1;
	uint offsetF = 0;
	char buffer[UI_BYTES_RECT];

	bool needed_fillBuffer = false;
};

#endif