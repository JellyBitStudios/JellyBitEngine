#include "ComponentLabel.h"
#include "ComponentRectTransform.h"

#include "ModuleFreetype.h"
#include "ModuleUI.h"

#include "GameObject.h"
#include "Application.h"

#include "glew/include/GL/glew.h"
#include "MathGeoLib/include/Geometry/Frustum.h"
#include "imgui/imgui.h"

ComponentLabel::ComponentLabel(GameObject * parent, ComponentTypes componentType, bool includeComponents) : Component(parent, ComponentTypes::LabelComponent)
{
	if (includeComponents)
	{
		if (parent->cmp_rectTransform == nullptr)
			parent->AddComponent(ComponentTypes::RectTransformComponent);

		if (parent->cmp_canvasRenderer == nullptr)
			parent->AddComponent(ComponentTypes::CanvasRendererComponent);

		maxLabelSize = App->ft->LoadFont("../Game/Assets/Textures/Font/arial.ttf", size, charactersBitmap);

		finalText = text;

		needed_recaclculate = true;
	}
}

ComponentLabel::ComponentLabel(const ComponentLabel & componentLabel, GameObject* parent, bool includeComponents) : Component(parent, ComponentTypes::LabelComponent)
{
	if (includeComponents)
	{
		charactersBitmap = componentLabel.charactersBitmap;		
		color = componentLabel.color;
		labelWord = componentLabel.labelWord;
		finalText = componentLabel.finalText;
		memcpy(text, finalText.data(), finalText.length());
	}
}

ComponentLabel::~ComponentLabel()
{
	parent->cmp_label = nullptr;
}

void ComponentLabel::Update()
{
	if (needed_recaclculate)
	{
		labelWord.clear();

		uint x_moving = 0;
		uint* rectParent = parent->cmp_rectTransform->GetRect();
		math::float3* parentCorners = parent->cmp_rectTransform->GetCorners();
		x_moving = rectParent[0];
		float sizeNorm = size / (float)sizeLoaded;
		uint contRows = 0;
		if (!charactersBitmap.empty())
			for (std::string::const_iterator c = finalText.begin(); c != finalText.end(); ++c)
			{
				if ((int)(*c) >= 32 && (int)(*c) < 128)//ASCII TABLE
				{
					LabelLetter l;
					memcpy(&l.letter, c._Ptr, sizeof(char));

					Character character;
					character = charactersBitmap.find(*c)->second;

					l.textureID = character.textureID;

					uint x = x_moving + character.bearing.x * sizeNorm;
					//								Normalize pos with all heights	 //	Check Y-ofset for letters that write below origin "p" //	 Control lines enters
					uint y = rectParent[1] + ((maxLabelSize - character.size.y) + ((character.size.y) - character.bearing.y)) * sizeNorm + contRows * maxLabelSize * sizeNorm;

					if (x + character.size.x * sizeNorm > rectParent[0] + rectParent[2])
					{
						y += maxLabelSize * sizeNorm;
						x = rectParent[0] + character.bearing.x * sizeNorm;
						contRows++;
					}

					if (parent->cmp_rectTransform->GetFrom() == ComponentRectTransform::RectFrom::RECT)
						ScreenDraw(l, x, y, character, sizeNorm);

					else
						WorldDraw(parentCorners, l, x, rectParent, y, character, sizeNorm);

					x_moving += character.advance * sizeNorm;

					labelWord.push_back(l);
				}
				else if ((int)(*c) == 10)//"\n"
				{
					contRows++;
					x_moving = rectParent[0];
				}
			}

		needed_recaclculate = false;
	}
}

void ComponentLabel::WorldDraw(math::float3 * parentCorners, ComponentLabel::LabelLetter &l, const uint x, uint * rectParent, const uint y, Character character, float sizeNorm)
{
	math::float3 xDirection = (parentCorners[0] - parentCorners[1]).Normalized();
	math::float3 yDirection = (parentCorners[2] - parentCorners[3]).Normalized();

	l.corners[1] = parentCorners[1] + (xDirection * ((float)(x - rectParent[0]) / WORLDTORECT)) + (yDirection * ((float)(y - rectParent[1]) / WORLDTORECT));
	l.corners[0] = l.corners[1] + (xDirection * ((float)character.size.x * sizeNorm / WORLDTORECT));
	l.corners[2] = l.corners[0] + (yDirection * ((float)character.size.y * sizeNorm / WORLDTORECT));
	l.corners[2] = l.corners[2] - (xDirection * ((float)character.size.x * sizeNorm / WORLDTORECT));

	math::float3 zDirection = xDirection.Cross(yDirection);

	float z = ZSEPARATOR + parent->cmp_rectTransform->GetZ();
	l.corners[1] -= zDirection * z;
	l.corners[0] -= zDirection * z;
	l.corners[2] -= zDirection * z;
	l.corners[3] -= zDirection * z;
}

void ComponentLabel::ScreenDraw(ComponentLabel::LabelLetter &l, const uint x, const uint y, Character character, float sizeNorm)
{
	l.rect[0] = x;
	l.rect[1] = y;
	l.rect[2] = character.size.x * sizeNorm;
	l.rect[3] = character.size.y * sizeNorm;
}

uint ComponentLabel::GetInternalSerializationBytes()
{
																			//SIZES		//SizeMap +  sizeString
	return sizeof(color) + sizeof(char) * finalText.length() + sizeof(int) * 2 + sizeof(uint) * 3 + sizeof(Character)*(charactersBitmap.size());
}

void ComponentLabel::OnInternalSave(char *& cursor)
{
	size_t bytes = sizeof(math::float4);
	memcpy(cursor, &color, bytes);
	cursor += bytes;

	bytes = sizeof(int);
	memcpy(cursor, &size, bytes);
	cursor += bytes;

	memcpy(cursor, &sizeLoaded, bytes);
	cursor += bytes;

	bytes = sizeof(uint);
	uint nameLenght = finalText.length();
	memcpy(cursor, &nameLenght, bytes);
	cursor += bytes;

	bytes = nameLenght;
	memcpy(cursor, finalText.c_str(), bytes);
	cursor += bytes;

	bytes = sizeof(uint);
	memcpy(cursor, &maxLabelSize, bytes);
	cursor += bytes;

	uint listSize = charactersBitmap.size();
	memcpy(cursor, &listSize, bytes);
	cursor += bytes;

	bytes = sizeof(Character);
	for (std::map<char, Character>::iterator it = charactersBitmap.begin(); it != charactersBitmap.end(); ++it)
	{
		memcpy(cursor, &(*it).second, bytes);
		cursor += bytes;
	}
}

void ComponentLabel::OnInternalLoad(char *& cursor)
{
	size_t bytes = sizeof(math::float4);
	memcpy(&color, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(int);
	memcpy(&size, cursor, bytes);
	cursor += bytes;

	memcpy(&sizeLoaded, cursor, bytes);
	cursor += bytes;

	//Load lenght + string
	bytes = sizeof(uint);
	uint nameLenght;
	memcpy(&nameLenght, cursor, bytes);
	cursor += bytes;

	bytes = nameLenght;
	finalText.resize(nameLenght);
	memcpy((void*)finalText.c_str(), cursor, bytes);
	finalText.resize(nameLenght);
	cursor += bytes;

	uint i = 0;
	for (std::string::iterator it = finalText.begin(); it != finalText.end(); ++it, ++i)
	{
		text[i] = *it;
	}
	bytes = sizeof(uint);
	memcpy(&maxLabelSize, cursor, bytes);
	cursor += bytes;

	uint listSize = charactersBitmap.size();
	memcpy(&listSize, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(Character);
	for (int i = 0; i < listSize; ++i)
	{
		Character character;
		memcpy(&character, cursor, bytes);
		charactersBitmap.insert(std::pair<char, Character>(i + 32, character));
		cursor += bytes;
	}

}

void ComponentLabel::OnUniqueEditor()
{
#ifndef GAMEMODE
	ImGui::Text("Text");
	ImGui::Separator();

	float sizeX = ImGui::GetWindowWidth();
	if (ImGui::InputTextMultiline("##source", text, IM_ARRAYSIZE(text), ImVec2(sizeX, ImGui::GetTextLineHeight() * 7), ImGuiInputTextFlags_AllowTabInput))
	{
		finalText = text;
		needed_recaclculate = true;
	}

	ImGui::PushItemWidth(200.0f);
	ImGui::ColorEdit4("Color", &color.x, ImGuiColorEditFlags_AlphaBar);

	ImGui::Separator();
	if (ImGui::DragInt("Load new size", &size, 1.0f, 0, 72))
		needed_recaclculate = true;
	if (ImGui::Button("Fix new size", ImVec2(125, 20)))
	{
		charactersBitmap.clear();
		maxLabelSize = App->ft->LoadFont("../Game/Assets/Textures/Font/arial.ttf", size, charactersBitmap);
		sizeLoaded = size;
		needed_recaclculate = true;
	}

#endif
}

const char * ComponentLabel::GetFinalText() const
{
	return finalText.data();
}

std::vector<ComponentLabel::LabelLetter>* ComponentLabel::GetLetterQueue()
{
	return &labelWord;
}

math::float4 ComponentLabel::GetColor() const
{
	return color;
}

void ComponentLabel::RectChanged()
{
	needed_recaclculate = true;
}
