#include "ComponentImage.h"

#include "Application.h"
#include "ModuleUI.h"
#include "ModuleResourceManager.h"

#include "ResourceTexture.h"

#include "GameObject.h"

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

ComponentImage::ComponentImage(const ComponentImage & componentRectTransform, GameObject* parent, bool includeComponents) : Component(parent, ComponentTypes::ImageComponent)
{
	memcpy(color, componentRectTransform.color, sizeof(float) * 4);
	res_image = componentRectTransform.res_image;
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

uint ComponentImage::GetInternalSerializationBytes()
{
	return sizeof(float) * 4 + sizeof(uint);
}

void ComponentImage::OnInternalSave(char *& cursor)
{
	size_t bytes = sizeof(float) * 4;
	memcpy(cursor, &color, bytes);
	cursor += bytes;

	bytes = sizeof(uint);
	memcpy(cursor, &res_image, bytes);
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
	ImGui::Button(texture == nullptr ? "Empty texture" : texture->GetName(), ImVec2(150.0f, 0.0f));

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
	ImGui::SameLine(); ImGui::PushItemWidth(10.0f);
	if (ImGui::Button("RC"))
	{
		color[COLOR_R] = 1.0f;
		color[COLOR_G] = 1.0f;
		color[COLOR_B] = 1.0f;
		color[COLOR_A] = 1.0f;
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