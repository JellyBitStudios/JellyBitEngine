#ifndef __COMPONENT_RECTTRANSFORM_H__
#define __COMPONENT_RECTTRANSFORM_H__

#define LEFT_RECT_STR "Left"
#define TOP_RECT_STR "Top"
#define RIGHT_RECT_STR "Right"
#define BOTTOM_RECT_STR "Bottom"

#define ANCHORS_POINTS_STR "Begin\0End"

#include "Component.h"

#include "Globals.h"

#include "MathGeoLib/include/Math/float3.h"

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
		TOPLEFT,
		BOTTOMRIGHT
	};

	enum RectFrom
	{
		RECT,
		WORLD,
		RECT_WORLD
	};

public:
	ComponentRectTransform(GameObject* parent, ComponentTypes componentType = ComponentTypes::RectTransformComponent, RectFrom rF = RectFrom::RECT);
	ComponentRectTransform(const ComponentRectTransform& componentRectTransform, GameObject* parent, bool includeComponents = true);
	~ComponentRectTransform();

	void Update();

	void OnEditor();

	void SetRect(uint x, uint y, uint x_dist, uint y_dist);

	uint* GetRect();
	math::float3* GetCorners();
	void CheckParentRect();

	void ChangeChildsRect(bool its_me = false, bool size_changed = false);

	RectFrom GetFrom() const;

private:

	RectFrom rFrom = RectFrom::RECT;

	//From Rect
	//x, y, x_dist, y_dist
	uint rectTransform[4] = { 0, 0, 100, 100 };
	uint* ui_rect = nullptr;
	uint* rectParent = nullptr;

	//FromWorld
	ComponentTransform* transformParent = nullptr;
	math::float3 corners[4] = { math::float3::zero, math::float3::zero, math::float3::zero, math::float3::zero };
	math::float3* parentCorners = nullptr;
	float z = 0.0f;

	//True, references top-left. False, refernces bottom-right. For every point of rect.
	bool use_margin = false;
	uint anchor[4] = {0,0,0,0};
	bool anchor_flags[4] = { false, false, false, false };
	float anchor_percenatges[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

private:
	void CalculateRectFromWorld();
	void CalculateCornersFromRect();

	void RecaculateAnchors();
	void RecaculateAnchors(int type);
	void RecaculatePercentage();

	void ParentChanged(bool size_changed = false);
	void UseMarginChanged(bool useMargin);

	virtual uint GetInternalSerializationBytes();
	virtual void OnInternalSave(char*& cursor);
	virtual void OnInternalLoad(char*& cursor);
	void OnUniqueEditor();

	float GetZ() const;
};

#endif