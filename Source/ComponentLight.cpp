#include "ComponentLight.h"
#include "Application.h"
#include "Lights.h"

#include "imgui/imgui.h"

ComponentLight::ComponentLight(GameObject* parent) : Component(parent, ComponentTypes::LightComponent)
{
	memset(color, 0, sizeof(float) * 3);
	App->lights->AddLight(this);
}

ComponentLight::ComponentLight(const ComponentLight& componentLight, GameObject* parent) : Component(parent, ComponentTypes::LightComponent)
{
	lightType = componentLight.lightType;
	intensity = componentLight.intensity;
	App->lights->AddLight(this);
}

ComponentLight::~ComponentLight()
{
	App->lights->EraseLight(this);
}

void ComponentLight::OnUniqueEditor()
{
#ifndef GAMEMODE
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Type");
	ImGui::SameLine();
	ImGui::PushItemWidth(100.0f);
	const char* lights[] = { "Directional", "Point", "Spot" };
	ImGui::Combo("##Light Type", (int*)&lightType, lights, IM_ARRAYSIZE(lights));
	ImGui::PopItemWidth();
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Intensity");
	ImGui::SameLine();
	ImGui::PushItemWidth(30.0f);
	ImGui::DragScalar("##Light Intesity", ImGuiDataType_::ImGuiDataType_U32, &intensity, 1.0f);
	ImGui::PopItemWidth();
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Color");
	ImGui::SameLine();
	int misc_flags = ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_AlphaPreview |
					 ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel;
	ImGui::ColorEdit3("Color", (float*)&color, misc_flags);
#endif
}

uint ComponentLight::GetInternalSerializationBytes()
{
	return sizeof(int) * 2;
}

void ComponentLight::OnInternalSave(char*& cursor)
{
	size_t bytes = sizeof(int);
	memcpy(cursor, &lightType, bytes);
	memcpy(cursor, &intensity, bytes);
}

void ComponentLight::OnInternalLoad(char*& cursor)
{
	size_t bytes = sizeof(int);
	memcpy(&lightType, cursor, bytes);
	memcpy(&intensity, cursor, bytes);
}
