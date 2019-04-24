#include "ComponentInterpolation.h"

#include "Application.h"
#include "ModuleInput.h"
#include "ModuleTimeManager.h"

#include "GameObject.h"
#include "ComponentTransform.h"

#include "Brofiler/Brofiler.h"

#include "imgui\imgui.h"



ComponentInterpolation::ComponentInterpolation(GameObject * parent) : Component(parent, ComponentTypes::TrailComponent)
{
	startPoint.name = "_startPoint";
}

ComponentInterpolation::ComponentInterpolation(const ComponentInterpolation& componentTrail, GameObject* parent) : Component(parent, ComponentTypes::TrailComponent)
{

}

ComponentInterpolation::~ComponentInterpolation()
{
	for (std::list<TransNode*>::iterator node = nodes.begin(); node != nodes.end(); ++node)
	{
		delete *node;
		*node = nullptr;
	}
}

void ComponentInterpolation::Update()
{
#ifndef GAMEMODE
	BROFILER_CATEGORY(__FUNCTION__, Profiler::Color::PapayaWhip);
#endif // !GAMEMODE
	if (move && currentNode != nullptr && !finished)
	{
		currTime += App->timeManager->GetDt();
		float normalized = (currTime * speed) / ((*nodes.begin())->distance);

		if (normalized >= 1.0f)
		{
			move = false;
			goBackTime.Start();

			if (!goBack)
				finished = true;
		}

		else
		{
			math::float3 newPos = parent->transform->GetPosition().Lerp((*nodes.begin())->position, normalized);
			math::Quat newRot = parent->transform->GetRotation().Lerp((*nodes.begin())->rotation, normalized);
			math::float3 newSize = parent->transform->GetScale().Lerp((*nodes.begin())->scale, normalized);

			parent->transform->SetPosition(newPos);
			parent->transform->SetRotation(newRot);
			parent->transform->SetScale(newSize);
		}
	}
	else if (goBack)
	{
		if (goBackTime.Read() > waitTime)
		{
			currentNode = &startPoint;
			move = true;
			goBack = false;
			finished = true;
		}
	}
}


void ComponentInterpolation::OnUniqueEditor()
{
#ifndef GAMEMODE

	if (ImGui::CollapsingHeader("Interpolation", ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_DefaultOpen))
	{
		for (std::list<TransNode*>::iterator node = nodes.begin(); node != nodes.end(); ++node)
		{
			ImGui::CollapsingHeader((*node)->name)
		}
	}

#endif
}


void ComponentInterpolation::StartInterpolation(char* nodeName, bool goBack, float time)
{
	for (std::list<TransNode*>::iterator node = nodes.begin(); node != nodes.end(); ++node)
	{
		if (std::strcmp((*node)->name.c_str, nodeName) == 0)
		{
			currentNode = *node;

			move = true;
			currTime = 0;
			this->goBack = goBack;
			waitTime = time;

			startPoint.position = parent->transform->GetPosition();
			startPoint.rotation = parent->transform->GetRotation();
			startPoint.scale = parent->transform->GetScale();

			break;
		}
	}

	currentNode = nullptr;
	move = false;
	this->goBack = false;
	waitTime = 0;
}

void ComponentInterpolation::GoBack()
{
	currentNode = &startPoint;
	move = true;
	goBack = false;
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
