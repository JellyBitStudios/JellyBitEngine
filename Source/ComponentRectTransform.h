#ifndef __COMPONENT_RECTTRANSFORM_H__
#define __COMPONENT_RECTTRANSFORM_H__

#include "Component.h"

#include "Globals.h"

#include "MathGeoLib/include/Math/float3.h"

#define WORLDTORECT 100.0f
#define ZSEPARATOR 0.005f

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

	void Update();

	void OnEditor();

private:
	virtual uint GetInternalSerializationBytes();
	virtual void OnInternalSave(char*& cursor);
	virtual void OnInternalLoad(char*& cursor);
	void OnUniqueEditor(); //Todo J Add the 7 pivots left

public:
	void SetRect(uint x, uint y, uint x_dist, uint y_dist);
	void SetRectPos(uint x, uint y);
	void SetRectDim(uint x_dist, uint y_dist);

	uint* GetRect();
	math::float3* GetCorners();
	void InitRect(); // Done, TODO J call methods of calculate anchors and percentage

	void RecalculateAndChilds();

	RectFrom GetFrom() const;

	bool IsInRect(uint* rect);
	
	void ScreenChanged();

	void TransformUpdated();
	void ParentChanged(bool canvas_changed = false);
	void CanvasChanged();
	void WorkSpaceChanged(uint diff, bool to);

	float GetZ() const;

private:
	//From World
	void CalculateRectFromWorld();
	//From World Rect
	void CalculateCornersFromRect();

	void CalculateAnchors(bool needed_newPercentages = false); //TODO J Add the 7 pivots left
	void RecaculateAnchors(); //TODO J Add the 7 pivots left
	void RecaculatePercentage();
	void RecalculateRectByPercentage();

private:
	//Where get the info
	RectFrom rFrom = RectFrom::RECT;

	//Billboard for world
	bool billboard = false;

	//Recalculate at next frame
	bool needed_recalculate = false;
	bool rectTransform_modified = false;
	bool noUpdatefromCanvas = false;

	//From Rect
	//x, y, x_dist, y_dist
	uint rectTransform[4] = { 0, 0, 100, 100 };
	uint lastPositionChange[2] = { 0,0 };//save when canvas change

	//FromWorld
	math::float3 corners[4] = { math::float3::zero, math::float3::zero, math::float3::zero, math::float3::zero };
		//Z fighting
	float z = 0.0f;

	//Pivot and anchor values
	bool usePivot = false;
	uint anchor[4] = {0,0,0,0};
	RectPrivot pivot = RectPrivot::P_TOPLEFT;
	float anchor_percenatges[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
};

#endif
