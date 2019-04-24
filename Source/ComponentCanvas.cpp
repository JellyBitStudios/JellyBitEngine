#include "ComponentCanvas.h"

#include "Application.h"
#include "ModuleUI.h"
#include "ModuleRenderer3D.h"
#include "ModuleScene.h"

#include "GameObject.h"
#include "ComponentTransform.h"
#include "ComponentRectTransform.h"
#include "ComponentCanvasRenderer.h"
#include "ComponentCamera.h"
#include "ComponentButton.h"
#include "ComponentImage.h"
#include "ComponentLabel.h"
#include "ComponentSlider.h"
#include "Panel.h"

#define CANVAS_TYPE_STR "Screen\0World Screen (Work In Progress)\0World"

#include "imgui\imgui.h"
#include "imgui\imgui_internal.h"

#include "MathGeoLib\include\Math\float4x4.h"
#include "MathGeoLib/include/Geometry/Frustum.h"

#define ZSEPARATOR 0.005f

ComponentCanvas::ComponentCanvas(GameObject * parent, ComponentTypes componentType, bool includeComponents) : Component(parent, ComponentTypes::CanvasComponent)
{
	if (includeComponents)
	{
		App->ui->canvas.push_back(parent);
		needed_change = true;
		Update();
	}
}

ComponentCanvas::ComponentCanvas(const ComponentCanvas & componentCanvas, GameObject * parent, bool includeComponents) : Component(parent, ComponentTypes::CanvasComponent)
{
	type = componentCanvas.type;
	if (includeComponents)
	{
		App->ui->canvas.push_back(parent);
		needed_change = true;
		Update();
	}
}

ComponentCanvas::~ComponentCanvas()
{
	if (transform)
		RELEASE(transform);
	if (fakeGo)
		RELEASE(fakeGo);

	parent->cmp_canvas = nullptr;
}

void ComponentCanvas::OnSystemEvent(System_Event event)
{
	switch (event.type)
	{
	case System_Event_Type::WRectTransformUpdated:
	{
		event.type = System_Event_Type::RectTransformUpdated;
		std::vector<GameObject*> rectChilds;
		parent->GetChildrenAndThisVectorFromLeaf(rectChilds);

		for (std::vector<GameObject*>::const_reverse_iterator go = rectChilds.crbegin(); go != rectChilds.crend(); go++)
			(*go)->OnSystemEvent(event);
		break;
	}
	case System_Event_Type::ScreenChanged:
	{
		std::vector<GameObject*> rectChilds;
		parent->GetChildrenAndThisVectorFromLeaf(rectChilds);

		for (std::vector<GameObject*>::iterator go = rectChilds.begin(); go != rectChilds.end(); go++)
			if((*go) != parent)
				(*go)->OnSystemEvent(event);
		break;
	}
	}
}

void ComponentCanvas::Update()
{
	if (needed_change)
	{
		if (transform)
			RELEASE(transform);
		if (fakeGo)
			RELEASE(fakeGo);


		for (GameObject* go : App->ui->canvas_screen)
		{
			if (go == parent)
			{
				App->ui->canvas_screen.remove(parent);
				break;
			}
		}

		for (GameObject* go : App->ui->canvas_worldScreen)
		{
			if (go == parent)
			{
				App->ui->canvas_worldScreen.remove(parent);
				break;
			}
		}

		for (GameObject* go : App->ui->canvas_world)
		{
			if (go == parent)
			{
				App->ui->canvas_world.remove(parent);
				break;
			}
		}

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



		if (!first_iterate)
		{
			System_Event canvasChanged;
			canvasChanged.type = System_Event_Type::CanvasChanged;

			std::vector<GameObject*> rectChilds;
			parent->GetChildrenAndThisVectorFromLeaf(rectChilds);

			for (std::vector<GameObject*>::const_reverse_iterator go = rectChilds.crbegin(); go != rectChilds.crend(); go++)
				(*go)->OnSystemEvent(canvasChanged);
		}

		needed_change = false;
		first_iterate = false;
	}

	std::vector<GameObject*> childs;
	parent->GetChildrenAndThisVectorFromLeaf(childs);

	for (std::vector<GameObject*>::const_reverse_iterator go = childs.crbegin(); go != childs.crend(); go++)
	{
		if ((*go)->cmp_rectTransform) (*go)->cmp_rectTransform->Update();
		if ((*go)->cmp_label) (*go)->cmp_label->Update();
		if ((*go)->cmp_image) (*go)->cmp_image->Update();
		if ((*go)->cmp_slider) (*go)->cmp_slider->Update();
		if (App->GetEngineState() == engine_states::ENGINE_PLAY && (*go)->cmp_button) (*go)->cmp_button->Update();
		if ((*go)->cmp_canvasRenderer) (*go)->cmp_canvasRenderer->Update();
	}
}

void ComponentCanvas::OnEditor()
{
#ifndef GAMEMODE
	if (ImGui::CollapsingHeader("Canvas", ImGuiTreeNodeFlags_DefaultOpen))
	{
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
		if (!needed_change)
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
	}
#endif
}

uint ComponentCanvas::GetInternalSerializationBytes()
{
	return sizeof(CanvasType);
}

void ComponentCanvas::OnInternalSave(char *& cursor)
{
	size_t bytes = sizeof(CanvasType);
	memcpy(cursor, &type, bytes);
	cursor += bytes;
}

void ComponentCanvas::OnInternalLoad(char *& cursor)
{
	size_t bytes = sizeof(CanvasType);
	memcpy(&type, cursor, bytes);
	cursor += bytes;

	needed_change = true;
	Update();
}

void ComponentCanvas::Change(CanvasType to)
{
	if (to)
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
		type = to;
		needed_change = true;
	}
}

ComponentCanvas::CanvasType ComponentCanvas::GetType() const
{
	return type;
}

math::float4x4 ComponentCanvas::GetGlobal() const
{
	math::float4x4 ret = math::float4x4::identity;

	switch (type)
	{
	case ComponentCanvas::WORLD_SCREEN:
		ret = transform->GetGlobalMatrix();
		break;
	case ComponentCanvas::WORLD:
		ret = parent->transform->GetGlobalMatrix();
		break;
	}

	return ret;
}

float ComponentCanvas::GetZ(GameObject * go, ComponentTypes type)
{
	uint farGO;
	ModuleUI::FindCanvas(go, farGO);
	switch (type)
	{
	case RectTransformComponent:
		return ZSEPARATOR * farGO;
		break;
	case LabelComponent:
		return ZSEPARATOR * (farGO + 1);
		break;
	case SliderComponent:
		return ZSEPARATOR * (farGO + 1);
		break;
	}
	return 0.0f;
}
