#include "ComponentCanvas.h"

#include "Application.h"
#include "ModuleUI.h"
#include "ModuleRenderer3D.h"
#include "ModuleScene.h"

#include "GameObject.h"
#include "ComponentTransform.h"
#include "ComponentCamera.h"

#define CANVAS_TYPE_STR "Screen\0World Screen\0World"

#include "imgui\imgui.h"
#include "imgui\imgui_internal.h"

#include "MathGeoLib\include\Math\float4x4.h"
#include "MathGeoLib/include/Geometry/Frustum.h"

ComponentCanvas::ComponentCanvas(GameObject * parent, ComponentTypes componentType) : Component(parent, ComponentTypes::CanvasComponent)
{
	App->ui->canvas.push_back(parent);
}

ComponentCanvas::ComponentCanvas(const ComponentCanvas & componentCanvas, GameObject * parent, bool includeComponents) : Component(parent, ComponentTypes::CanvasComponent)
{
	type = componentCanvas.type;
	if (includeComponents)
	{
		App->ui->canvas.push_back(parent);
		needed_change = true;
	}
}

ComponentCanvas::~ComponentCanvas()
{
	if (transform)
		RELEASE(transform);
	if (fakeGo)
		RELEASE(fakeGo);
}

void ComponentCanvas::Update()
{
	if (needed_change)
	{
		if (transform)
			RELEASE(transform);
		if (fakeGo)
			RELEASE(fakeGo);

		switch (type)
		{
		case CanvasType::SCREEN:
			App->ui->canvas_screen.push_back(parent);
			break;
		case CanvasType::WORLD_SCREEN:
			App->ui->canvas_worldScreen.push_back(parent);

			if (!transform)
				transform = new ComponentTransform(nullptr);
			if (!fakeGo)
				fakeGo = new GameObject("", nullptr, true);
			fakeGo->transform = transform;
			if (App->renderer3D->GetCurrentCamera()->GetParent())
			{
				fakeGo->SetParent(App->renderer3D->GetCurrentCamera()->GetParent());
				fakeGo->transform->UpdateGlobal();
			}
			else
			{
				//math::float4x4 matrix = math::float4x4(App->renderer3D->GetCurrentCamera()->frustum.WorldMatrix());
				//transform->SetMatrixFromGlobal(matrix);
			}
			break;
		case CanvasType::WORLD:
			App->ui->canvas_world.push_back(parent);
			break;
		}

		needed_change = false;
	}

	switch (type)
	{
	case ComponentCanvas::SCREEN:
		break;
	case ComponentCanvas::WORLD_SCREEN:
		/*
		if (!fakeGoTransform->GetParent())
		{
			math::float4x4 matrix = math::float4x4(App->renderer3D->GetCurrentCamera()->frustum.WorldMatrix());
			fakeGoTransform->transform->SetMatrixFromGlobal(matrix);
		}*/
		break;
	case ComponentCanvas::WORLD:
		break;
	default:
		break;
	}
}

void ComponentCanvas::OnEditor()
{
#ifndef GAMEMODE
	ImGui::Text("Canvas");
	int current_type = int(type);
	if (ImGui::Combo("Using", &current_type, CANVAS_TYPE_STR))
	{
		if ((CanvasType)current_type != type)
		{
			switch (type)
			{
			case ComponentCanvas::SCREEN:
				App->ui->canvas_screen.remove(parent);
				break;
			case ComponentCanvas::WORLD_SCREEN:
				App->ui->canvas_worldScreen.remove(parent);
				break;
			case ComponentCanvas::WORLD:
				App->ui->canvas_world.remove(parent);
				break;
			}
			type = (CanvasType)current_type;
			needed_change = true;
		}
	}
	if(!needed_change)
		if (parent->transform && type != CanvasType::SCREEN)
			if (type == CanvasType::WORLD_SCREEN)
			{
				if (fakeGo->GetParent())
					App->scene->OnGizmos(fakeGo);
				transform->OnUniqueEditor();
			}
			else
			{
				App->scene->OnGizmos(parent);
				parent->transform->OnUniqueEditor(); 
			}
#endif
}

uint ComponentCanvas::GetInternalSerializationBytes()
{
	return 0;
}

void ComponentCanvas::OnInternalSave(char *& cursor)
{
}

void ComponentCanvas::OnInternalLoad(char *& cursor)
{
}

ComponentCanvas::CanvasType ComponentCanvas::GetType() const
{
	return type;
}
