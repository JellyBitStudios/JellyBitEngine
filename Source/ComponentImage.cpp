#include "ComponentImage.h"

#include "Application.h"
#include "ModuleUI.h"
#include "ModuleResourceManager.h"

#include "ResourceTexture.h"

#include "GameObject.h"

#include "ComponentRectTransform.h"

#include "imgui\imgui.h"
#include "imgui\imgui_internal.h"

ComponentImage::ComponentImage(GameObject * parent, ComponentTypes componentType, bool includeComponents) : Component(parent, ComponentTypes::ImageComponent)
{
	if (includeComponents)
	{
		if (parent->cmp_rectTransform == nullptr)
			parent->AddComponent(ComponentTypes::RectTransformComponent);

		if (parent->cmp_canvasRenderer == nullptr)
			parent->AddComponent(ComponentTypes::CanvasRendererComponent);
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
	}
}

ComponentImage::~ComponentImage()
{
	if (res_image > 0)
		App->res->SetAsUnused(res_image);

	parent->cmp_image = nullptr;
}

const float * ComponentImage::GetColor() const
{
	return color;
}

void ComponentImage::SetResImageUuid(uint res_image_uuid)
{
	if (res_image > 0)
		App->res->SetAsUnused(res_image);

	res_image = res_image_uuid;

	if (res_image > 0)
		App->res->SetAsUsed(res_image);
}

uint ComponentImage::GetResImageUuid() const
{
	return res_image;
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
		uint* rect = parent->cmp_rectTransform->GetRect();
		rect_initValues[0] = rect[ComponentRectTransform::Rect::XDIST];
		rect_initValues[1] = rect[ComponentRectTransform::Rect::YDIST];
	}
}

void ComponentImage::RectChanged()
{
	if (mask)
	{
		uint* rect = parent->cmp_rectTransform->GetRect();
		mask_values[1] = ((rect_initValues[1] - (float)rect[ComponentRectTransform::Rect::YDIST]) / rect_initValues[1]);
		mask_values[0] = 1.0f - ((rect_initValues[0] - (float)rect[ComponentRectTransform::Rect::XDIST]) / rect_initValues[0]);
	}
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
	ImGui::Text("Image Component");
	ImGui::Spacing();

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
			App->res->SetAsUnused(res_image);
			res_image = *(uint*)payload->Data;
			App->res->SetAsUsed(res_image);
		}
		ImGui::EndDragDropTarget();
	}
	ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	if (ImGui::Checkbox("M", &mask))
		SetMask();
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::Text("Mask\nActivate if the size is final.");
		ImGui::EndTooltip();
	}
	ImGui::SameLine(); ImGui::PushItemWidth(10.0f);
	if (ImGui::Button("RC"))
	{
		color[Color::R] = 1.0f;
		color[Color::G] = 1.0f;
		color[Color::B] = 1.0f;
		color[Color::A] = 1.0f;
	}
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::Text("Reset color");
		ImGui::EndTooltip();
	}
	ImGui::SameLine(); ImGui::PushItemWidth(10.0f);
	if (ImGui::Button("CT"))
	{
		if(res_image > 0)
			App->res->SetAsUnused(res_image);
		res_image = 0;
	}
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::Text("Clear Texture");
		ImGui::EndTooltip();
	}
#endif
}