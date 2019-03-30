#include "ComponentRectTransform.h"

#include "Application.h"
#include "ModuleUI.h"
#include "ModuleRenderer3D.h"

#include "GameObject.h"
#include "ComponentCanvas.h"
#include "ComponentTransform.h"
#include "ComponentCamera.h"
#include "ComponentImage.h"

#include "imgui\imgui.h"
#include "imgui\imgui_internal.h"

#define WORLDTORECT 100.0f
#define ZSEPARATOR 0.005f

#define PIVOT_POINTS_STR "Top Left\0Top Right\0Bottom Left\0Bottom Right\0Center\0Top\0Left\0Right\0Bottom"


ComponentRectTransform::ComponentRectTransform(GameObject * parent, ComponentTypes componentType, bool includeComponents) : Component(parent, ComponentTypes::RectTransformComponent)
{
	if (includeComponents)
	{
		if (parent->GetParent())
		{
			GameObject* goCanvas = ModuleUI::FindCanvas(parent);
			if (goCanvas)
			{
				ComponentCanvas* canvas = goCanvas->cmp_canvas;

				switch (canvas->GetType())
				{
					case ComponentCanvas::CanvasType::SCREEN:
						rFrom = RectFrom::RECT;
						break;
					case ComponentCanvas::CanvasType::WORLD_SCREEN:
					case ComponentCanvas::CanvasType::WORLD:
						if (canvas == parent->cmp_canvas)
							rFrom = RectFrom::WORLD;
						else
							rFrom = RectFrom::RECT_WORLD;
						break;
				}
			}

			InitRect();
		}
	}
}

ComponentRectTransform::ComponentRectTransform(const ComponentRectTransform & componentRectTransform, GameObject* parent, bool includeComponents) : Component(parent, ComponentTypes::RectTransformComponent)
{
	rFrom = componentRectTransform.rFrom;
	pivot = componentRectTransform.pivot;
	usePivot = componentRectTransform.usePivot;
	billboard = componentRectTransform.billboard;
	z = componentRectTransform.z;
	center = componentRectTransform.center;

	memcpy(rectTransform, componentRectTransform.rectTransform, sizeof(uint) * 4);
	memcpy(anchor, componentRectTransform.anchor, sizeof(uint) * 4);
	memcpy(lastPositionChange, componentRectTransform.lastPositionChange, sizeof(uint) * 2);
	memcpy(corners, componentRectTransform.corners, sizeof(math::float3) * 4);
	memcpy(anchor_percenatges, componentRectTransform.anchor_percenatges, sizeof(float) * 4);

	noUpdatefromCanvas = true;
}

ComponentRectTransform::~ComponentRectTransform()
{
	parent->cmp_rectTransform = nullptr;
}

void ComponentRectTransform::Update()
{
	if (rFrom == RectFrom::WORLD)
	{
		CalculateRectFromWorld();
		needed_recalculate = false;
	}

	if (needed_recalculate)
	{
		switch (rFrom)
		{
			case ComponentRectTransform::RECT:
				(rectTransform_modified) ? CalculateAnchors(true) :
					((usePivot) ? RecaculateAnchors() : RecalculateRectByPercentage());
				break;
			case ComponentRectTransform::WORLD:
				CalculateRectFromWorld();
				break;
			case ComponentRectTransform::RECT_WORLD:
				(rectTransform_modified) ? CalculateAnchors(true) :
					((usePivot) ? RecaculateAnchors() : RecalculateRectByPercentage());
				CalculateCornersFromRect();
				break;
		}

		needed_recalculate = false;
		rectTransform_modified = false;
	}
}

void ComponentRectTransform::OnEditor()
{
	OnUniqueEditor();
}

void ComponentRectTransform::SetRect(uint x, uint y, uint x_dist, uint y_dist)
{
	if (rFrom != ComponentRectTransform::WORLD)
	{
		rectTransform[Rect::X] = x;
		rectTransform[Rect::Y] = y;
		rectTransform[Rect::XDIST] = x_dist;
		rectTransform[Rect::YDIST] = y_dist;

		RecalculateAndChilds();
	}
}

uint* ComponentRectTransform::GetRect()
{
	return rectTransform;
}

math::float3 * ComponentRectTransform::GetCorners()
{
	return corners;
}

void ComponentRectTransform::InitRect()
{
	switch (rFrom)
	{
		case ComponentRectTransform::RECT:
		{
			uint* rectParent = nullptr;
			if (parent->cmp_canvas)
				rectParent = App->ui->GetRectUI();
			else
				rectParent = parent->GetParent()->cmp_rectTransform->GetRect();

			rectTransform[Rect::X] = rectParent[Rect::X];
			rectTransform[Rect::Y] = rectParent[Rect::Y];

			if (rectParent[Rect::XDIST] < rectTransform[Rect::XDIST])
				rectTransform[Rect::XDIST] = rectParent[Rect::XDIST];
			if (rectParent[Rect::YDIST] < rectTransform[Rect::YDIST])
				rectTransform[Rect::YDIST] = rectParent[Rect::YDIST];

			CalculateAnchors(true);
			break;
		}
		case ComponentRectTransform::WORLD:
		{
			CalculateRectFromWorld();
			break;
		}
		case ComponentRectTransform::RECT_WORLD:
		{
			ComponentRectTransform* rTParent = parent->GetParent()->cmp_rectTransform;
			uint* rectParent = rTParent->GetRect();

			z = rTParent->GetZ() + ZSEPARATOR;
			if (rectTransform[Rect::XDIST] > rectParent[Rect::XDIST])
				rectTransform[Rect::XDIST] = rectParent[Rect::XDIST];
			if (rectTransform[Rect::YDIST] > rectParent[Rect::YDIST])
				rectTransform[Rect::YDIST] = rectParent[Rect::YDIST];

			CalculateCornersFromRect();
			CalculateAnchors(true);
			break;
		}
	}
}

void ComponentRectTransform::RecalculateAndChilds()
{
	needed_recalculate = true;
	std::vector<GameObject*> childs;
	parent->GetChildrenVector(childs, false);
	for (GameObject* c_go : childs)
		c_go->cmp_rectTransform->RecalculateAndChilds();
}

ComponentRectTransform::RectFrom ComponentRectTransform::GetFrom() const
{
	return rFrom;
}

void ComponentRectTransform::ScreenChanged()
{
	RecalculateRectByPercentage();
}

void ComponentRectTransform::TransformUpdated()
{
	needed_recalculate = true;
}

void ComponentRectTransform::ParentChanged(bool canvas_changed)
{
	if (!parent->cmp_canvas)
	{
		GameObject* gCanvas = ModuleUI::FindCanvas(parent);
		if (gCanvas->cmp_canvas->GetType() == ComponentCanvas::CanvasType::SCREEN && rFrom == RectFrom::RECT_WORLD)
			rFrom = RectFrom::RECT;
		else if (gCanvas->cmp_canvas->GetType() != ComponentCanvas::CanvasType::SCREEN && rFrom == RectFrom::RECT)
			rFrom = RectFrom::RECT_WORLD;

		if (!canvas_changed)
			RecalculateAndChilds();
	}
}

void ComponentRectTransform::CanvasChanged()
{
	if (parent->cmp_canvas)
	{
		if (!noUpdatefromCanvas)
		{
			switch (parent->cmp_canvas->GetType())
			{
				case ComponentCanvas::CanvasType::SCREEN:
				{
					rFrom = RectFrom::RECT;
					math::float3 scale = parent->transform->GetScale();

					rectTransform[Rect::XDIST] = (uint)(scale.x * WORLDTORECT);
					rectTransform[Rect::YDIST] = (uint)(scale.y * WORLDTORECT);

					rectTransform[Rect::X] = lastPositionChange[0];
					rectTransform[Rect::Y] = lastPositionChange[1];

					rectTransform_modified = true;
					break;
				}
				case ComponentCanvas::CanvasType::WORLD_SCREEN:
				case ComponentCanvas::CanvasType::WORLD:
				{
					rFrom = RectFrom::WORLD;

					math::float3 scale = parent->transform->GetScale();
					parent->transform->SetScale({ (float)rectTransform[Rect::XDIST] / WORLDTORECT, (float)rectTransform[Rect::YDIST] / WORLDTORECT, scale.z });

					lastPositionChange[0] = rectTransform[Rect::X];
					lastPositionChange[1] = rectTransform[Rect::Y];
					break;
				}
			}

			std::vector<GameObject*> childs;
			parent->GetChildrenAndThisVectorFromLeaf(childs);
			for (GameObject* rectGo : childs) if (rectGo->cmp_rectTransform) rectGo->cmp_rectTransform->ParentChanged(true);

			RecalculateAndChilds();
		}

		noUpdatefromCanvas = false;
	}
}

void ComponentRectTransform::WorkSpaceChanged(uint diff, bool to)
{
	if (rFrom == RectFrom::RECT)
	{
		if (parent->cmp_canvas)
			(to) ? (rectTransform[Rect::X] += diff) : (rectTransform[Rect::X] -= diff);
		RecalculateAndChilds();
	}
}

void ComponentRectTransform::RecalculateRectByPercentage()
{
	uint* rectParent = nullptr;
	if (parent->cmp_canvas)
		rectParent = App->ui->GetRectUI();
	else
		rectParent = parent->GetParent()->cmp_rectTransform->GetRect();

	rectTransform[Rect::X] = (uint)(anchor_percenatges[RectPercentage::X0] * (float)rectParent[Rect::XDIST]) + rectParent[Rect::X];
	rectTransform[Rect::XDIST] = rectParent[Rect::XDIST] - (((rectTransform[Rect::X] - rectParent[Rect::X]) + (uint)(anchor_percenatges[RectPercentage::X1] * (float)rectParent[Rect::XDIST])));
	rectTransform[Rect::Y] = (uint)(anchor_percenatges[RectPercentage::Y0] * (float)rectParent[Rect::YDIST]) + rectParent[Rect::Y];
	rectTransform[Rect::YDIST] = rectParent[Rect::YDIST] - ((rectTransform[Rect::Y] - rectParent[Rect::Y]) + (uint)(anchor_percenatges[RectPercentage::Y1] * (float)rectParent[Rect::YDIST]));

	CalculateAnchors();
}

void ComponentRectTransform::CalculateRectFromWorld()
{
	if (billboard)
	{
		math::float3 zAxis = App->renderer3D->GetCurrentCamera()->frustum.front;
		math::float3 yAxis = App->renderer3D->GetCurrentCamera()->frustum.up;
		math::float3 xAxis = yAxis.Cross(zAxis).Normalized();

		math::float3 pos = math::float3::zero;
		math::Quat rot = math::Quat::identity;
		math::float3 scale = math::float3::zero;
		math::float4x4 global = parent->transform->GetGlobalMatrix();
		global.Decompose(pos, rot, scale);

		if (!scale.IsFinite())
		{
			scale = math::float3::one;
			CONSOLE_LOG(LogTypes::Warning, "If canvas use billboard, you can't set size of transform to 0. Reset scale..")
		}

		parent->transform->SetMatrixFromGlobal(global.FromTRS(pos, math::Quat::identity * math::float3x3(xAxis, yAxis, zAxis).ToQuat(), scale));
	}

	math::float4x4 globalmatrix = parent->transform->GetGlobalMatrix();
	corners[Rect::RTOPLEFT] = math::float4(globalmatrix * math::float4(-0.5f, 0.5f, 0.0f, 1.0f)).Float3Part();
	corners[Rect::RTOPRIGHT] = math::float4(globalmatrix * math::float4(0.5f, 0.5f, 0.0f, 1.0f)).Float3Part();
	corners[Rect::RBOTTOMLEFT] = math::float4(globalmatrix * math::float4(-0.5f, -0.5f, 0.0f, 1.0f)).Float3Part();
	corners[Rect::RBOTTOMRIGHT] = math::float4(globalmatrix * math::float4(0.5f, -0.5f, 0.0f, 1.0f)).Float3Part();

	rectTransform[Rect::X] = 0;
	rectTransform[Rect::Y] = 0;
	rectTransform[Rect::XDIST] = abs(math::Distance(corners[Rect::RTOPRIGHT], corners[Rect::RTOPLEFT])) * WORLDTORECT;
	rectTransform[Rect::YDIST] = abs(math::Distance(corners[Rect::RBOTTOMLEFT], corners[Rect::RTOPLEFT])) * WORLDTORECT;

	//Mask calcs
	if (parent->cmp_image)
		parent->cmp_image->RectChanged();

	RecalculateAndChilds();
}

void ComponentRectTransform::CalculateCornersFromRect()
{
	math::float3* parentCorners = parent->GetParent()->cmp_rectTransform->GetCorners();
	uint* rectParent = parent->GetParent()->cmp_rectTransform->GetRect();

	math::float3 xDirection = (parentCorners[Rect::RTOPLEFT] - parentCorners[Rect::RTOPRIGHT]).Normalized();
	math::float3 yDirection = (parentCorners[Rect::RBOTTOMLEFT] - parentCorners[Rect::RTOPLEFT]).Normalized();

	corners[Rect::RTOPRIGHT] = parentCorners[Rect::RTOPRIGHT] + (xDirection * ((float)(rectTransform[Rect::X] - rectParent[Rect::X]) / WORLDTORECT)) + (yDirection * ((float)(rectTransform[Rect::Y] - rectParent[Rect::Y]) / WORLDTORECT));
	corners[Rect::RTOPLEFT] = corners[Rect::RTOPRIGHT] + (xDirection * ((float)rectTransform[Rect::XDIST] / WORLDTORECT));
	corners[Rect::RBOTTOMLEFT] = corners[Rect::RTOPLEFT] + (yDirection * ((float)rectTransform[Rect::YDIST] / WORLDTORECT));
	corners[Rect::RBOTTOMRIGHT] = corners[Rect::RBOTTOMLEFT] - (xDirection * ((float)rectTransform[Rect::XDIST] / WORLDTORECT));

	math::float3 zDirection = xDirection.Cross(yDirection);

	corners[Rect::RTOPRIGHT] -= zDirection * z;
	corners[Rect::RTOPLEFT] -= zDirection * z;
	corners[Rect::RBOTTOMLEFT] -= zDirection * z;
	corners[Rect::RBOTTOMRIGHT] -= zDirection * z;
}

void ComponentRectTransform::CalculateAnchors(bool needed_newPercentages)
{
	uint* rectParent = nullptr;
	if (parent->cmp_canvas)
		rectParent = App->ui->GetRectUI();
	else
		rectParent = parent->GetParent()->cmp_rectTransform->GetRect();

	//Mask calcs
	if (parent->cmp_image)
		parent->cmp_image->RectChanged();

	switch (pivot)
	{
		case ComponentRectTransform::P_TOPLEFT:
		{
			anchor[Anchor::LEFT] = rectTransform[Rect::X] - rectParent[Rect::X];
			anchor[Anchor::TOP] = rectTransform[Rect::Y] - rectParent[Rect::Y];
			anchor[Anchor::RIGHT] = (rectTransform[Rect::X] + rectTransform[Rect::XDIST]) - rectParent[Rect::X];
			anchor[Anchor::BOTTOM] = (rectTransform[Rect::Y] + rectTransform[Rect::YDIST]) - rectParent[Rect::Y];
			break;
		}
		case ComponentRectTransform::P_TOPRIGHT:
		{
			anchor[Anchor::LEFT] = (rectParent[Rect::X] + rectParent[Rect::XDIST]) - rectTransform[Rect::X];
			anchor[Anchor::TOP] = rectTransform[Rect::Y] - rectParent[Rect::Y];
			anchor[Anchor::RIGHT] = (rectParent[Rect::X] + rectParent[Rect::XDIST]) - (rectTransform[Rect::X] + rectTransform[Rect::XDIST]);
			anchor[Anchor::BOTTOM] = (rectTransform[Rect::Y] + rectTransform[Rect::YDIST]) - rectParent[Rect::Y];
			break;
		}
		case ComponentRectTransform::P_BOTTOMLEFT:
		{
			anchor[Anchor::LEFT] = rectTransform[Rect::X] - rectParent[Rect::X];
			anchor[Anchor::TOP] = (rectParent[Rect::Y] + rectParent[Rect::YDIST]) - rectTransform[Rect::Y];
			anchor[Anchor::RIGHT] = rectParent[Rect::X] + anchor[Anchor::LEFT] + rectTransform[Rect::XDIST];
			anchor[Anchor::BOTTOM] = (rectParent[Rect::Y] + rectParent[Rect::YDIST]) - (rectTransform[Rect::Y] + rectTransform[Rect::YDIST]);
			break;
		}
		case ComponentRectTransform::P_BOTTOMRIGHT:
		{
			anchor[Anchor::LEFT] = (rectParent[Rect::X] + rectParent[Rect::XDIST]) - rectTransform[Rect::X];
			anchor[Anchor::TOP] = (rectParent[Rect::Y] + rectParent[Rect::YDIST]) - rectTransform[Rect::Y];
			anchor[Anchor::RIGHT] = (rectParent[Rect::X] + rectParent[Rect::XDIST]) - (rectTransform[Rect::X] + rectTransform[Rect::XDIST]);
			anchor[Anchor::BOTTOM] = (rectParent[Rect::Y] + rectParent[Rect::YDIST]) - (rectTransform[Rect::Y] + rectTransform[Rect::YDIST]);
			break;
		}
		case ComponentRectTransform::P_TOP:
		{
			uint pCenter = rectParent[Rect::X] + (rectParent[Rect::XDIST] / 2) + center;
			anchor[Anchor::LEFT] = anchor[Anchor::RIGHT] = (rectTransform[Rect::XDIST] / 2);
			rectTransform[Rect::X] = pCenter - anchor[Anchor::LEFT];
			anchor[Anchor::TOP] = rectTransform[Rect::Y] - rectParent[Rect::Y];
			anchor[Anchor::BOTTOM] = anchor[Anchor::TOP] + rectTransform[Rect::YDIST];
			break;
		}
		case ComponentRectTransform::P_LEFT:
		{
			break;
		}
		case ComponentRectTransform::P_RIGHT:
		{
			break;
		}
		case ComponentRectTransform::P_BOTTOM:
		{
			uint pCenter = rectParent[Rect::X] + (rectParent[Rect::XDIST] / 2) + center;
			rectTransform[Rect::X] = pCenter - (rectTransform[Rect::XDIST] / 2);
			anchor[Anchor::BOTTOM] = (rectParent[Rect::Y] + rectParent[Rect::YDIST]) - (rectTransform[Rect::Y] + rectTransform[Rect::YDIST]);
			anchor[Anchor::TOP] = (rectParent[Rect::Y] + rectParent[Rect::YDIST]) - rectTransform[Rect::Y];
			anchor[Anchor::LEFT] = anchor[Anchor::RIGHT] = (rectTransform[Rect::XDIST] / 2);
			break;
		}
		case ComponentRectTransform::P_CENTER:
		{
			uint pxCenter = rectParent[Rect::X] + (rectParent[Rect::XDIST] / 2);
			uint pyCenter = rectParent[Rect::Y] + (rectParent[Rect::YDIST] / 2);
			anchor[Anchor::TOP] = anchor[Anchor::BOTTOM] = (rectTransform[Rect::YDIST] / 2);
			anchor[Anchor::LEFT] = anchor[Anchor::RIGHT] = (rectTransform[Rect::XDIST] / 2);
			rectTransform[Rect::X] = pxCenter - anchor[Anchor::LEFT];
			rectTransform[Rect::Y] = pyCenter - anchor[Anchor::TOP];
			break;
		}
	}
	if (needed_newPercentages)
		RecaculatePercentage();
}

void ComponentRectTransform::RecaculateAnchors()
{
	uint* rectParent = nullptr;
	if (parent->cmp_canvas)
		rectParent = App->ui->GetRectUI();
	else
		rectParent = parent->GetParent()->cmp_rectTransform->GetRect();

	switch (pivot)
	{
		case ComponentRectTransform::P_TOPLEFT:
		{
			rectTransform[Rect::X] = anchor[Anchor::LEFT] + rectParent[Rect::X];
			rectTransform[Rect::Y] = anchor[Anchor::TOP] + rectParent[Rect::Y];
			anchor[Anchor::RIGHT] = anchor[Anchor::LEFT] + rectTransform[Rect::XDIST];
			anchor[Anchor::BOTTOM] = anchor[Anchor::TOP] + rectTransform[Rect::YDIST];
			break;
		}
		case ComponentRectTransform::P_TOPRIGHT:
		{
			rectTransform[Rect::X] = (rectParent[Rect::X] + rectParent[Rect::XDIST]) - anchor[Anchor::RIGHT] - rectTransform[Rect::XDIST];
			rectTransform[Rect::Y] = rectParent[Rect::Y] + anchor[Anchor::TOP];
			anchor[Anchor::BOTTOM] = anchor[Anchor::TOP] + rectTransform[Rect::YDIST];
			anchor[Anchor::LEFT] = anchor[Anchor::RIGHT] + rectTransform[Rect::XDIST];
			break;
		}
		case ComponentRectTransform::P_BOTTOMLEFT:
		{
			rectTransform[Rect::X] = rectParent[Rect::X] + anchor[Anchor::LEFT];
			rectTransform[Rect::Y] = (rectParent[Rect::Y] + rectParent[Rect::YDIST]) - (anchor[Anchor::BOTTOM] + rectTransform[Rect::YDIST]);
			anchor[Anchor::TOP] = anchor[Anchor::BOTTOM] + rectTransform[Rect::YDIST];
			anchor[Anchor::RIGHT] = anchor[Anchor::LEFT] + rectTransform[Rect::XDIST];
			break;
		}
		case ComponentRectTransform::P_BOTTOMRIGHT:
		{
			rectTransform[Rect::X] = (rectParent[Rect::X] + rectParent[Rect::XDIST]) - anchor[Anchor::RIGHT] - rectTransform[Rect::XDIST];
			rectTransform[Rect::Y] = (rectParent[Rect::Y] + rectParent[Rect::YDIST]) - anchor[Anchor::BOTTOM] - rectTransform[Rect::YDIST];
			anchor[Anchor::LEFT] = (rectTransform[Rect::X] + rectTransform[Rect::XDIST]) + anchor[Anchor::RIGHT];
			anchor[Anchor::TOP] = (rectTransform[Rect::Y] + rectTransform[Rect::YDIST]) + anchor[Anchor::BOTTOM];
			break;
		}
		case ComponentRectTransform::P_TOP:
		{
			uint pCenter = rectParent[Rect::X] + (rectParent[Rect::XDIST] / 2) + center;
			anchor[Anchor::LEFT] = anchor[Anchor::RIGHT] = (rectTransform[Rect::XDIST] / 2);
			rectTransform[Rect::X] = pCenter - anchor[Anchor::LEFT];
			rectTransform[Rect::Y] = rectParent[Rect::Y] + anchor[Anchor::TOP];
			anchor[Anchor::BOTTOM] = anchor[Anchor::TOP] + rectTransform[Rect::YDIST];
			break;
		}
		case ComponentRectTransform::P_LEFT:
		{
			break;
		}
		case ComponentRectTransform::P_RIGHT:
		{
			break;
		}
		case ComponentRectTransform::P_BOTTOM:
		{
			uint pCenter = rectParent[Rect::X] + (rectParent[Rect::XDIST] / 2) + center;
			rectTransform[Rect::X] = pCenter - anchor[Anchor::LEFT];
			rectTransform[Rect::Y] = (rectParent[Rect::Y] + rectParent[Rect::YDIST]) - (anchor[Anchor::BOTTOM] + rectTransform[Rect::YDIST]);
			anchor[Anchor::TOP] = anchor[Anchor::BOTTOM] + rectTransform[Rect::YDIST];
			anchor[Anchor::LEFT] = anchor[Anchor::RIGHT] = (rectTransform[Rect::XDIST] / 2);
			break;
		}
		case ComponentRectTransform::P_CENTER:
		{
			uint pxCenter = rectParent[Rect::X] + (rectParent[Rect::XDIST] / 2);
			uint pyCenter = rectParent[Rect::Y] + (rectParent[Rect::YDIST] / 2);
			anchor[Anchor::TOP] = anchor[Anchor::BOTTOM] = (rectTransform[Rect::YDIST] / 2);
			anchor[Anchor::LEFT] = anchor[Anchor::RIGHT] = (rectTransform[Rect::XDIST] / 2);
			rectTransform[Rect::X] = pxCenter - anchor[Anchor::LEFT];
			rectTransform[Rect::Y] = pyCenter - anchor[Anchor::TOP];
			break;
		}
	}
	RecaculatePercentage();
}

void ComponentRectTransform::RecaculatePercentage()
{
	uint* rectParent = nullptr;
	if (parent->cmp_canvas)
		rectParent = App->ui->GetRectUI();
	else
		rectParent = parent->GetParent()->cmp_rectTransform->GetRect();

	anchor_percenatges[RectPercentage::X0] = (float)(rectTransform[Rect::X] - rectParent[Rect::X]) / (float)rectParent[Rect::XDIST];
	anchor_percenatges[RectPercentage::X1] = (float)((rectParent[Rect::X] + rectParent[Rect::XDIST]) - (rectTransform[Rect::X] + rectTransform[Rect::XDIST])) / (float)rectParent[Rect::XDIST];
	anchor_percenatges[RectPercentage::Y0] = (float)(rectTransform[Rect::Y] - rectParent[Rect::Y]) / (float)rectParent[Rect::YDIST];
	anchor_percenatges[RectPercentage::Y1] = (float)((rectParent[Rect::Y] + rectParent[Rect::YDIST]) - (rectTransform[Rect::Y] + rectTransform[Rect::YDIST])) / (float)rectParent[Rect::YDIST];
}

uint ComponentRectTransform::GetInternalSerializationBytes()
{
	return sizeof(RectFrom) + sizeof(RectPrivot) + sizeof(bool) * 2 + sizeof(math::float3) * 4 + sizeof(uint) * 10 + sizeof(float) * 5 + sizeof(int);
}

void ComponentRectTransform::OnInternalSave(char *& cursor)
{
	size_t bytes = sizeof(RectFrom);
	memcpy(cursor, &rFrom, bytes);
	cursor += bytes;

	bytes = sizeof(RectPrivot);
	memcpy(cursor, &pivot, bytes);
	cursor += bytes;

	bytes = sizeof(bool);
	memcpy(cursor, &billboard, bytes);
	cursor += bytes;

	memcpy(cursor, &usePivot, bytes);
	cursor += bytes;

	bytes = sizeof(math::float3) * 4;
	memcpy(cursor, &corners, bytes);
	cursor += bytes;

	bytes = sizeof(uint) * 4;
	memcpy(cursor, &rectTransform, bytes);
	cursor += bytes;

	memcpy(cursor, &anchor, bytes);
	cursor += bytes;

	bytes = sizeof(uint) * 2;
	memcpy(cursor, &lastPositionChange, bytes);
	cursor += bytes;

	bytes = sizeof(float);
	memcpy(cursor, &z, bytes);
	cursor += bytes;

	bytes *= 4;
	memcpy(cursor, &anchor_percenatges, bytes);
	cursor += bytes;

	bytes = sizeof(int);
	memcpy(cursor, &center, bytes);
	cursor += bytes;
}

void ComponentRectTransform::OnInternalLoad(char *& cursor)
{
	size_t bytes = sizeof(RectFrom);
	memcpy(&rFrom, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(RectPrivot);
	memcpy(&pivot, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(bool);
	memcpy(&billboard, cursor, bytes);
	cursor += bytes;

	memcpy(&usePivot, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(math::float3) * 4;
	memcpy(&corners, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(uint) * 4;
	memcpy(&rectTransform, cursor, bytes);
	cursor += bytes;

	memcpy(&anchor, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(uint) * 2;
	memcpy(&lastPositionChange, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(float);
	memcpy(&z, cursor, bytes);
	cursor += bytes;

	bytes *= 4;
	memcpy(&anchor_percenatges, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(int);
	memcpy(&center, cursor, bytes);
	cursor += bytes;

	noUpdatefromCanvas = true;
}

void ComponentRectTransform::OnUniqueEditor()
{
#ifndef GAMEMODE

	ImGui::Text("Rect Transform");
	ImGui::Spacing();

	uint* rectParent = nullptr;
	if (parent->cmp_canvas)
		rectParent = App->ui->GetRectUI();
	else
		rectParent = parent->GetParent()->cmp_rectTransform->GetRect();

	uint r_height = 0;
	uint r_width = 0;

	uint max_xpos = 0;
	uint max_ypos = 0;

	uint max_xdist = 0;
	uint max_ydist = 0;

	uint x_editor = 0;
	uint y_editor = 0;
	int i = 0;
	switch (rFrom)
	{
		case ComponentRectTransform::RECT:
			r_width = rectParent[Rect::XDIST];
			r_height = rectParent[Rect::YDIST];

			max_xpos = r_width - rectTransform[Rect::XDIST];
			max_ypos = r_height - rectTransform[Rect::YDIST];

			x_editor = rectTransform[Rect::X] - rectParent[Rect::X];
			y_editor = rectTransform[Rect::Y] - rectParent[Rect::Y];

			max_xdist = r_width - x_editor;
			max_ydist = r_height - y_editor;
			break;
		case ComponentRectTransform::WORLD:
		{
			ImGui::Text("Modify Transforfm For change RectTransform");
			ImGui::Text("World|Rect difference: %i", i = WORLDTORECT);

			ImGui::Text("Info about Rect:");
			ImGui::Text("Positions X & Y");
			ImGui::Text("X: %u | Y: %u", rectTransform[Rect::X], rectTransform[Rect::Y]);
			ImGui::Text("Dist X & Y");
			ImGui::Text("Dist X: %u | Dist Y: %u", rectTransform[Rect::XDIST], rectTransform[Rect::YDIST]);
			if (parent->cmp_canvas->GetType() == ComponentCanvas::CanvasType::WORLD)
			{
				bool tmp_billboard = billboard;
				if (ImGui::Checkbox("Billboard", &tmp_billboard))
					billboard = tmp_billboard;
			}
			return;
		}
		case ComponentRectTransform::RECT_WORLD:
			r_width = rectParent[Rect::XDIST];
			r_height = rectParent[Rect::YDIST];

			max_xpos = r_width - rectTransform[Rect::XDIST];
			max_ypos = r_height - rectTransform[Rect::YDIST];

			x_editor = rectTransform[Rect::X] - rectParent[Rect::X];
			y_editor = rectTransform[Rect::Y] - rectParent[Rect::Y];

			max_xdist = r_width - x_editor;
			max_ydist = r_height - y_editor;
			break;
	}


	ImGui::PushItemWidth(50.0f);

	ImGui::Text("Positions X & Y");
	if (ImGui::DragScalar("##PosX", ImGuiDataType_U32, &x_editor, 1, 0, &max_xpos, "%u", 1.0f))
	{
		if (x_editor > max_xpos)
			x_editor = max_xpos;

		if (rectParent != nullptr)
			rectTransform[Rect::X] = x_editor + rectParent[Rect::X];
		else
			rectTransform[Rect::X] = x_editor;

		needed_recalculate = true;
	}
	ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	if (ImGui::DragScalar("##PosY", ImGuiDataType_U32, &y_editor, 1, 0, &max_ypos, "%u", 1.0f))
	{
		if (y_editor > max_ypos)
			y_editor = max_ypos;

		if (rectParent != nullptr)
			rectTransform[Rect::Y] = y_editor + rectParent[Rect::Y];
		else
			rectTransform[Rect::Y] = y_editor;

		needed_recalculate = true;
	}
	ImGui::Text("Size X & Y");
	if (ImGui::DragScalar("##SizeX", ImGuiDataType_U32, (void*)&rectTransform[Rect::XDIST], 1, 0, &max_xdist, "%u", 1.0f))
	{
		if (rectTransform[Rect::XDIST] > max_xdist)
			rectTransform[Rect::XDIST] = max_xdist;

		needed_recalculate = true;
	}
	ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	if (ImGui::DragScalar("##SizeY", ImGuiDataType_U32, (void*)&rectTransform[Rect::YDIST], 1, 0, &max_ydist, "%u", 1.0f))
	{
		if (rectTransform[Rect::YDIST] > max_ydist)
			rectTransform[Rect::YDIST] = max_ydist;

		needed_recalculate = true;
	}

	if (needed_recalculate)
		rectTransform_modified = true;

	ImGui::Checkbox("Use Pivot", &usePivot);
	if (usePivot)
	{
		ImGui::PushItemWidth(150.0f);
		ImGui::Text("Pivot");
		ImGui::PushItemWidth(50.0f);

		int current_anchor_flag = (int)pivot;
		if (ImGui::Combo("Using: ", &current_anchor_flag, PIVOT_POINTS_STR))
		{
			pivot = (RectPrivot)current_anchor_flag;

			CalculateAnchors();
		}

		ImGui::PushItemWidth(50.0f);

		ImGui::Text("Margin");

		uint max_yAnchor = rectParent[Rect::YDIST] - rectTransform[Rect::YDIST];
		uint max_xAnhor = rectParent[Rect::XDIST] - rectTransform[Rect::XDIST];

		switch (pivot)
		{
			case ComponentRectTransform::P_TOPLEFT:
			{
				ImGui::Text("Top Left");
				if (ImGui::DragScalar("##MTop", ImGuiDataType_U32, (void*)&anchor[Anchor::TOP], 1, 0, &max_yAnchor, "%u", 1.0f))
					needed_recalculate = true;
				if (ImGui::IsItemHovered())
				{
					ImGui::BeginTooltip();
					ImGui::Text("Top");
					ImGui::EndTooltip();
				}
				ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
				if (ImGui::DragScalar("##MLeft", ImGuiDataType_U32, (void*)&anchor[Anchor::LEFT], 1, 0, &max_xAnhor, "%u", 1.0f))
					needed_recalculate = true;
				if (ImGui::IsItemHovered())
				{
					ImGui::BeginTooltip();
					ImGui::Text("Left");
					ImGui::EndTooltip();
				}
				break;
			}
			case ComponentRectTransform::P_TOPRIGHT:
			{
				ImGui::Text("Top Right");
				if (ImGui::DragScalar("##MTop", ImGuiDataType_U32, (void*)&anchor[Anchor::TOP], 1, 0, &max_yAnchor, "%u", 1.0f))
					needed_recalculate = true;
				if (ImGui::IsItemHovered())
				{
					ImGui::BeginTooltip();
					ImGui::Text("Top");
					ImGui::EndTooltip();
				}
				ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
				if (ImGui::DragScalar("##MRight", ImGuiDataType_U32, (void*)&anchor[Anchor::RIGHT], 1, 0, &max_xAnhor, "%u", 1.0f))
					needed_recalculate = true;
				if (ImGui::IsItemHovered())
				{
					ImGui::BeginTooltip();
					ImGui::Text("Right");
					ImGui::EndTooltip();
				}
				break;
			}
			case ComponentRectTransform::P_BOTTOMLEFT:
			{
				ImGui::Text("Bottom Left");
				if (ImGui::DragScalar("##MBottom", ImGuiDataType_U32, (void*)&anchor[Anchor::BOTTOM], 1, 0, &max_yAnchor, "%u", 1.0f))
					needed_recalculate = true;
				if (ImGui::IsItemHovered())
				{
					ImGui::BeginTooltip();
					ImGui::Text("Bottom");
					ImGui::EndTooltip();
				}
				ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
				if (ImGui::DragScalar("##MLeft", ImGuiDataType_U32, (void*)&anchor[Anchor::LEFT], 1, 0, &max_xAnhor, "%u", 1.0f))
					needed_recalculate = true;
				if (ImGui::IsItemHovered())
				{
					ImGui::BeginTooltip();
					ImGui::Text("Left");
					ImGui::EndTooltip();
				}
				break;
			}
			case ComponentRectTransform::P_BOTTOMRIGHT:
			{
				ImGui::Text("Bottom Right");
				if (ImGui::DragScalar("##MBottom", ImGuiDataType_U32, (void*)&anchor[Anchor::BOTTOM], 1, 0, &max_yAnchor, "%u", 1.0f))
					needed_recalculate = true;
				if (ImGui::IsItemHovered())
				{
					ImGui::BeginTooltip();
					ImGui::Text("Bottom");
					ImGui::EndTooltip();
				}
				ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
				if (ImGui::DragScalar("##MRight", ImGuiDataType_U32, (void*)&anchor[Anchor::RIGHT], 1, 0, &max_xAnhor, "%u", 1.0f))
					needed_recalculate = true;
				if (ImGui::IsItemHovered())
				{
					ImGui::BeginTooltip();
					ImGui::Text("Right");
					ImGui::EndTooltip();
				}
			}
			break;
			case ComponentRectTransform::P_CENTER:
			{
				uint pxCenter = rectParent[Rect::X] + (rectParent[Rect::XDIST] / 2);
				uint pyCenter = rectParent[Rect::Y] + (rectParent[Rect::YDIST] / 2);
				ImGui::Text("Center point from parent (screen point):");
				ImGui::Text("X: %u | Y: %u", pxCenter, pyCenter);
				break;
			}
			case ComponentRectTransform::P_TOP:
			{
				int min_center = -((rectParent[Rect::XDIST] - rectTransform[Rect::XDIST]) / 2);
				int max_center = (rectParent[Rect::XDIST] - rectTransform[Rect::XDIST]) / 2;
				ImGui::Text("Top");
				if (ImGui::DragScalar("##MCenter", ImGuiDataType_S32, (void*)&center, 1, &min_center, &max_center, "%i", 1.0f))
					needed_recalculate = true;
				if (ImGui::IsItemHovered())
				{
					ImGui::BeginTooltip();
					ImGui::Text("X Center");
					ImGui::EndTooltip();
				}
				ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
				if (ImGui::DragScalar("##Top", ImGuiDataType_U32, (void*)&anchor[Anchor::TOP], 1, 0, &max_yAnchor, "%u", 1.0f))
					needed_recalculate = true;
				if (ImGui::IsItemHovered())
				{
					ImGui::BeginTooltip();
					ImGui::Text("Top");
					ImGui::EndTooltip();
				}
				break;
			}
			case ComponentRectTransform::P_LEFT:
			{
				break;
			}
			case ComponentRectTransform::P_RIGHT:
			{
				break;
			}
			case ComponentRectTransform::P_BOTTOM:
			{
				int min_center = -((rectParent[Rect::XDIST] - rectTransform[Rect::XDIST]) / 2);
				int max_center = (rectParent[Rect::XDIST] - rectTransform[Rect::XDIST]) / 2;
				ImGui::Text("Bottom");
				if (ImGui::DragScalar("##MCenter", ImGuiDataType_S32, (void*)&center, 1, &min_center, &max_center, "%i", 1.0f))
					needed_recalculate = true;
				if (ImGui::IsItemHovered())
				{
					ImGui::BeginTooltip();
					ImGui::Text("X Center");
					ImGui::EndTooltip();
				}
				ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
				if (ImGui::DragScalar("##MBottom", ImGuiDataType_U32, (void*)&anchor[Anchor::BOTTOM], 1, 0, &max_yAnchor, "%u", 1.0f))
					needed_recalculate = true;
				if (ImGui::IsItemHovered())
				{
					ImGui::BeginTooltip();
					ImGui::Text("Bottom");
					ImGui::EndTooltip();
				}
				break;
			}
		}
	}

	if (needed_recalculate)
		RecalculateAndChilds();
#endif
}

float ComponentRectTransform::GetZ() const
{
	return z;
}