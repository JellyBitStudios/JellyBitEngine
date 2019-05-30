#include "ComponentCanvasRenderer.h"

#include "Application.h"
#include "ModuleUI.h"

#include "GameObject.h"

#include "ComponentImage.h"
#include "ComponentRectTransform.h"
#include "ComponentLabel.h"
#include "ComponentSlider.h"

#ifndef GAMEMODE
#include "imgui\imgui.h"
#include "imgui\imgui_internal.h"
#endif

ComponentCanvasRenderer::ComponentCanvasRenderer(GameObject * parent, ComponentTypes componentType, bool includeComponents) : Component(parent, ComponentTypes::CanvasRendererComponent)
{
	if (includeComponents)
	{
		rend_queue.push_back(new ToUIRend());
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
				{
					rend->Set(RenderTypes::IMAGE, cmp_image);
					break;
				}
			}
		}
	ComponentLabel* cmp_label = (ComponentLabel*)parent->GetComponent(ComponentTypes::LabelComponent);
	if (cmp_label)
		if (cmp_label->IsTreeActive())
		{
			for (ToUIRend* rend : rend_queue)
			{
				if (rend->isRendered())
				{
					rend->Set(RenderTypes::LABEL, cmp_label);
					break;
				}
			}
		}
	ComponentSlider* cmp_slider = (ComponentSlider*)parent->GetComponent(ComponentTypes::SliderComponent);
	if (cmp_slider)
		if (cmp_slider->IsTreeActive())
		{
			for (ToUIRend* rend : rend_queue)
			{
				if (rend->isRendered())
				{
					rend->Set(RenderTypes::SLIDER, cmp_slider);
					break;
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

	for (ToUIRend* rend : rend_queue)
	{
		if (!destroyed && !parent->cmp_image && !parent->cmp_label && !parent->cmp_slider)
		{
			parent->DestroyComponent(this);
			destroyed = true;
		}
	}
	#endif
}

//Rend Queue Struct

math::float4 ComponentCanvasRenderer::ToUIRend::GetColor()
{
	isRendered_flag = true;

	switch (type)
	{
	case ComponentCanvasRenderer::IMAGE:
	{
		const float* colors = ((ComponentImage*)cmp)->GetColor();
		return { colors[ComponentImage::Color::R], colors[ComponentImage::Color::G], colors[ComponentImage::Color::B], colors[ComponentImage::Color::A] };
		break;
	}
	case ComponentCanvasRenderer::LABEL:
	{
		return ((ComponentLabel*)cmp)->GetColor();
		break;
	}
	default:
		return { 1.0f, 1.0f, 1.0f, 1.0f };
		break;
	}
}

uint ComponentCanvasRenderer::ToUIRend::GetTexture()
{
	return ((ComponentImage*)cmp)->GetResImage();
}

math::float2 ComponentCanvasRenderer::ToUIRend::GetMaskValues()
{
	if (type == SLIDER)
		return { ((ComponentSlider*)cmp)->GetPercentage(), 0.0f };
	else
	{
		if (((ComponentImage*)cmp)->useMask())
		{
			float* mask = ((ComponentImage*)cmp)->GetMask();
			return { mask[0], mask[1] };
		}
		else
			return { -1.0f, -1.0f };
	}
}

std::vector<LabelLetter*>* ComponentCanvasRenderer::ToUIRend::GetWord()
{
	return ((ComponentLabel*)cmp)->GetWord();
}

int ComponentCanvasRenderer::ToUIRend::GetIndex() const
{
	if (type == RenderTypes::IMAGE)
		return ((ComponentImage*)cmp)->GetBufferIndex();
	else
		return ((ComponentLabel*)cmp)->GetBufferIndex();
}
