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
		rect = new ComponentRectTransform(parent, RectTransformComponent, ComponentRectTransform::RectFrom::RECT);
	}
}

ComponentLabel::ComponentLabel(const ComponentLabel & componentLabel, GameObject* parent, bool includeComponents) : Component(parent, ComponentTypes::LabelComponent)
{
	if (includeComponents)
	{
		charactersBitmap = componentLabel.charactersBitmap;
		rect = new ComponentRectTransform(*componentLabel.rect, parent, includeComponents);
	}
}

ComponentLabel::~ComponentLabel()
{
	parent->cmp_label = nullptr;
}

void ComponentLabel::Draw(uint ui_shader, uint VAO)
{
	uint* rectParent = parent->cmp_rectTransform->GetRect();
	rect->SetRectPos(rectParent[0], rectParent[1]);
	rect->Update();
	float sizeNorm = size / (float)sizeLoaded;
	uint contRows = 0;
	if (!charactersBitmap.empty())
		for (std::string::const_iterator c = finalText.begin(); c != finalText.end(); ++c)
		{

			if ((int)(*c) >= 32 && (int)(*c) < 128)//ASCII TABLE
			{
				Character character;
				character = charactersBitmap.find(*c)->second;

				uint x = rect->GetRect()[0] + character.bearing.x * sizeNorm;
				//								Normalize pos with all heights	 //	Check Y-ofset for letters that write below origin "p" //	 Control lines enters
				uint y = rect->GetRect()[1] + ((maxLabelSize - character.size.y) + ((character.size.y) - character.bearing.y)) * sizeNorm + contRows * maxLabelSize * sizeNorm;

				if (x + character.size.x * sizeNorm > rectParent[0] + rectParent[2])
				{
					y += maxLabelSize * sizeNorm;
					x = rectParent[0] + character.bearing.x * sizeNorm;
					contRows++;
				}
				rect->SetRectPos(x,y);
				rect->SetRectDim(character.size.x * sizeNorm, character.size.y * sizeNorm);
				rect->Update();

				if (rect->IsInRect(rectParent))
				{
					//Shader
					glUseProgram(ui_shader);
					SetRectToShader(ui_shader);
					glUniform1i(glGetUniformLocation(ui_shader, "using_texture"), 0);
					glUniform1i(glGetUniformLocation(ui_shader, "isLabel"), 1);

					glUniform4f(glGetUniformLocation(ui_shader, "spriteColor"), color.x, color.y, color.z, color.w);

					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, character.textureID);
					glUniform1ui(glGetUniformLocation(ui_shader, "image"), 0);

					glBindVertexArray(VAO);
					glDrawArrays(GL_TRIANGLES, 0, 6);

					glBindTexture(GL_TEXTURE_2D, 0);
					glBindVertexArray(0);

					glUseProgram(0);

					rect->SetRectPos(rect->GetRect()[0] + character.advance * sizeNorm, rectParent[1]);
					rect->Update();
				}
				else
					break;
			}
			else if ((int)(*c) == 10)//"\n"
			{
				contRows++;
				rect->SetRectPos(rectParent[0], rectParent[1]);
				rect->Update();
			}
		}
}

void ComponentLabel::Update()
{
}

uint ComponentLabel::GetInternalSerializationBytes()
{
																			//SIZES		//SizeMap +  sizeString
	return sizeof(color) + sizeof(char) * finalText.length() + sizeof(int) * 2 + sizeof(uint) * 3 + sizeof(Character)*(charactersBitmap.size()+1);
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
	ImGui::InputTextMultiline("##source", text, IM_ARRAYSIZE(text), ImVec2(sizeX, ImGui::GetTextLineHeight()*7), ImGuiInputTextFlags_AllowTabInput);
	finalText = text;

	ImGui::PushItemWidth(200.0f);
	ImGui::ColorEdit4("Color", &color.x, ImGuiColorEditFlags_AlphaBar);

	ImGui::Separator();
	ImGui::DragInt("Load new size", &size, 1.0f, 0, 72);
	if (ImGui::Button("Fix new size", ImVec2(125, 20)))
	{
		charactersBitmap.clear();
		maxLabelSize = App->ft->LoadFont("../Game/Assets/Textures/Font/arial.ttf", size, charactersBitmap);
		sizeLoaded = size;
	}

#endif
}

const char * ComponentLabel::GetFinalText() const
{
	return finalText.data();
}

void ComponentLabel::SetRectToShader(uint shader)
{
	uint* rect_points = nullptr;
	math::float2 pos;
	float w_width;
	float w_height;

	rect_points = rect->GetRect();
	glUniform1i(glGetUniformLocation(shader, "isScreen"), 1);
	glUniform1i(glGetUniformLocation(shader, "useMask"), 0);

	w_width = App->ui->GetRectUI()[ModuleUI::Screen::WIDTH];
	w_height = App->ui->GetRectUI()[ModuleUI::Screen::HEIGHT];

	pos = math::Frustum::ScreenToViewportSpace({ (float)rect_points[0], (float)rect_points[1] }, w_width, w_height);
	glUniform3f(glGetUniformLocation(shader, "topLeft"), pos.x, pos.y, 0.0f);
	pos = math::Frustum::ScreenToViewportSpace({ (float)rect_points[0] + (float)rect_points[2], (float)rect_points[1] }, w_width, w_height);
	glUniform3f(glGetUniformLocation(shader, "topRight"), pos.x, pos.y, 0.0f);
	pos = math::Frustum::ScreenToViewportSpace({ (float)rect_points[0], (float)rect_points[1] + (float)rect_points[3] }, w_width, w_height);
	glUniform3f(glGetUniformLocation(shader, "bottomLeft"), pos.x, pos.y, 0.0f);
	pos = math::Frustum::ScreenToViewportSpace({ (float)rect_points[0] + (float)rect_points[2], (float)rect_points[1] + (float)rect_points[3] }, w_width, w_height);
	glUniform3f(glGetUniformLocation(shader, "bottomRight"), pos.x, pos.y, 0.0f);
}
