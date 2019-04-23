#include "ComponentInterpolation.h"

#include "Application.h"
#include "ModuleInput.h"
#include "ModuleResourceManager.h"
#include "ModuleInternalResHandler.h"

#include "GameObject.h"
#include "ComponentTransform.h"
//#include "ModuleTrails.h"

#include "Brofiler/Brofiler.h"

#include "imgui\imgui.h"



ComponentInterpolation::ComponentInterpolation(GameObject * parent) : Component(parent, ComponentTypes::TrailComponent)
{

}

ComponentInterpolation::ComponentInterpolation(const ComponentInterpolation& componentTrail, GameObject* parent) : Component(parent, ComponentTypes::TrailComponent)
{

}

ComponentInterpolation::~ComponentInterpolation()
{

}

void ComponentInterpolation::Update()
{
#ifndef GAMEMODE
	BROFILER_CATEGORY(__FUNCTION__, Profiler::Color::PapayaWhip);
#endif // !GAMEMODE

}


void ComponentInterpolation::OnUniqueEditor()
{
#ifndef GAMEMODE

	if (ImGui::CollapsingHeader("Interpolation", ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_DefaultOpen))
	{

	}

#endif
}


uint ComponentInterpolation::GetInternalSerializationBytes()
{
	// Todo
	return 0u;
}

void ComponentInterpolation::OnInternalSave(char *& cursor)
{
	// Todo
	//size_t bytes = sizeof(materialRes);
	//memcpy(cursor, &materialRes, bytes);
	//cursor += bytes;
}

void ComponentInterpolation::OnInternalLoad(char *& cursor)
{
	// Todo
	//size_t bytes = sizeof(materialRes);
	//memcpy(&materialRes, cursor, bytes);
	//cursor += bytes;
}
