#ifndef __COMPONENT_RECTTRANSFORM_H__
#define __COMPONENT_RECTTRANSFORM_H__

#include "Component.h"

#include "Globals.h"

#include "MathGeoLib/include/Math/float3.h"

#define WORLDTORECT 100.0f

class ComponentTransform;

class ComponentRectTransform : public Component
{
public:
	enum Rect
	{
		X,
		Y,
		XDIST,
		YDIST,
		//Corners
		RTOPLEFT = 0,
		RTOPRIGHT,
		RBOTTOMLEFT,
		RBOTTOMRIGHT
	};

	enum Anchor
	{
		LEFT,
		TOP,
		RIGHT,
		BOTTOM
	};

	enum RectPercentage
	{
		X0,
		X1,
		Y0,
		Y1
	};

	enum RectPrivot
	{
		P_TOPLEFT,
		P_TOPRIGHT,
		P_BOTTOMLEFT,
		P_BOTTOMRIGHT,
		P_CENTER,
		P_TOP,
		P_LEFT,
		P_RIGHT,
		P_BOTTOM
	};

	enum RectFrom
	{
		RECT,
		WORLD,
		RECT_WORLD
	};

public:
	ComponentRectTransform(GameObject* parent, ComponentTypes componentType = ComponentTypes::RectTransformComponent, bool includeComponents = true);
	ComponentRectTransform(const ComponentRectTransform& componentRectTransform, GameObject* parent, bool includeComponents = true);
	~ComponentRectTransform();

	//NOTE: If you override this method, make sure to call the base class method. 
	//(Component::OnSystemEvent(event); at start)
	void OnSystemEvent(System_Event event);

	void Update();

	void OnEditor();

private:
	virtual uint GetInternalSerializationBytes();
	virtual void OnInternalSave(char*& cursor);
	virtual void OnInternalLoad(char*& cursor);
	void OnUniqueEditor();

public:
	void SetRect(int x, int y, int x_dist, int y_dist, bool fromAnim = false);
	void SetRect(int rect[4], bool fromAnim = false);

	int* GetRect();
	math::float3* GetCorners();
	void InitRect(); // Done, TODO J call methods of calculate anchors and percentage

	RectFrom GetFrom() const;
	
	void ParentChanged();
	void CanvasChanged();

	void FromLabel();

private:
	//From Sreen
	void CalculateScreenCorners();
	//From World
	void CalculateRectFromWorld();
	//From World Rect
	void CalculateCornersFromRect();

	void CalculateAnchors(bool needed_newPercentages = false); //TODO J Add the 7 pivots left
	void RecaculateAnchors(); //TODO J Add the 7 pivots left
	void CaculatePercentage();
	void RecalculateRectByPercentage();

private:
	bool isFromLabel = false;
	//Where get the info
	RectFrom rFrom = RectFrom::RECT;

	//Billboard for world
	bool billboard = false;

	//Recalculate at next frame
	bool needed_recalculate = false;
	bool rectTransform_modified = false;
	bool noUpdatefromCanvas = false;
	bool recalculate_byPercentage = false;

	//From Rect
	//x, y, x_dist, y_dist
	int rectTransform[4] = { 0, 0, 100, 100 };
	int lastPositionChange[2] = { 0,0 };//save when canvas change

	//FromWorld
	math::float3 corners[4] = { math::float3::zero, math::float3::zero, math::float3::zero, math::float3::zero };
		//Z fighting
	float z = 0.0f;

	//Pivot and anchor values
	bool usePivot = false;
	int anchor[4] = {0,0,0,0};
	RectPrivot pivot = RectPrivot::P_TOPLEFT;
	float anchor_percenatges[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	int center = 0;
};

#endif
