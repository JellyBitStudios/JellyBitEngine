#include "ComponentLabel.h"
#include "ComponentRectTransform.h"

#include "ModuleFreetype.h"
#include "ModuleUI.h"

#include "GameObject.h"
#include "Application.h"

#include "glew/include/GL/glew.h"
#include "MathGeoLib/include/Geometry/Frustum.h"
#include "imgui/imgui.h"

ComponentLabel::ComponentLabel(GameObject * parent, ComponentTypes componentType) : Component(parent, ComponentTypes::LabelComponent)
{
	App->ui->componentsUI.push_back(this);
	App->ft->LoadFont("../Game/Assets/Textures/Font/arial.ttf", size, charactersBitmap);
	rect = new ComponentRectTransform(parent, RectTransformComponent,ComponentRectTransform::RectFrom::RECT);
}

ComponentLabel::ComponentLabel(const ComponentLabel & componentLabel, GameObject* parent, bool includeComponents) : Component(parent, ComponentTypes::LabelComponent)
{
	if (includeComponents)
	{
		App->ui->componentsUI.push_back(this);
		charactersBitmap = componentLabel.charactersBitmap;
		rect = new ComponentRectTransform(*componentLabel.rect, parent, includeComponents);
	}
}

ComponentLabel::~ComponentLabel()
{
	App->ui->componentsUI.remove(this);
	RELEASE(rect);
}

void ComponentLabel::Draw(uint ui_shader, uint VAO)
{
	uint* rectParent = parent->cmp_rectTransform->GetRect();
	rect->SetRect(rectParent[0], rectParent[1], 0, 0);
	float sizeNorm = size / (float)sizeLoaded;

	for (std::string::const_iterator c = finalText.begin(); c != finalText.end(); ++c) 
	{

		if ((int)(*c) >= 0 && (int)(*c) < 128 )
		{
			Character character;
			character = charactersBitmap.find(*c)->second;

			uint x = rect->GetRect()[0] + character.bearing.x;
			uint y = rect->GetRect()[1] + (character.size.y - character.bearing.y);

			rect->SetRect(x, y, character.size.x * sizeNorm, character.size.y * sizeNorm);

			//Shader
			glUseProgram(ui_shader);
			SetRectToShader(ui_shader);
			glUniform1i(glGetUniformLocation(ui_shader, "use_color"), 1);
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
		}
	}
}

void ComponentLabel::Update()
{
}

uint ComponentLabel::GetInternalSerializationBytes()
{
	return 0u;
}

void ComponentLabel::OnInternalSave(char *& cursor)
{
}

void ComponentLabel::OnInternalLoad(char *& cursor)
{}

void ComponentLabel::OnUniqueEditor()
{
#ifndef GAMEMODE
	ImGui::Text("Text");
	ImGui::Separator();

	static char text[300] = "Edit Text";
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
		App->ft->LoadFont("../Game/Assets/Textures/Font/arial.ttf", size, charactersBitmap);
		sizeLoaded = size;
	}

#endif
}

const char * ComponentLabel::GetFinalText() const
{
	return finalText.data();
}

void ComponentLabel::LinkToUIModule()
{
	App->ui->componentsUI.push_back(this);
	rect = new ComponentRectTransform(parent, RectTransformComponent, ComponentRectTransform::RectFrom::RECT);
}

void ComponentLabel::SetRectToShader(uint shader)
{
	uint* rect_points = nullptr;
	math::float2 pos;
	float w_width;
	float w_height;

	rect_points = rect->GetRect();
	glUniform1i(glGetUniformLocation(shader, "isScreen"), 1);

	w_width = App->ui->GetRectUI()[ModuleUI::Screen::WIDTH];
	w_height = App->ui->GetRectUI()[ModuleUI::Screen::HEIGHT];

	pos = math::Frustum::ScreenToViewportSpace({ (float)rect_points[0], (float)rect_points[1] }, w_width, w_height);
	glUniform3f(glGetUniformLocation(shader, "bottomLeft"), pos.x, pos.y, 0.0f);
	pos = math::Frustum::ScreenToViewportSpace({ (float)rect_points[0] + (float)rect_points[2], (float)rect_points[1] }, w_width, w_height);
	glUniform3f(glGetUniformLocation(shader, "bottomRight"), pos.x, pos.y, 0.0f);
	pos = math::Frustum::ScreenToViewportSpace({ (float)rect_points[0], (float)rect_points[1] + (float)rect_points[3] }, w_width, w_height);
	glUniform3f(glGetUniformLocation(shader, "topLeft"), pos.x, pos.y, 0.0f);
	pos = math::Frustum::ScreenToViewportSpace({ (float)rect_points[0] + (float)rect_points[2], (float)rect_points[1] + (float)rect_points[3] }, w_width, w_height);
	glUniform3f(glGetUniformLocation(shader, "topRight"), pos.x, pos.y, 0.0f);
}