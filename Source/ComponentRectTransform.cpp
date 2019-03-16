#include "ComponentRectTransform.h"

#include "Application.h"
#include "ModuleUI.h"
#include "ModuleRenderer3D.h"

#include "GameObject.h"
#include "ComponentTransform.h"
#include "ComponentCamera.h"

#include "imgui\imgui.h"
#include "imgui\imgui_internal.h"

#define WORLDTORECT 100.0f
#define ZSEPARATOR 0.005f

ComponentRectTransform::ComponentRectTransform(GameObject * parent, ComponentTypes componentType, RectFrom rF) : Component(parent, ComponentTypes::RectTransformComponent)
{
	rFrom = rF;
	if (parent->GetParent() != nullptr)
	{
		if (rFrom == RectFrom::WORLD)
		{
			App->ui->componentsWorldUI.push_back(this);
			App->ui->GOsWorldCanvas.push_back(parent);
		}
		else
			App->ui->componentsUI.push_back(this);

		CheckParentRect();
	}
}

ComponentRectTransform::ComponentRectTransform(const ComponentRectTransform & componentRectTransform, GameObject* parent, bool includeComponents) : Component(parent, ComponentTypes::RectTransformComponent)
{
	rFrom = componentRectTransform.rFrom;

	if (includeComponents)
	{
		if (rFrom == RectFrom::WORLD)
		{
			App->ui->componentsWorldUI.push_back(this);
			App->ui->GOsWorldCanvas.push_back(parent);
		}
		else
			App->ui->componentsUI.push_back(this);
	}

	memcpy(rectTransform, componentRectTransform.rectTransform, sizeof(uint) * 4);
	memcpy(anchor, componentRectTransform.anchor, sizeof(uint) * 4);
	memcpy(anchor_flags, componentRectTransform.anchor_flags, sizeof(bool) * 4);
	use_margin = componentRectTransform.use_margin;
	billboard = componentRectTransform.billboard;

	//heckParentRect();
}

ComponentRectTransform::~ComponentRectTransform()
{
	if (rFrom == RectFrom::WORLD)
	{
		App->ui->componentsWorldUI.remove(this);
		App->ui->GOsWorldCanvas.remove(parent);
	}
	else
		App->ui->componentsUI.remove(this);
}

void ComponentRectTransform::Update()
{
	switch (rFrom)
	{
	case ComponentRectTransform::RECT:
		break;
	case ComponentRectTransform::WORLD:
		CalculateRectFromWorld(false);
		break;
	case ComponentRectTransform::RECT_WORLD:
		break;
	default:
		break;
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
		bool size_changed = false;
		rectTransform[Rect::X] = x;
		rectTransform[Rect::Y] = y;
		if (rectTransform[Rect::XDIST] != x_dist || rectTransform[Rect::YDIST] != y_dist)
			size_changed = true;
		rectTransform[Rect::XDIST] = x_dist;
		rectTransform[Rect::YDIST] = y_dist;
    
		RecaculateAnchors();
		RecaculatePercentage();
		ParentChanged(size_changed);
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

void ComponentRectTransform::CheckParentRect()
{
	Component* rect = nullptr;
	switch (rFrom)
	{
	case ComponentRectTransform::RECT:
		ui_rect = App->ui->GetRectUI();

		if (parent->GetParent() != nullptr && (rect = parent->GetParent()->GetComponent(ComponentTypes::RectTransformComponent)) != nullptr)
		{
			rectParent = ((ComponentRectTransform*)rect)->GetRect();

			if (rectParent[Rect::XDIST] < rectTransform[Rect::XDIST])
				rectTransform[Rect::XDIST] = rectParent[Rect::XDIST];
			if (rectParent[Rect::YDIST] < rectTransform[Rect::YDIST])
				rectTransform[Rect::YDIST] = rectParent[Rect::YDIST];

			ParentChanged();
		}

		RecaculateAnchors();
		RecaculatePercentage();
		break;
	case ComponentRectTransform::WORLD:
		if (parent != nullptr && (rect = parent->GetComponent(ComponentTypes::TransformComponent)) != nullptr)
		{
			transformParent = (ComponentTransform*)rect;
			transformParent->UpdateGlobal();
			CalculateRectFromWorld(true);
		}
		break;
	case ComponentRectTransform::RECT_WORLD:
		if (parent->GetParent() != nullptr && (rect = parent->GetParent()->GetComponent(ComponentTypes::RectTransformComponent)) != nullptr)
		{
			ComponentRectTransform* r = (ComponentRectTransform*)rect;
			parentCorners = r->GetCorners();
			rectParent = r->GetRect();
			z = r->GetZ() + ZSEPARATOR;
			if (rectTransform[Rect::XDIST] > rectParent[Rect::XDIST])
				rectTransform[Rect::XDIST] = rectParent[Rect::XDIST];
			if (rectTransform[Rect::YDIST] > rectParent[Rect::YDIST])
				rectTransform[Rect::YDIST] = rectParent[Rect::YDIST];

			RecaculateAnchors();
			RecaculatePercentage();

			ParentChanged();
		}
		break;
	}
}

void ComponentRectTransform::ChangeChildsRect(bool its_me, bool size_changed)
{
	if (!its_me)
	{
		if (use_margin)
			ParentChanged(false);
		else
			ParentChanged(size_changed);
	}

	std::vector<GameObject*> childs;
	parent->GetChildrenVector(childs);
	std::reverse(childs.begin(), childs.end());
	for (GameObject* c_go : childs)
	{
		if (c_go != parent)
			((ComponentRectTransform*)c_go->GetComponent(ComponentTypes::RectTransformComponent))->ChangeChildsRect(false, size_changed);
	}
}

ComponentRectTransform::RectFrom ComponentRectTransform::GetFrom() const
{
	return rFrom;
}

void ComponentRectTransform::ParentChanged(bool size_changed)
{
	if (size_changed)
	{
		rectTransform[Rect::X] = (uint)(anchor_percenatges[RectPercentage::X0] * (float)rectParent[Rect::XDIST]) + rectParent[Rect::X];
		rectTransform[Rect::XDIST] = rectParent[Rect::XDIST] - ((rectTransform[Rect::X] - rectParent[Rect::X]) + (uint)(anchor_percenatges[RectPercentage::X1] * (float)rectParent[Rect::XDIST]));
		rectTransform[Rect::Y] = (uint)(anchor_percenatges[RectPercentage::Y0] * (float)rectParent[Rect::YDIST]) + rectParent[Rect::Y];
		rectTransform[Rect::YDIST] = rectParent[Rect::YDIST] - ((rectTransform[Rect::Y] - rectParent[Rect::Y]) + (uint)(anchor_percenatges[RectPercentage::Y1] * (float)rectParent[Rect::YDIST]));
	
		RecaculateAnchors();
	}
	else
	{
		if (anchor_flags[Anchor::LEFT] == RectPrivot::TOPLEFT)
		{
			rectTransform[Rect::X] = anchor[Anchor::LEFT] + rectParent[Rect::X];
			rectTransform[Rect::Y] = anchor[Anchor::TOP] + rectParent[Rect::Y];
			anchor[Anchor::RIGHT] = rectParent[Rect::XDIST] - (rectTransform[Rect::X] + rectTransform[Rect::XDIST]);
			anchor[Anchor::BOTTOM] = rectParent[Rect::YDIST] - (rectTransform[Rect::Y] + rectTransform[Rect::YDIST]);
		}
		else
		{
			rectTransform[Rect::X] = (rectParent[Rect::X] + rectParent[Rect::XDIST]) - anchor[Anchor::RIGHT] - rectTransform[Rect::XDIST];
			rectTransform[Rect::Y] = (rectParent[Rect::Y] + rectParent[Rect::YDIST]) - anchor[Anchor::BOTTOM] - rectTransform[Rect::YDIST];
			anchor[Anchor::LEFT] = (rectTransform[Rect::X] + rectTransform[Rect::XDIST]) + anchor[Anchor::RIGHT];
			anchor[Anchor::TOP] = (rectTransform[Rect::Y] + rectTransform[Rect::YDIST]) + anchor[Anchor::BOTTOM];
		}
		RecaculatePercentage();
	}

	if (rFrom == RectFrom::RECT_WORLD)
		CalculateCornersFromRect();
}

void ComponentRectTransform::UseMarginChanged(bool useMargin)
{
	use_margin = useMargin;

	std::vector<GameObject*> childs;
	parent->GetChildrenVector(childs);
	std::reverse(childs.begin(), childs.end());
	for (GameObject* c_go : childs)
	{
		if (c_go != parent)
			((ComponentRectTransform*)c_go->GetComponent(ComponentTypes::RectTransformComponent))->UseMarginChanged(useMargin);
	}
}

void ComponentRectTransform::CalculateRectFromWorld(bool individualcheck)
{
	if (billboard)
	{
		math::float3 zAxis = App->renderer3D->GetCurrentCamera()->frustum.front;
		math::float3 yAxis = App->renderer3D->GetCurrentCamera()->frustum.up;
		math::float3 xAxis = yAxis.Cross(zAxis).Normalized();

		math::float3 pos = math::float3::zero;
		math::Quat rot = math::Quat::identity;
		math::float3 scale = math::float3::zero;
		math::float4x4 global = transformParent->GetGlobalMatrix();
		global.Decompose(pos, rot, scale);

		transformParent->SetMatrixFromGlobal(global.FromTRS(pos, math::Quat::identity * math::float3x3(xAxis, yAxis, zAxis).ToQuat(), scale));
	}

	math::float4x4 globalmatrix = transformParent->GetGlobalMatrix();
	corners[Rect::RTOPLEFT] = math::float4(globalmatrix * math::float4(-0.5f, 0.5f, 0.0f, 1.0f)).Float3Part();
	corners[Rect::RTOPRIGHT] = math::float4(globalmatrix * math::float4(0.5f, 0.5f, 0.0f, 1.0f)).Float3Part();
	corners[Rect::RBOTTOMLEFT] = math::float4(globalmatrix * math::float4(-0.5f, -0.5f, 0.0f, 1.0f)).Float3Part();
	corners[Rect::RBOTTOMRIGHT] = math::float4(globalmatrix * math::float4(0.5f, -0.5f, 0.0f, 1.0f)).Float3Part();

	rectTransform[Rect::X] = 0;
	rectTransform[Rect::Y] = 0;
	rectTransform[Rect::XDIST] = abs(math::Distance(corners[Rect::RTOPRIGHT], corners[Rect::RTOPLEFT])) * WORLDTORECT;
	rectTransform[Rect::YDIST] = abs(math::Distance(corners[Rect::RBOTTOMLEFT], corners[Rect::RTOPLEFT])) * WORLDTORECT;

	if(!individualcheck)
		ChangeChildsRect(true);
}

void ComponentRectTransform::CalculateCornersFromRect()
{
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

void ComponentRectTransform::RecaculateAnchors()
{
	if (rectParent != nullptr)
	{
		if (anchor_flags[Anchor::LEFT] == RectPrivot::TOPLEFT)
			anchor[Anchor::LEFT] = rectTransform[Rect::X] - rectParent[Rect::X];
		else
			anchor[Anchor::LEFT] = (rectParent[Rect::X] + rectParent[Rect::XDIST]) - rectTransform[Rect::X];

		if (anchor_flags[Anchor::TOP] == RectPrivot::TOPLEFT)
			anchor[Anchor::TOP] = rectTransform[Rect::Y] - rectParent[ModuleUI::Screen::Y];
		else
			anchor[Anchor::TOP] = (rectParent[Rect::Y] + rectParent[Rect::YDIST]) - rectTransform[Rect::Y];

		if (anchor_flags[Anchor::RIGHT] == RectPrivot::TOPLEFT)
			anchor[Anchor::RIGHT] = (rectTransform[Rect::X] + rectTransform[Rect::XDIST]) - rectParent[Rect::X];
		else
			anchor[Anchor::RIGHT] = (rectParent[Rect::X] + rectParent[Rect::XDIST]) - (rectTransform[Rect::X] + rectTransform[Rect::XDIST]);

		if (anchor_flags[Anchor::BOTTOM] == RectPrivot::TOPLEFT)
			anchor[Anchor::BOTTOM] = (rectTransform[Rect::Y] + rectTransform[Rect::YDIST]) - rectParent[Rect::Y];
		else
			anchor[Anchor::BOTTOM] = (rectParent[Rect::Y] + rectParent[Rect::YDIST]) - (rectTransform[Rect::Y] + rectTransform[Rect::YDIST]);
	}
	else
	{
		if (anchor_flags[Anchor::LEFT] == RectPrivot::TOPLEFT)
			anchor[Anchor::LEFT] = rectTransform[Rect::X] - ui_rect[ModuleUI::Screen::X];
		else
			anchor[Anchor::LEFT] = ui_rect[ModuleUI::Screen::WIDTH] - rectTransform[Rect::X];

		if (anchor_flags[Anchor::TOP] == RectPrivot::TOPLEFT)
			anchor[Anchor::TOP] = rectTransform[Rect::Y] - ui_rect[ModuleUI::Screen::Y];
		else
			anchor[Anchor::TOP] = ui_rect[ModuleUI::Screen::HEIGHT] - rectTransform[Rect::Y];

		if (anchor_flags[Anchor::RIGHT] == RectPrivot::TOPLEFT)
			anchor[Anchor::RIGHT] = (rectTransform[Rect::X] + rectTransform[Rect::XDIST]) - ui_rect[ModuleUI::Screen::X];
		else
			anchor[Anchor::RIGHT] = ui_rect[ModuleUI::Screen::WIDTH] - (rectTransform[Rect::X] + rectTransform[Rect::XDIST]);

		if (anchor_flags[Anchor::BOTTOM] == RectPrivot::TOPLEFT)
			anchor[Anchor::BOTTOM] = (rectTransform[Rect::Y] + rectTransform[Rect::YDIST]) - ui_rect[ModuleUI::Screen::X];
		else
			anchor[Anchor::BOTTOM] = ui_rect[ModuleUI::Screen::HEIGHT] - (rectTransform[Rect::Y] + rectTransform[Rect::YDIST]);
	}
}

void ComponentRectTransform::RecaculateAnchors(int type)
{
	if (RectPrivot::TOPLEFT == type)
	{
		if (rectParent != nullptr)
		{
			rectTransform[Rect::X] = anchor[Anchor::LEFT] + rectParent[Rect::X];
			rectTransform[Rect::Y] = anchor[Anchor::TOP] + rectParent[Rect::Y];
			anchor[Anchor::RIGHT] = rectParent[Rect::XDIST] - (rectTransform[Rect::X] + rectTransform[Rect::XDIST]);
			anchor[Anchor::BOTTOM] = rectParent[Rect::YDIST] - (rectTransform[Rect::Y] + rectTransform[Rect::YDIST]);
		}
		else
		{
			rectTransform[Rect::X] = anchor[Anchor::LEFT] + ui_rect[ModuleUI::Screen::X];
			rectTransform[Rect::Y] = anchor[Anchor::TOP] + ui_rect[ModuleUI::Screen::Y];
			anchor[Anchor::RIGHT] = ui_rect[ModuleUI::Screen::WIDTH] - (rectTransform[Rect::X] + rectTransform[Rect::XDIST]);
			anchor[Anchor::BOTTOM] = ui_rect[ModuleUI::Screen::HEIGHT] - (rectTransform[Rect::Y] + rectTransform[Rect::YDIST]);
		}
	}
	else if (RectPrivot::BOTTOMRIGHT == type)
	{
		if (rectParent != nullptr)
		{
			rectTransform[Rect::X] = (rectParent[Rect::X] + rectParent[Rect::XDIST]) - anchor[Anchor::RIGHT] - rectTransform[Rect::XDIST];
			rectTransform[Rect::Y] = (rectParent[Rect::Y] + rectParent[Rect::YDIST]) - anchor[Anchor::BOTTOM] - rectTransform[Rect::YDIST];
			anchor[Anchor::LEFT] = (rectTransform[Rect::X] + rectTransform[Rect::XDIST]) + anchor[Anchor::RIGHT];
			anchor[Anchor::TOP] = (rectTransform[Rect::Y] + rectTransform[Rect::YDIST]) + anchor[Anchor::BOTTOM];
		}
		else
		{
			rectTransform[Rect::X] = (ui_rect[ModuleUI::Screen::WIDTH] + ui_rect[ModuleUI::Screen::X]) - anchor[Anchor::RIGHT] - rectTransform[Rect::XDIST];
			rectTransform[Rect::Y] = (ui_rect[ModuleUI::Screen::HEIGHT] + ui_rect[ModuleUI::Screen::Y]) - anchor[Anchor::BOTTOM] - rectTransform[Rect::YDIST];
			anchor[Anchor::LEFT] = (rectTransform[Rect::X] + rectTransform[Rect::XDIST]) + anchor[Anchor::RIGHT];
			anchor[Anchor::TOP] = (rectTransform[Rect::Y] + rectTransform[Rect::YDIST]) + anchor[Anchor::BOTTOM];
		}
	}

	ChangeChildsRect(true);
}

void ComponentRectTransform::RecaculatePercentage()
{
	if (rectParent != nullptr)
	{
		anchor_percenatges[RectPercentage::X0] = (float)(rectTransform[Rect::X] - rectParent[Rect::X]) / (float)rectParent[Rect::XDIST];
		anchor_percenatges[RectPercentage::X1] = (float)((rectParent[Rect::X] + rectParent[Rect::XDIST]) - (rectTransform[Rect::X] + rectTransform[Rect::XDIST])) / (float)rectParent[Rect::XDIST];
		anchor_percenatges[RectPercentage::Y0] = (float)(rectTransform[Rect::Y] - rectParent[Rect::Y]) / (float)rectParent[Rect::YDIST];
		anchor_percenatges[RectPercentage::Y1] = (float)((rectParent[Rect::Y] + rectParent[Rect::YDIST]) - (rectTransform[Rect::Y] + rectTransform[Rect::YDIST])) / (float)rectParent[Rect::YDIST];
	}
	else
	{
		anchor_percenatges[RectPercentage::X0] = (float)(rectTransform[Rect::X] - ui_rect[ModuleUI::Screen::X]) / (float)ui_rect[ModuleUI::Screen::WIDTH];
		anchor_percenatges[RectPercentage::X1] = (float)((ui_rect[ModuleUI::Screen::X] + ui_rect[ModuleUI::Screen::WIDTH]) - (rectTransform[Rect::X] + rectTransform[Rect::XDIST])) / (float)ui_rect[ModuleUI::Screen::WIDTH];
		anchor_percenatges[RectPercentage::Y0] = (float)(rectTransform[Rect::Y] - ui_rect[ModuleUI::Screen::Y]) / (float)ui_rect[ModuleUI::Screen::HEIGHT];
		anchor_percenatges[RectPercentage::Y1] = (float)((ui_rect[ModuleUI::Screen::Y] + ui_rect[ModuleUI::Screen::HEIGHT]) - (rectTransform[Rect::Y] + rectTransform[Rect::YDIST])) / (float)ui_rect[ModuleUI::Screen::HEIGHT];
	}
}

uint ComponentRectTransform::GetInternalSerializationBytes()
{
	return sizeof(uint) * 8 + sizeof(bool) * 6 + sizeof(int);
}

void ComponentRectTransform::OnInternalSave(char *& cursor)
{
	size_t bytes = sizeof(uint) * 4;
	memcpy(cursor, &rectTransform, bytes);
	cursor += bytes;

	bytes = sizeof(uint) * 4;
	memcpy(cursor, &anchor, bytes);
	cursor += bytes;

	bytes = sizeof(bool) * 4;
	memcpy(cursor, &anchor_flags, bytes);
	cursor += bytes;

	bytes = sizeof(bool);
	memcpy(cursor, &use_margin, bytes);
	cursor += bytes;

	bytes = sizeof(bool);
	memcpy(cursor, &billboard, bytes);
	cursor += bytes;

	bytes = sizeof(int);
	int i = (int)rFrom;
	memcpy(cursor, &i, bytes);
	cursor += bytes;
}

void ComponentRectTransform::OnInternalLoad(char *& cursor)
{
	size_t bytes = sizeof(uint) * 4;
	memcpy(&rectTransform, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(uint) * 4;
	memcpy(&anchor, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(bool) * 4;
	memcpy(&anchor_flags, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(bool);
	memcpy(&use_margin, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(bool);
	memcpy(&billboard, cursor, bytes);
	cursor += bytes;
	
	bytes = sizeof(int);
	int i = 0;
	memcpy(&i, cursor, bytes);
	cursor += bytes;
	rFrom = (RectFrom)i;
	
	LinkToUIModule();
}

void ComponentRectTransform::OnUniqueEditor()
{
#ifndef GAMEMODE

	ImGui::Text("Rect Transform");
	ImGui::Spacing();

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
		if (rectParent != nullptr)
		{
			r_width = rectParent[Rect::XDIST];
			r_height = rectParent[Rect::YDIST];

			max_xpos = r_width - rectTransform[Rect::XDIST];
			max_ypos = r_height - rectTransform[Rect::YDIST];

			x_editor = rectTransform[Rect::X] - rectParent[Rect::X];
			y_editor = rectTransform[Rect::Y] - rectParent[Rect::Y];

			max_xdist = r_width - x_editor;
			max_ydist = r_height - y_editor;
		}
		else
		{
			r_height = ui_rect[ModuleUI::Screen::HEIGHT];
			r_width = ui_rect[ModuleUI::Screen::WIDTH];

			max_xpos = r_width - rectTransform[Rect::XDIST];
			max_ypos = r_height - rectTransform[Rect::YDIST];

			x_editor = rectTransform[Rect::X];
			y_editor = rectTransform[Rect::Y];

			max_xdist = r_width - x_editor;
			max_ydist = r_height - y_editor;
		}
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
		bool tmp_billboard = billboard;
		if (ImGui::Checkbox("Billboard", &tmp_billboard))
			billboard = tmp_billboard;
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

	bool needed_recalculate = false;
	bool size_changed = false;

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
		size_changed = true;
	}
	ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	if (ImGui::DragScalar("##SizeY", ImGuiDataType_U32, (void*)&rectTransform[Rect::YDIST], 1, 0, &max_ydist, "%u", 1.0f))
	{
		if (rectTransform[Rect::YDIST] > max_ydist)
			rectTransform[Rect::YDIST] = max_ydist;

		needed_recalculate = true;
		size_changed = true;
	}

	if (needed_recalculate)
	{
		RecaculateAnchors();
		RecaculatePercentage();
		ChangeChildsRect(true, size_changed);
	}

	ImGui::Checkbox("Use margin", &use_margin);
	ImGui::SameLine();
	if (ImGui::Button("Set to childs"))
		UseMarginChanged(use_margin);

	if (use_margin)
	{
		ImGui::PushItemWidth(150.0f);
		ImGui::Text("Anchor");
		ImGui::PushItemWidth(50.0f);

		int current_anchor_flag = (int)anchor_flags[Anchor::LEFT];
		ImGui::Text("Reference points");
		ImGui::Text("Begin - Top/Left");
		ImGui::Text("End - Bottom/Right");
		if (ImGui::Combo("Using: ", &current_anchor_flag, ANCHORS_POINTS_STR))
		{
			anchor_flags[Anchor::LEFT] = current_anchor_flag;
			anchor_flags[Anchor::TOP] = current_anchor_flag;
			anchor_flags[Anchor::RIGHT] = current_anchor_flag;
			anchor_flags[Anchor::BOTTOM] = current_anchor_flag;

			RecaculateAnchors();
		}

		needed_recalculate = false;

		ImGui::PushItemWidth(50.0f);

		ImGui::Text("Margin");
		if (current_anchor_flag == RectPrivot::TOPLEFT)
		{
			uint max_leftAnhor = 0;
			uint max_topAnchor = 0;
			if (rectParent)
			{
				max_leftAnhor = rectParent[Rect::XDIST] - rectTransform[Rect::XDIST];
				max_topAnchor = rectParent[Rect::YDIST] - rectTransform[Rect::YDIST];
			}
			else
			{
				max_leftAnhor = ui_rect[ModuleUI::Screen::WIDTH] - rectTransform[Rect::XDIST];
				max_topAnchor = ui_rect[ModuleUI::Screen::HEIGHT] - rectTransform[Rect::YDIST];
			}

			ImGui::Text("Left/Top");
			if (ImGui::DragScalar("##MLeft", ImGuiDataType_U32, (void*)&anchor[Anchor::LEFT], 1, 0, &max_leftAnhor, "%u", 1.0f))
				needed_recalculate = true;
			ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
			if (ImGui::DragScalar("##MTop", ImGuiDataType_U32, (void*)&anchor[Anchor::TOP], 1, 0, &max_topAnchor, "%u", 1.0f))
				needed_recalculate = true;

			if (needed_recalculate)
				RecaculateAnchors(RectPrivot::TOPLEFT);
		}
		else
		{
			uint max_rightAnhor = 0;
			uint max_bottomAnchor = 0;

			if (rectParent)
			{
				max_rightAnhor = rectParent[Rect::XDIST] - rectTransform[Rect::XDIST];
				max_bottomAnchor = rectParent[Rect::YDIST] - rectTransform[Rect::YDIST];
			}
			else
			{
				max_rightAnhor = ui_rect[ModuleUI::Screen::WIDTH] - rectTransform[Rect::XDIST];
				max_bottomAnchor = ui_rect[ModuleUI::Screen::HEIGHT] - rectTransform[Rect::YDIST];
			}

			ImGui::Text("Right/Bottom");
			if (ImGui::DragScalar("##MRight", ImGuiDataType_U32, (void*)&anchor[Anchor::RIGHT], 1, 0, &max_rightAnhor, "%u", 1.0f))
				needed_recalculate = true;
			ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
			if (ImGui::DragScalar("##MBottom", ImGuiDataType_U32, (void*)&anchor[Anchor::BOTTOM], 1, 0, &max_bottomAnchor, "%u", 1.0f))
				needed_recalculate = true;

			if (needed_recalculate)
				RecaculateAnchors(RectPrivot::BOTTOMRIGHT);
		}
	}
#endif
}

float ComponentRectTransform::GetZ() const
{
	return z;
}

void ComponentRectTransform::LinkToUIModule()
{
	if (rFrom == RectFrom::WORLD)
	{
		App->ui->componentsWorldUI.push_back(this);
		App->ui->GOsWorldCanvas.push_back(parent);
	}
	else
		App->ui->componentsUI.push_back(this);
}
