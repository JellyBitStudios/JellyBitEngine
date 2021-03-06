#include "ComponentImage.h"

#include "Application.h"
#include "ModuleResourceManager.h"

#include "ResourceTexture.h"

#include "GameObject.h"

#include "ComponentRectTransform.h"
#include "ComponentLabel.h"
#include "ComponentButton.h"
#include "ComponentUIAnimation.h"

#ifndef GAMEMODE
#include "imgui\imgui.h"
#include "imgui\imgui_internal.h"
#endif

ComponentImage::ComponentImage(GameObject * parent, ComponentTypes componentType, bool includeComponents) : Component(parent, ComponentTypes::ImageComponent)
{
	if (includeComponents)
	{
		if (parent->cmp_rectTransform == nullptr)
			parent->AddComponent(ComponentTypes::RectTransformComponent);

		if (parent->cmp_canvasRenderer == nullptr)
			parent->AddComponent(ComponentTypes::CanvasRendererComponent);

		App->glCache->RegisterBufferIndex(&offset, &index, ComponentTypes::ImageComponent, this);
		needed_recalculate = true;
	}
}

ComponentImage::ComponentImage(const ComponentImage & componentImage, GameObject* parent, bool includeComponents) : Component(parent, ComponentTypes::ImageComponent)
{
	res_image = componentImage.res_image;
	mask = componentImage.mask;
	memcpy(color, componentImage.color, sizeof(float) * 4);
	memcpy(mask_values, componentImage.mask_values, sizeof(float) * 2);
	memcpy(rect_initValues, componentImage.rect_initValues, sizeof(float) * 2);

	if (includeComponents)
	{
		if(res_image > 0)
			App->res->SetAsUsed(res_image);

		App->glCache->RegisterBufferIndex(&offset, &index, ComponentTypes::ImageComponent, this);
		needed_recalculate = true;
	}
}

ComponentImage::~ComponentImage()
{
	if (res_image > 0)
		App->res->SetAsUnused(res_image);

	if (index != -1)
		App->glCache->UnRegisterBufferIndex(offset, ComponentTypes::ImageComponent);

	parent->cmp_image = nullptr;
}

void ComponentImage::Update()
{
	if (needed_recalculate)
	{
		if (mask)
		{
			if (neeeded_reset_mask)
			{
				mask_values[0] = 1;
				mask_values[1] = 0;
				int* rect = parent->cmp_rectTransform->GetRect();
				rect_initValues[0] = rect[ComponentRectTransform::Rect::XDIST];
				rect_initValues[1] = rect[ComponentRectTransform::Rect::YDIST];
				neeeded_reset_mask = false;
			}
			else
			{
				int* rect = parent->cmp_rectTransform->GetRect();
				mask_values[0] = 1.0f - ((rect_initValues[0] - (float)rect[ComponentRectTransform::Rect::XDIST]) / rect_initValues[0]);
				mask_values[1] = ((rect_initValues[1] - (float)rect[ComponentRectTransform::Rect::YDIST]) / rect_initValues[1]);
			}
		}
		if(App->glCache->isShaderStorage() && parent->IsStatic() && index != -1)
			FillBuffer();

		needed_recalculate = false;
	}
}

float * ComponentImage::GetColor()
{
	return color;
}

void ComponentImage::SetColor(float r, float g, float b, float a, bool from_cmpUIAnimation)
{
	color[Color::R] = r;
	color[Color::G] = g;
	color[Color::B] = b;
	color[Color::A] = a;

	if (!from_cmpUIAnimation && parent->cmp_uiAnimation)
		parent->cmp_uiAnimation->SetInitAlpha(a);
}

void ComponentImage::ResetColor()
{
	color[Color::R] = 1.0f;
	color[Color::G] = 1.0f;
	color[Color::B] = 1.0f;
	color[Color::A] = 1.0f;
}

void ComponentImage::SetResImageUuid(uint res_image_uuid, bool isIdle)
{
	if (res_image != res_image_uuid)
	{
		if (res_image > 0)
			App->res->SetAsUnused(res_image);

		res_image = res_image_uuid;

		if (res_image > 0)
			App->res->SetAsUsed(res_image);

		if (parent->cmp_button && isIdle && res_image > 0)
			parent->cmp_button->SetIdleTexture(res_image);
	}
}

int ComponentImage::GetBufferIndex() const
{
	return index;
}

void ComponentImage::SetBufferRangeAndFIll(uint offs, int index)
{
	offset = offs;
	this->index = index;

	if (index != -1)
		FillBuffer();
}

uint ComponentImage::GetResImageUuid() const
{
	return res_image;
}

std::string ComponentImage::GetResImageName() const
{
	if (res_image != 0u)
	{
		ResourceTexture* texture = (ResourceTexture*)App->res->GetResource(res_image);
		if (texture)
			return texture->GetName();
		else
			return "";
	}

	return "";
}

void ComponentImage::SetResImageName(const std::string& name, bool isIdle)
{
	std::vector<Resource*> textures = App->res->GetResourcesByType(ResourceTypes::TextureResource);
	for (int i = 0; i < textures.size(); ++i)
	{
		ResourceTexture* texture = (ResourceTexture*)textures[i];
		if (strcmp(name.c_str(), texture->GetName())==0)
		{
			if (res_image != 0u)
				App->res->SetAsUnused(res_image);

			App->res->SetAsUsed(texture->GetUuid());
			res_image = texture->GetUuid();

			if (parent->cmp_button && isIdle && res_image > 0)
				parent->cmp_button->SetIdleTexture(res_image);
			break;
		}
	}
}

void ComponentImage::OnSystemEvent(System_Event event)
{
	switch (event.type)
	{
	case System_Event_Type::ScreenChanged:
	{
		neeeded_reset_mask = true;
		needed_recalculate = true;
		break;
	}
	case System_Event_Type::CanvasChanged:
	case System_Event_Type::RectTransformUpdatedFromAnimation:
	case System_Event_Type::RectTransformUpdated:
	case System_Event_Type::OnGOStatic:
	{
		needed_recalculate = true;
		break;
	}
	}
}

uint ComponentImage::GetResImage()const
{
	ResourceTexture* texture = (ResourceTexture*)App->res->GetResource(res_image);
	if (texture)
		return texture->GetId();
	else
		return 0;
}

void ComponentImage::SetMask()
{
	if (mask)
	{
		mask_values[0] = 1;
		mask_values[1] = 0;
		int* rect = parent->cmp_rectTransform->GetRect();
		rect_initValues[0] = rect[ComponentRectTransform::Rect::XDIST];
		rect_initValues[1] = rect[ComponentRectTransform::Rect::YDIST];
	}
}

void ComponentImage::ResetTexture()
{
	if (res_image > 0)
		App->res->SetAsUnused(res_image);
	res_image = 0;
}

void ComponentImage::SetAlpha(float alpha, bool from_cmpUIAnimation)
{
	if (alpha > 1.0f) alpha = 1.0;
	if (alpha < 0.0f) alpha = 0.0;
	color[Color::A] = alpha;

	if (!from_cmpUIAnimation && parent->cmp_uiAnimation)
		parent->cmp_uiAnimation->SetInitAlpha(alpha);
}

float ComponentImage::GetAlpha() const
{
	return color[Color::A];
}

bool ComponentImage::useMask() const
{
	return mask;
}

float * ComponentImage::GetMask()
{
	return mask_values;
}

uint ComponentImage::GetInternalSerializationBytes()
{
	return sizeof(float) * 8 + sizeof(bool) + sizeof(uint);
}

void ComponentImage::OnInternalSave(char *& cursor)
{
	size_t bytes = sizeof(float) * 4;
	memcpy(cursor, &color, bytes);
	cursor += bytes;

	bytes = sizeof(uint);
	memcpy(cursor, &res_image, bytes);
	cursor += bytes;

	bytes = sizeof(float) * 2;
	memcpy(cursor, &mask_values, bytes);
	cursor += bytes;

	memcpy(cursor, &rect_initValues, bytes);
	cursor += bytes;

	bytes = sizeof(bool);
	memcpy(cursor, &mask, bytes);
	cursor += bytes;
}

void ComponentImage::OnInternalLoad(char *& cursor)
{
	size_t bytes = sizeof(float) * 4;
	memcpy(&color, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(uint);
	memcpy(&res_image, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(float) * 2;
	memcpy(&mask_values, cursor, bytes);
	cursor += bytes;

	memcpy(&rect_initValues, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(bool);
	memcpy(&mask, cursor, bytes);
	cursor += bytes;

	if(res_image > 0)
		App->res->SetAsUsed(res_image);
}

void ComponentImage::OnUniqueEditor()
{
#ifndef GAMEMODE
	if (ImGui::CollapsingHeader("Image", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text("Color RGB with alpha");
		ImGui::PushItemWidth(200.0f);
		ImGui::ColorEdit4("##Color_RGBA", color, ImGuiColorEditFlags_::ImGuiColorEditFlags_AlphaBar);

		ResourceTexture* texture = nullptr;
		if (res_image != 0)
			texture = (ResourceTexture*)App->res->GetResource(res_image);
		ImGui::Button(texture == nullptr ? "Empty texture" : texture->GetName(), ImVec2(100.0f, 0.0f));

		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::Text("%u", res_image);
			ImGui::EndTooltip();
		}
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TEXTURE_INSPECTOR_SELECTOR"))
			{
				if(res_image > 0u)
					App->res->SetAsUnused(res_image);
				res_image = *(uint*)payload->Data;
				App->res->SetAsUsed(res_image);
			}
			ImGui::EndDragDropTarget();
		}
		ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
		bool tempMask = mask;
		if (ImGui::Checkbox("M", &tempMask))
		{
			if (tempMask)
			{
				mask = true;
				SetMask();
			}
			else
				mask = false;
		}

		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::Text("Mask\nActivate if the size is final.");
			ImGui::EndTooltip();
		}
		ImGui::SameLine(); ImGui::PushItemWidth(10.0f);
		if (ImGui::Button("RC"))
			ResetColor();
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::Text("Reset color");
			ImGui::EndTooltip();
		}
		ImGui::SameLine(); ImGui::PushItemWidth(10.0f);
		if (ImGui::Button("CT"))
		{
			if (res_image > 0)
				App->res->SetAsUnused(res_image);
			res_image = 0;
		}
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::Text("Clear Texture");
			ImGui::EndTooltip();
		}
		ImGui::PopItemWidth();
	}
#endif
}

void ComponentImage::FillBuffer()
{
	math::float3* rCorners = parent->cmp_rectTransform->GetCorners();
	float one = 1.0f;
	char* cursor = buffer;
	size_t bytes = sizeof(float) * 3;
	memcpy(cursor, &rCorners[ComponentRectTransform::Rect::RTOPLEFT], bytes);
	cursor += bytes; memcpy(cursor, &one, sizeof(float)); cursor += sizeof(float);
	memcpy(cursor, &rCorners[ComponentRectTransform::Rect::RTOPRIGHT], bytes);
	cursor += bytes; memcpy(cursor, &one, sizeof(float)); cursor += sizeof(float);
	memcpy(cursor, &rCorners[ComponentRectTransform::Rect::RBOTTOMLEFT], bytes);
	cursor += bytes; memcpy(cursor, &one, sizeof(float)); cursor += sizeof(float);
	memcpy(cursor, &rCorners[ComponentRectTransform::Rect::RBOTTOMRIGHT], bytes);
	cursor += bytes; memcpy(cursor, &one, sizeof(float)); cursor += sizeof(float);
	App->glCache->FillBufferRange(offset, UI_BYTES_RECT, buffer);
}
