#include "ComponentCanvasRenderer.h"

#include "Application.h"
#include "ModuleUI.h"

#include "GameObject.h"

#include "ComponentImage.h"
#include "ComponentRectTransform.h"

#include "imgui\imgui.h"
#include "imgui\imgui_internal.h"

ComponentCanvasRenderer::ComponentCanvasRenderer(GameObject * parent, ComponentTypes componentType) : Component(parent, ComponentTypes::CanvasRendererComponent)
{

	if(parent->cmp_rectTransform->GetFrom() == ComponentRectTransform::RectFrom::RECT)
		App->ui->componentsScreenRendererUI.push_back(this);
	else
	{
		App->ui->componentsWorldRendererUI.push_back(this);
		fromWorld = true;
	}

	rend_queue.push_back(new ToUIRend());
	rend_queue.push_back(new ToUIRend());
}

ComponentCanvasRenderer::ComponentCanvasRenderer(const ComponentCanvasRenderer & componentRectTransform, GameObject* parent, bool includeComponents) : Component(parent, ComponentTypes::CanvasRendererComponent)
{
	if (includeComponents)
	{
		if (parent->cmp_rectTransform->GetFrom() == ComponentRectTransform::RectFrom::RECT)
			App->ui->componentsScreenRendererUI.push_back(this);
		else
		{
			App->ui->componentsWorldRendererUI.push_back(this);
			fromWorld = true;
		}

		rend_queue.push_back(new ToUIRend());
		rend_queue.push_back(new ToUIRend());
	}
}

ComponentCanvasRenderer::~ComponentCanvasRenderer()
{
	if (!fromWorld)
		App->ui->componentsScreenRendererUI.remove(this);
	else
		App->ui->componentsWorldRendererUI.remove(this);

	for (ToUIRend* rend : rend_queue)
		RELEASE(rend);
	rend_queue.clear();
}

void ComponentCanvasRenderer::Update()
{
	ComponentImage* cmp_image = (ComponentImage*)parent->GetComponent(ComponentTypes::ImageComponent);
	if (cmp_image)
		if (cmp_image->IsActive() && parent->IsActive())
		{
			if (cmp_image->UseColor())
			{
				for (ToUIRend* rend : rend_queue)
				{
					if (rend->isRendered())
						rend->Set(RenderTypes::COLOR_VECTOR, cmp_image);
				}
			}
			else
			{
				for (ToUIRend* rend : rend_queue)
				{
					if (rend->isRendered())
						rend->Set(RenderTypes::TEXTURE, cmp_image);
				}
			}
		}
}

void ComponentCanvasRenderer::OnEditor()
{
#ifndef GAMEMODE
	OnUniqueEditor();
#endif
}

ComponentCanvasRenderer::ToUIRend* ComponentCanvasRenderer::GetDrawAvaiable() const
{
	if (rend_queue.size() > NULL)
		for (ToUIRend* rend : rend_queue)
			if (!rend->isRendered())
				return rend;
	else
		return nullptr;
}

uint ComponentCanvasRenderer::GetInternalSerializationBytes()
{
	return 0;
}

void ComponentCanvasRenderer::OnInternalSave(char *& cursor)
{
}

void ComponentCanvasRenderer::OnInternalLoad(char *& cursor)
{
	LinkToUIModule();
}

void ComponentCanvasRenderer::OnUniqueEditor()
{
#ifndef GAMEMODE
	ImGui::Text("Canvas Renderer");
#endif
}

void ComponentCanvasRenderer::LinkToUIModule()
{
	/*
	if (parent->cmp_rectTransform->GetFrom() == ComponentRectTransform::RectFrom::RECT)
		App->ui->componentsScreenRendererUI.push_back(this);
	else
	{
		App->ui->componentsWorldRendererUI.push_back(this);
		fromWorld = true;
	}
	*/
	//rend_queue.push_back(new ToUIRend());
	//rend_queue.push_back(new ToUIRend());
}

//Rend Queue Struct

math::float4 ComponentCanvasRenderer::ToUIRend::GetColor()
{
	isRendered_flag = true;
	const float* colors = ((ComponentImage*)cmp)->GetColor();
	return { colors[COLOR_R], colors[COLOR_G], colors[COLOR_B], colors[COLOR_A] };
}

uint ComponentCanvasRenderer::ToUIRend::GetTexture()
{
	isRendered_flag = true;
	return ((ComponentImage*)cmp)->GetResImage();
}
