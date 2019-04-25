#include "ComponentInterpolation.h"

#include "Application.h"
#include "ModuleInput.h"
#include "ModuleTimeManager.h"

#include "ModuleInterpolation.h"

#include "GameObject.h"
#include "ComponentTransform.h"

#include "Brofiler/Brofiler.h"

#include "imgui\imgui.h"



ComponentInterpolation::ComponentInterpolation(GameObject * parent) : Component(parent, ComponentTypes::InterpolationComponent)
{
	startPoint.name = "_startPoint";

	App->interpolation->interpolations.push_back(this);
}

ComponentInterpolation::ComponentInterpolation(const ComponentInterpolation& componentTrail, GameObject* parent) : Component(parent, ComponentTypes::InterpolationComponent)
{

}

ComponentInterpolation::~ComponentInterpolation()
{
	for (std::list<TransNode*>::iterator node = nodes.begin(); node != nodes.end(); ++node)
	{
		delete *node;
		*node = nullptr;
	}

	nodes.clear();

	App->interpolation->interpolations.remove(this);

}

void ComponentInterpolation::Update()
{
#ifndef GAMEMODE
	BROFILER_CATEGORY(__FUNCTION__, Profiler::Color::PapayaWhip);
#endif // !GAMEMODE

	if (move && !finished)
	{
		float dt = 0.0f;
		dt = App->GetDt();
#ifdef GAMEMODE
		dt = App->timeManager->GetDt();
#endif // GAMEMODE

		currTime += dt;
		float normalized = (currTime * speed) / (currentNode.distance);

		if (normalized >= 1.0f)
		{
			move = false;
			goBackTime.Start();

			if (!goBack || goingBack)
				finished = true;
		}

		else
		{
			math::float3 newPos = startPoint.position.Lerp(currentNode.position, normalized);
			math::Quat newRot = startPoint.rotation.Lerp(currentNode.rotation, normalized);
			math::float3 newSize = startPoint.scale.Lerp(currentNode.scale, normalized);

			parent->transform->SetPosition(newPos);
			parent->transform->SetRotation(newRot);
			parent->transform->SetScale(newSize);
		}
	}
	else if (goBack)
	{
		if (goBackTime.Read() > waitTime)
		{
			GoBack();
		}
	}
}


void ComponentInterpolation::OnUniqueEditor()
{
#ifndef GAMEMODE

	if (ImGui::CollapsingHeader("Interpolation", ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_DefaultOpen))
	{
		std::list<std::list<TransNode*>::iterator> toDelete;

		ImGui::DragFloat("Speed", &speed);
		ImGui::DragFloat("Min distance", &minDist);
		int id = 0;
		for (std::list<TransNode*>::iterator node = nodes.begin(); node != nodes.end(); ++node)
		{
			if (ImGui::CollapsingHeader((*node)->name.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
			{
				static char nodeName[INPUT_BUF_SIZE];

				strcpy_s(nodeName, IM_ARRAYSIZE(nodeName), (*node)->name.c_str());

				ImGui::PushItemWidth(100.0f);
				ImGuiInputTextFlags inputFlag = ImGuiInputTextFlags_EnterReturnsTrue;
				std::string _name = "##nodeName" + std::to_string(id);
				if (ImGui::InputText(_name.c_str(), nodeName, IM_ARRAYSIZE(nodeName), inputFlag))
				{
					(*node)->name = nodeName;
				}

				ImGui::Text("Position");
				std::string _pos = "##nodePos" + std::to_string(id);
				ImGui::DragFloat3(_pos.c_str() , &(*node)->position[0], 0.01f, 0.0f, 0.0f, "%.3f");

				ImGui::Text("Rotation");
				math::float3 axis;
				float angle;
				(*node)->rotation.ToAxisAngle(axis, angle);
				axis *= angle;
				axis *= RADTODEG;
				std::string _rot = "##nodeRot" + std::to_string(id);
				if (ImGui::DragFloat3(_rot.c_str(), &axis[0], 0.1f, 0.0f, 0.0f, "%.3f"))
				{
					axis *= DEGTORAD;
					(*node)->rotation.SetFromAxisAngle(axis.Normalized(), axis.Length());
				}

				ImGui::Text("Scale");
				std::string _scale = "##nodeScale" + std::to_string(id);
				ImGui::DragFloat3(_scale.c_str() , &(*node)->scale[0], 0.01f, 0.0f, 0.0f, "%.3f");

				if (parent->transform)
				{
					if (ImGui::Button("Set Current"))
					{
						(*node)->position = parent->transform->GetPosition();
						(*node)->rotation = parent->transform->GetRotation();
						(*node)->scale = parent->transform->GetScale();
					}
				}

				if (ImGui::Button("Remove"))
				{
					toDelete.push_back(node);
				}
			}
			id++;
		}

		for (std::list<std::list<TransNode*>::iterator>::iterator node = toDelete.begin(); node != toDelete.end(); ++node)
		{
			delete **node;
			**node = nullptr;
			nodes.erase(*node);
		}

		ImGui::Separator();

		if (ImGui::Button("Add new"))
		{
			TransNode* node = new TransNode();
			node->position = parent->transform->GetPosition();
			node->rotation = parent->transform->GetRotation();
			node->scale = parent->transform->GetScale();

			nodes.push_back(node);
		}
	}

#endif
}


void ComponentInterpolation::StartInterpolation(char* nodeName, bool goBack, float time)
{
	for (std::list<TransNode*>::iterator node = nodes.begin(); node != nodes.end(); ++node)
	{
		if (std::strcmp((*node)->name.c_str(), nodeName) == 0)
		{
			currentNode = **node;

			move = true;
			currTime = 0;
			this->goBack = goBack;
			waitTime = time;
			finished = false;
			goingBack = false;

			startPoint.position = parent->transform->GetPosition();
			startPoint.rotation = parent->transform->GetRotation();
			startPoint.scale = parent->transform->GetScale();

			currentNode.distance = startPoint.position.Distance(currentNode.position);

			return;
		}
	}

	currentNode;
	move = false;
	this->goBack = false;
	waitTime = 0;
}

void ComponentInterpolation::GoBack()
{
	TransNode tmp = currentNode;
	currentNode = startPoint;
	startPoint = tmp;

	currTime = 0;
	move = true;
	goBack = false;
	finished = false;
	goingBack = true;

	currentNode.distance = startPoint.position.Distance(currentNode.position);
}

uint ComponentInterpolation::GetNodesBytes()
{
	uint size = 0;

	for (std::list<TransNode*>::iterator node = nodes.begin(); node != nodes.end(); ++node)
	{
		size += (*node)->Size();
	}

	return size;
}

void ComponentInterpolation::SaveNodes(char* &cursor)
{
	for (std::list<TransNode*>::iterator node = nodes.begin(); node != nodes.end(); ++node)
	{
		(*node)->Save(cursor);
	}
}

void ComponentInterpolation::LoadNodes(char* &cursor, int size)
{
	for (std::list<TransNode*>::iterator node = nodes.begin(); node != nodes.end(); ++node)
	{
		delete *node;
		*node = nullptr;
	}
	nodes.clear();

	nodes.resize(size);


	for (std::list<TransNode*>::iterator node = nodes.begin(); node != nodes.end(); ++node)
	{
		(*node) = new TransNode(cursor);
	}
}


uint ComponentInterpolation::GetInternalSerializationBytes()
{
	return sizeof(speed) + sizeof(minDist) + sizeof(move) + sizeof(goBack) + sizeof(waitTime) + sizeof(int) + GetNodesBytes();
}

void ComponentInterpolation::OnInternalSave(char *& cursor)
{
	size_t bytes = sizeof(speed);
	memcpy(cursor, &speed, bytes);
	cursor += bytes;

	bytes = sizeof(minDist);
	memcpy(cursor, &minDist, bytes);
	cursor += bytes;

	bytes = sizeof(move);
	memcpy(cursor, &move, bytes);
	cursor += bytes;

	bytes = sizeof(goBack);
	memcpy(cursor, &goBack, bytes);
	cursor += bytes;

	bytes = sizeof(waitTime);
	memcpy(cursor, &waitTime, bytes);
	cursor += bytes;

	bytes = sizeof(int);
	int size = nodes.size();
	memcpy(cursor, &size, bytes);
	cursor += bytes;

	SaveNodes(cursor);
}

void ComponentInterpolation::OnInternalLoad(char *& cursor)
{
	size_t bytes = sizeof(speed);
	memcpy(&speed, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(minDist);
	memcpy(&minDist, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(move);
	memcpy(&move, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(goBack);
	memcpy(&goBack, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(waitTime);
	memcpy(&waitTime, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(int);
	int nodesSize = 0;
	memcpy(&nodesSize, cursor, bytes);
	cursor += bytes;

	LoadNodes(cursor, nodesSize);
}
