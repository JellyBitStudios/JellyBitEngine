#include "ComponentLight.h"
#include "Application.h"
#include "Lights.h"

#include "imgui/imgui.h"

ComponentLight::ComponentLight(GameObject* parent) : Component(parent, ComponentTypes::LightComponent)
{
	// Default color
	color[0] = 1.0f;
	color[1] = 0.9568627f;
	color[2] = 0.8392157f;
	App->lights->AddLight(this);
}

ComponentLight::ComponentLight(const ComponentLight& componentLight, GameObject* parent) : Component(parent, ComponentTypes::LightComponent)
{
	lightType = componentLight.lightType;
	quadratic = componentLight.quadratic;
	linear = componentLight.linear;
	memcpy(color, componentLight.color, sizeof(float) * 3);
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
	const char* lights[] = { "", "Directional", "Point", "Spot" };
	ImGui::Combo("##Light Type", (int*)&lightType, lights, IM_ARRAYSIZE(lights));
	ImGui::PopItemWidth();
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Color");
	ImGui::SameLine();
	int misc_flags = ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_AlphaPreview |
		ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel;
	ImGui::ColorEdit3("Color", (float*)&color, misc_flags);
	ImGui::AlignTextToFramePadding();
	if (lightType == LightTypes::PointLight)
	{
		ImGui::Text("Constants");
		ImGui::Text("Quadratic");
		ImGui::SameLine();
		ImGui::PushItemWidth(50.0f);
		int min = 0, max = 1;
		ImGui::DragFloat("##Light Quadratic", &linear, 0.01f, 0.0f, 1.0f);
		ImGui::PopItemWidth();
		ImGui::Text("Linear");
		ImGui::SameLine();
		ImGui::PushItemWidth(50.0f);
		ImGui::DragFloat("##Light Linaer", &quadratic, 0.01f, 0.0f, 1.0f);
		ImGui::PopItemWidth();
	}
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
	cursor += bytes;
	bytes = sizeof(float) * 3;
	memcpy(cursor, color, sizeof(float) * 3);
	cursor += bytes;
	bytes = sizeof(float);
	memcpy(cursor, &quadratic, bytes);
	cursor += bytes;
	memcpy(cursor, &linear, bytes);
	cursor += bytes;
}

void ComponentLight::OnInternalLoad(char*& cursor)
{
	size_t bytes = sizeof(int);
	memcpy(&lightType, cursor, bytes);
	cursor += bytes;
	bytes = sizeof(float) * 3;
	memcpy(color, cursor, sizeof(float) * 3);
	cursor += bytes;
	bytes = sizeof(float);
	memcpy(&quadratic, cursor, bytes);
	cursor += bytes;
	memcpy(&linear, cursor, bytes);
	cursor += bytes;
}
