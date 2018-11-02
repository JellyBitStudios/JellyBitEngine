#include "Component.h"
#include "GameObject.h"

#include "imgui/imgui.h"

Component::Component(GameObject* parent, ComponentType type) : parent(parent), type(type) {}

Component::~Component() {}

void Component::Update() {}

void Component::OnEditor()
{
	if (ImGui::Button("Delete"))
		GetParent()->MarkToDeleteComponentByValue(this);

	OnUniqueEditor();
}

void Component::OnUniqueEditor() {}

ComponentType Component::GetType() const
{
	return type;
}

void Component::SetParent(GameObject* parent)
{
	this->parent = parent;
}

GameObject* Component::GetParent() const
{
	return parent;
}

void Component::OnSave(JSON_Object* file)
{
	json_object_set_number(file, "Type", type);
	OnInternalSave(file);
}