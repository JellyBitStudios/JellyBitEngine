#include "ComponentLight.h"
#include "Application.h"
#include "Lights.h"

#include "imgui/imgui.h"

#include "Globals.h"

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

	behaviour = componentLight.behaviour;
	lightBB = componentLight.lightBB;

	App->lights->AddLight(this);
}

ComponentLight::~ComponentLight()
{
	App->lights->EraseLight(this);
}

void ComponentLight::OnUniqueEditor()
{
#ifndef GAMEMODE
	if (ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_DefaultOpen))
	{
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
		if (ImGui::Checkbox("Behaviour", &behaviour))
		{
			if (lightBB.size() == 0)
			{
				LightBehaviourBlock block;
				memcpy(block.color, color, sizeof(float) * 3);
				lightBB.push_back(block);
			}
			if (behaviour)
			{
				currentLightBB = 0;
				nextInterval = RandomFloat(lightBB[currentLightBB].interval,
					lightBB[currentLightBB].interval -
					lightBB[currentLightBB].elpasedInterval);
				// set everything
				enabled = lightBB[currentLightBB].enabled;
				quadratic = lightBB[currentLightBB].quadratic;
				linear = lightBB[currentLightBB].linear;
			}
		}
		static bool hideBB = false;
		ImGui::Checkbox("Show Behaviour", &hideBB);
		if (hideBB)
		{
			char name[INPUT_BUF_SIZE];
			for (int i = 0; i < lightBB.size();)
			{
				ImGui::Separator();
				sprintf_s(name, INPUT_BUF_SIZE, "LightBB color ##%i", i);
				ImGui::ColorEdit3(name, (float*)&lightBB[i].color, misc_flags);
				sprintf_s(name, INPUT_BUF_SIZE, "LightBB enabled ##%i", i);
				ImGui::Checkbox(name, &lightBB[i].enabled);
				ImGui::PushItemWidth(50.0f);
				sprintf_s(name, INPUT_BUF_SIZE, "LightBB interval ##%i", i);
				ImGui::DragFloat(name, &lightBB[i].interval, 0.1f, 0.0f, 1000.0f);
				sprintf_s(name, INPUT_BUF_SIZE, "LightBB elpasedInterval ##%i", i);
				ImGui::DragFloat(name, &lightBB[i].elpasedInterval, 0.1f, 0.0f, lightBB[i].interval);
				if (lightType == LightTypes::PointLight)
				{
					sprintf_s(name, INPUT_BUF_SIZE, "LightBB quadratic ##%i", i);
					ImGui::DragFloat(name, &lightBB[i].quadratic, 0.01f, 0.0f, 1.0f);
					sprintf_s(name, INPUT_BUF_SIZE, "LightBB linear ##%i", i);
					ImGui::DragFloat(name, &lightBB[i].linear, 0.01f, 0.0f, 1.0f);
					ImGui::PopItemWidth();
				}
				if (i != 0)
				{
					sprintf_s(name, INPUT_BUF_SIZE, "Delete Block ##%i", i);
					if (ImGui::Button(name))
						lightBB.erase(lightBB.begin() + i);
					else
						++i;
				}
				else
					++i;
			}
			if (ImGui::Button("Add BB"))
			{
				LightBehaviourBlock block;
				memcpy(block.color, lightBB[lightBB.size() - 1].color, sizeof(float) * 3);
				lightBB.push_back(block);
			}
		}
		else
		{
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
		}
	}
#endif
}

void ComponentLight::Update()
{
	if (behaviour)
	{
		timer += App->GetDt();
		if (timer >= nextInterval)
		{
			timer = 0.0f;
			currentLightBB += 1;
			if (currentLightBB >= lightBB.size())
				currentLightBB = 0;

			nextInterval = RandomFloat(lightBB[currentLightBB].interval,
									   lightBB[currentLightBB].interval -
									   lightBB[currentLightBB].elpasedInterval);
			// set everything
			enabled = lightBB[currentLightBB].enabled;
			quadratic = lightBB[currentLightBB].quadratic;
			linear = lightBB[currentLightBB].linear;
			memcpy(color, lightBB[currentLightBB].color, sizeof(float) * 3);
		}
	}
}

uint ComponentLight::GetInternalSerializationBytes()
{										 // behaviour
	return sizeof(int) + sizeof(float) * 5 + sizeof(int) + (sizeof(float) * 7 + sizeof(bool)) * lightBB.size() + sizeof(bool);
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

	//lights bb
	bytes = sizeof(int);
	int totalBlock = lightBB.size();
	memcpy(cursor, &totalBlock, bytes);
	cursor += bytes;
	for each (LightBehaviourBlock block in lightBB)
	{
		bytes = sizeof(float);
		memcpy(cursor, block.color, bytes * 3);
		cursor += bytes * 3;
		memcpy(cursor, &block.elpasedInterval, bytes);
		cursor += bytes;
		memcpy(cursor, &block.interval, bytes);
		cursor += bytes;
		memcpy(cursor, &block.linear, bytes);
		cursor += bytes;
		memcpy(cursor, &block.quadratic, bytes);
		cursor += bytes;
		bytes = sizeof(bool);
		memcpy(cursor, &block.enabled, bytes);
		cursor += bytes;
	}

	bytes = sizeof(bool);
	memcpy(cursor, &behaviour, bytes);
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

	//lightbb
	bytes = sizeof(int);
	int totalBlock;
	memcpy(&totalBlock, cursor, bytes);
	cursor += bytes;
	lightBB.resize(totalBlock);

	for (int i = 0; i < totalBlock; ++i)
	{
		bytes = sizeof(float);
		memcpy(lightBB[i].color, cursor, bytes * 3);
		cursor += bytes * 3;
		memcpy(&lightBB[i].elpasedInterval, cursor, bytes);
		cursor += bytes;
		memcpy(&lightBB[i].interval, cursor, bytes);
		cursor += bytes;
		memcpy(&lightBB[i].linear, cursor, bytes);
		cursor += bytes;
		memcpy(&lightBB[i].quadratic, cursor, bytes);
		cursor += bytes;
		bytes = sizeof(bool);
		memcpy(&lightBB[i].enabled, cursor, bytes);
		cursor += bytes;
	}

	bytes = sizeof(bool);
	memcpy(&behaviour, cursor, bytes);
	cursor += bytes;

	if (behaviour)
	{
		currentLightBB = 0;
		nextInterval = RandomFloat(lightBB[currentLightBB].interval,
			lightBB[currentLightBB].interval -
			lightBB[currentLightBB].elpasedInterval);
		// set everything
		enabled = lightBB[currentLightBB].enabled;
		quadratic = lightBB[currentLightBB].quadratic;
		linear = lightBB[currentLightBB].linear;
	}
}
