#include "ComponentLabel.h"
#include "ComponentRectTransform.h"

#include "ModuleUI.h"
#include "ModuleResourceManager.h"

#include "ResourceFont.h"

#include "GameObject.h"
#include "Application.h"

#include "glew/include/GL/glew.h"
#include "MathGeoLib/include/Geometry/Frustum.h"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imgui_stl.h"

ComponentLabel::ComponentLabel(GameObject * parent, ComponentTypes componentType, bool includeComponents) : Component(parent, ComponentTypes::LabelComponent)
{
	if (includeComponents)
	{
		if (parent->cmp_rectTransform == nullptr)
			parent->AddComponent(ComponentTypes::RectTransformComponent);

		if (parent->cmp_canvasRenderer == nullptr)
			parent->AddComponent(ComponentTypes::CanvasRendererComponent);

		std::vector<Resource*> fonts = App->res->GetResourcesByType(ResourceTypes::FontResource);
		fontUuid = !fonts.empty() ? fonts[0]->GetUuid() : 0u;
		if (fontUuid)
			size = ((ResourceFont*)fonts[0])->fontData.fontSize;

		needed_recaclculate = true;
	}
}

ComponentLabel::ComponentLabel(const ComponentLabel & componentLabel, GameObject* parent, bool includeComponents) : Component(parent, ComponentTypes::LabelComponent)
{
	if (includeComponents)
	{
		color = componentLabel.color;
		labelWord = componentLabel.labelWord;
		finalText = componentLabel.finalText;
		//memcpy(text, finalText.data(), finalText.length());
		fontUuid = componentLabel.fontUuid;
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

		ResourceFont* fontRes = fontUuid != 0 ? (ResourceFont*)App->res->GetResource(fontUuid) : nullptr;

		if (fontRes && !fontRes->fontData.charactersMap.empty())
		{
			uint x_moving = 0;
			uint* rectParent = parent->cmp_rectTransform->GetRect();
			math::float3* parentCorners = parent->cmp_rectTransform->GetCorners();
			x_moving = rectParent[0];
			float sizeNorm = size / (float)fontRes->fontData.fontSize;
			uint contRows = 0;

			for (std::string::const_iterator c = finalText.begin(); c != finalText.end(); ++c)
			{
				if ((int)(*c) >= 32 && (int)(*c) < 128)//ASCII TABLE
				{
					LabelLetter l;
					memcpy(&l.letter, c._Ptr, sizeof(char));

					Character character;
					character = fontRes->fontData.charactersMap.find(*c)->second;

					l.textureID = character.textureID;

					uint x = x_moving + character.bearing.x * sizeNorm;
					//								Normalize pos with all heights	 //	Check Y-ofset for letters that write below origin "p" //	 Control lines enters
					uint y = rectParent[1] + ((fontRes->fontData.maxCharHeight - character.size.y) + ((character.size.y) - character.bearing.y)) * sizeNorm + contRows * fontRes->fontData.maxCharHeight * sizeNorm;

					if (x + character.size.x * sizeNorm > rectParent[0] + rectParent[2])
					{
						y += fontRes->fontData.maxCharHeight * sizeNorm;
						x = rectParent[0] + character.bearing.x * sizeNorm;
						contRows++;
					}

					if (parent->cmp_rectTransform->GetFrom() == ComponentRectTransform::RectFrom::RECT)
						ScreenDraw(l.rect, x, y, character.size, sizeNorm);

					else
						WorldDraw(parentCorners, l.corners, rectParent, x, y, character.size, sizeNorm);

					x_moving += character.advance * sizeNorm;

					labelWord.push_back(l);
				}
				else if ((int)(*c) == 10)//"\n"
				{
					contRows++;
					x_moving = rectParent[0];
				}
			}
		}
		needed_recaclculate = false;
	}
}

void ComponentLabel::WorldDraw(math::float3 * parentCorners, math::float3 corners[4], uint * rectParent, const uint x, const uint y, math::float2 characterSize, float sizeNorm)
{
	math::float3 xDirection = (parentCorners[0] - parentCorners[1]).Normalized();
	math::float3 yDirection = (parentCorners[2] - parentCorners[0]).Normalized();

	corners[1] = parentCorners[1] + (xDirection * ((float)(x - rectParent[0]) / WORLDTORECT)) + (yDirection * ((float)(y - rectParent[1]) / WORLDTORECT));
	corners[0] = corners[1] + (xDirection * (characterSize.x * sizeNorm / WORLDTORECT));
	corners[2] = corners[0] + (yDirection * (characterSize.y * sizeNorm / WORLDTORECT));
	corners[3] = corners[2] - (xDirection * (characterSize.x * sizeNorm / WORLDTORECT));

	math::float3 zDirection = xDirection.Cross(yDirection);

	float z = ZSEPARATOR + parent->cmp_rectTransform->GetZ();
	corners[1] -= zDirection * z;
	corners[0] -= zDirection * z;
	corners[2] -= zDirection * z;
	corners[3] -= zDirection * z;
}

void ComponentLabel::ScreenDraw(uint rect[4], const uint x, const uint y, math::float2 characterSize, float sizeNorm)
{
	rect[0] = x;
	rect[1] = y;
	rect[2] = characterSize.x * sizeNorm;
	rect[3] = characterSize.y * sizeNorm;
}

uint ComponentLabel::GetInternalSerializationBytes()
{
	return sizeof(color) + sizeof(int) + sizeof(uint) * 2 + sizeof(char) * finalText.length();
}

void ComponentLabel::OnInternalSave(char *& cursor)
{
	size_t bytes = sizeof(math::float4);
	memcpy(cursor, &color, bytes);
	cursor += bytes;

	bytes = sizeof(int);
	memcpy(cursor, &size, bytes);
	cursor += bytes;

	bytes = sizeof(uint);
	memcpy(cursor, &fontUuid, bytes);
	cursor += bytes;

	uint nameLenght = finalText.length();
	memcpy(cursor, &nameLenght, bytes);
	cursor += bytes;

	bytes = nameLenght;
	memcpy(cursor, finalText.c_str(), bytes);
	cursor += bytes;
}

void ComponentLabel::OnInternalLoad(char *& cursor)
{
	size_t bytes = sizeof(math::float4);
	memcpy(&color, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(int);
	memcpy(&size, cursor, bytes);
	cursor += bytes;

	//Load lenght + string
	bytes = sizeof(uint);
	memcpy(&fontUuid, cursor, bytes);
	cursor += bytes;

	uint nameLenght;
	memcpy(&nameLenght, cursor, bytes);
	cursor += bytes;

	bytes = nameLenght;
	finalText.resize(nameLenght);
	memcpy((void*)finalText.c_str(), cursor, bytes);
	finalText.resize(nameLenght);
	cursor += bytes;

	//sprintf(text, "%s", finalText.data());
}

void ComponentLabel::OnUniqueEditor()
{
#ifndef GAMEMODE
	if (ImGui::CollapsingHeader("Label", ImGuiTreeNodeFlags_DefaultOpen))
	{
		float sizeX = ImGui::GetWindowWidth();
		if (ImGui::InputTextMultiline("##source", &finalText, ImVec2(sizeX, ImGui::GetTextLineHeight() * 7), ImGuiInputTextFlags_AllowTabInput))
		{
			needed_recaclculate = true;
		}

		ImGui::PushItemWidth(200.0f);
		ImGui::ColorEdit4("Color", &color.x, ImGuiColorEditFlags_AlphaBar);

		if (ImGui::DragInt("Load new size", &size, 1.0f, 0, 72))
			needed_recaclculate = true;

		//-----------------------------------------

		DragDropFont();
	}
#endif
}

void ComponentLabel::DragDropFont()
{
#ifndef GAMEMODE
	ImGui::Separator();
	uint buttonWidth = 0.60 * ImGui::GetWindowWidth();
	ImVec2 cursorPos = ImGui::GetCursorScreenPos();
	ImGui::SetCursorScreenPos({ cursorPos.x, cursorPos.y + 5 });

		ImGui::Text("Font: "); ImGui::SameLine();

		cursorPos = ImGui::GetCursorScreenPos();
		ImGui::SetCursorScreenPos({ cursorPos.x, cursorPos.y - 5 });

		cursorPos = { cursorPos.x, cursorPos.y - 5 };

		ImGui::ButtonEx("##Font", { (float)buttonWidth, 20 }, ImGuiButtonFlags_::ImGuiButtonFlags_Disabled);

		//Case 1: Dragging Real GameObjects
		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FONT_RESOURCE", ImGuiDragDropFlags_AcceptBeforeDelivery | ImGuiDragDropFlags_AcceptNoDrawDefaultRect);
			if (payload)
			{
				uint uuid = *(uint*)payload->Data;

				if (ImGui::IsMouseReleased(0))
				{
					fontUuid = uuid;
					needed_recaclculate = true;
					size = ((ResourceFont*)App->res->GetResource(uuid))->fontData.fontSize;
				}
			}
			ImGui::EndDragDropTarget();
		}
	

	//Text in quat
	std::string name;
	ResourceFont* font = nullptr;
	if (fontUuid > 0)
	{
		font = (ResourceFont*)App->res->GetResource(fontUuid);
		if(font)
			name = font->GetName();
	}
	std::string originalText = (fontUuid > 0 && font) ? name : "Waiting font...";
	std::string clampedText;

	ImVec2 textSize = ImGui::CalcTextSize(originalText.data());

	if (textSize.x > buttonWidth - 7)
	{
		uint maxTextLenght = originalText.length() * (buttonWidth - 7) / textSize.x;
		clampedText = originalText.substr(0, maxTextLenght - 7);
		clampedText.append("(...)");
	}
	else
		clampedText = originalText;

	cursorPos = { cursorPos.x + 12, cursorPos.y + 3 };
	ImGui::SetCursorScreenPos(cursorPos);
	ImGui::Text(clampedText.data());
#endif
}

void ComponentLabel::SetFinalText(const char * newText)
{
	finalText = newText;
	needed_recaclculate = true;
}

const char * ComponentLabel::GetFinalText() const
{
	return finalText.data();
}

std::vector<ComponentLabel::LabelLetter>* ComponentLabel::GetLetterQueue()
{
	return &labelWord;
}

void ComponentLabel::SetColor(math::float4 newColor)
{
	color = newColor;
}

math::float4 ComponentLabel::GetColor() const
{
	return color;
}

void ComponentLabel::RectChanged()
{
	needed_recaclculate = true;
}
