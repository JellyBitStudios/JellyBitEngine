#include "ComponentCanvasRenderer.h"

#include "Application.h"
#include "ModuleUI.h"

#include "GameObject.h"

#include "ComponentImage.h"
#include "ComponentRectTransform.h"

#include "imgui\imgui.h"
#include "imgui\imgui_internal.h"

ComponentCanvasRenderer::ComponentCanvasRenderer(GameObject * parent, ComponentTypes componentType, bool includeComponents) : Component(parent, ComponentTypes::CanvasRendererComponent)
{
	if (includeComponents)
	{
		rend_queue.push_back(new ToUIRend());
		rend_queue.push_back(new ToUIRend());
	}
}

ComponentCanvasRenderer::ComponentCanvasRenderer(const ComponentCanvasRenderer & componentRectTransform, GameObject* parent, bool includeComponents) : Component(parent, ComponentTypes::CanvasRendererComponent)
{
	if (includeComponents)
	{
		rend_queue.push_back(new ToUIRend());
		rend_queue.push_back(new ToUIRend());
	}
}

ComponentCanvasRenderer::~ComponentCanvasRenderer()
{
	for (ToUIRend* rend : rend_queue)
		RELEASE(rend);
	rend_queue.clear();

	parent->cmp_canvasRenderer = nullptr;
}

void ComponentCanvasRenderer::OnSystemEvent(System_Event event)
{
}

void ComponentCanvasRenderer::Update()
{
	ComponentImage* cmp_image = (ComponentImage*)parent->GetComponent(ComponentTypes::ImageComponent);
	if (cmp_image)
		if (cmp_image->IsTreeActive())
		{
			for (ToUIRend* rend : rend_queue)
			{
				if (rend->isRendered())
					rend->Set(RenderTypes::IMAGE, cmp_image);
			}
		}
	ComponentLabel* cmp_label = (ComponentLabel*)parent->GetComponent(ComponentTypes::LabelComponent);
	if (cmp_label)
		if (cmp_label->IsTreeActive())
		{
			for (ToUIRend* rend : rend_queue)
			{
				if (rend->isRendered())
					rend->Set(RenderTypes::LABEL, cmp_label);
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

bool ComponentCanvasRenderer::IsWorld() const
{
	return fromWorld;
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
}

void ComponentCanvasRenderer::OnUniqueEditor()
{
#ifndef GAMEMODE
	ImGui::Text("Canvas Renderer");
#endif
}

//Rend Queue Struct

math::float4 ComponentCanvasRenderer::ToUIRend::GetColor()
{
	isRendered_flag = true;
	if(type == RenderTypes::IMAGE)
	{
		const float* colors = ((ComponentImage*)cmp)->GetColor();
		return { colors[ComponentImage::Color::R], colors[ComponentImage::Color::G], colors[ComponentImage::Color::B], colors[ComponentImage::Color::A] }; 
	}
	else
	{
		return ((ComponentLabel*)cmp)->GetColor();
	}
}

uint ComponentCanvasRenderer::ToUIRend::GetTexture()
{
	isRendered_flag = true;
	return ((ComponentImage*)cmp)->GetResImage();
}

const char* ComponentCanvasRenderer::ToUIRend::GetText()
{
	isRendered_flag = true;
	return ((ComponentLabel*)cmp)->GetFinalText();
}

math::float2 ComponentCanvasRenderer::ToUIRend::GetMaskValues()
{
	if (((ComponentImage*)cmp)->useMask())
	{
		float* mask = ((ComponentImage*)cmp)->GetMask();
		return { mask[0], mask[1] };
	}
	else
		return { -1.0f, -1.0f };
}

std::vector<ComponentLabel::LabelLetter>* ComponentCanvasRenderer::ToUIRend::GetWord()
{
	isRendered_flag = true;
	return ((ComponentLabel*)cmp)->GetLetterQueue();
}
