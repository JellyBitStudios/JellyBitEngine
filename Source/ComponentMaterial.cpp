#include "ComponentMaterial.h"
#include "GameObject.h"
#include "MaterialImporter.h"

#include "imgui/imgui.h"

ComponentMaterial::ComponentMaterial(GameObject* parent) : Component(parent, ComponentType::Material_Component) {}

ComponentMaterial::ComponentMaterial(const ComponentMaterial& componentMaterial) : Component(componentMaterial.parent, ComponentType::Material_Component)
{
	res = componentMaterial.res;
}

ComponentMaterial::~ComponentMaterial()
{
	parent->materialRenderer = nullptr;
}

void ComponentMaterial::Update() {}

void ComponentMaterial::OnUniqueEditor()
{
	ImGui::Text("Material:");

	ImGui::SameLine();

	ImGui::ColorButton("##currentMaterial", ImVec4(0.0f, 0.0f, 0.0f, 0.213f), ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_AlphaPreview, ImVec2(16, 16));

	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		
		ImGui::Text("%u", (res.size() > 0) ? res.front() : 0);
		ImGui::EndTooltip();
	}

	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MESH_INSPECTOR_SELECTOR"))
		{

		}
		ImGui::EndDragDropTarget();
	}
}

void ComponentMaterial::OnInternalSave(JSON_Object* file)
{
	json_object_set_number(file, "ResourceMaterial", 0012013);
}

void ComponentMaterial::OnLoad(JSON_Object* file)
{
	//LOAD MATERIAL
}