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

		needed_recalculate = true;
	}
}

ComponentLabel::ComponentLabel(const ComponentLabel & componentLabel, GameObject* parent, bool includeComponents) : Component(parent, ComponentTypes::LabelComponent)
{
	color = componentLabel.color;
	labelWord = componentLabel.labelWord;
	finalText = componentLabel.finalText;
	fontUuid = componentLabel.fontUuid;
	size = componentLabel.size;
	labelWord = componentLabel.labelWord;
	textureWord = componentLabel.textureWord;

	if (includeComponents)
		if (!labelWord.empty())
			FIllBuffer();
}

ComponentLabel::~ComponentLabel()
{
	parent->cmp_label = nullptr;
}

void ComponentLabel::OnSystemEvent(System_Event event)
{
	switch (event.type)
	{
	case System_Event_Type::CanvasChanged:
	case System_Event_Type::RectTransformUpdated:
	{
		needed_recalculate = true;
		break;
	}
	case System_Event_Type::ScreenChanged:
	{
		//change size by z & y
		break;
	}
	}
}

void ComponentLabel::Update()
{
	if (needed_recalculate)
	{
		labelWord.clear();
		textureWord.clear();

		ResourceFont* fontRes = fontUuid != 0 ? (ResourceFont*)App->res->GetResource(fontUuid) : nullptr;

		if (fontRes && !fontRes->fontData.charactersMap.empty())
		{
			uint x_moving = 0;
			uint* rectParent = parent->cmp_rectTransform->GetRect();
			math::float3* parentCorners = parent->cmp_rectTransform->GetCorners();
			x_moving = rectParent[X_UI_RECT];
			float sizeNorm = size / (float)fontRes->fontData.fontSize;
			uint contRows = 0;

			for (std::string::const_iterator c = finalText.begin(); c != finalText.end(); ++c)
			{
				if ((int)(*c) >= 32 && (int)(*c) < 128)//ASCII TABLE
				{
					LabelLetter l;

					Character character;
					character = fontRes->fontData.charactersMap.find(*c)->second;

					l.textureID = character.textureID;

					uint x = x_moving + character.bearing.x * sizeNorm;
					//								Normalize pos with all heights	 //	Check Y-ofset for letters that write below origin "p" //	 Control lines enters
					uint y = rectParent[Y_UI_RECT] + ((fontRes->fontData.maxCharHeight - character.size.y) + ((character.size.y) - character.bearing.y)) * sizeNorm + contRows * fontRes->fontData.maxCharHeight * sizeNorm;

					if (x + character.size.x * sizeNorm > rectParent[X_UI_RECT] + rectParent[W_UI_RECT])
					{
						y += fontRes->fontData.maxCharHeight * sizeNorm;
						x = rectParent[X_UI_RECT] + character.bearing.x * sizeNorm;
						x_moving = x;
						contRows++;
					}

					if (parent->cmp_rectTransform->GetFrom() == ComponentRectTransform::RectFrom::RECT)
						ScreenDraw(l.corners, x, y, character.size, sizeNorm);

					else
						WorldDraw(parentCorners, l.corners, rectParent, x, y, character.size, sizeNorm);

					if (y + character.size.y * sizeNorm < rectParent[Y_UI_RECT] + rectParent[H_UI_RECT])
					{
						labelWord.push_back(l);
						textureWord.push_back(l.textureID);
					}
					else
						break;

					x_moving += character.advance * sizeNorm;
				}
				else if ((int)(*c) == 10)//"\n"
				{
					contRows++;
					x_moving = rectParent[X_UI_RECT];
				}
			}
			if (!labelWord.empty())
				FIllBuffer();
			else
				last_word_size = 0;
		}

		needed_recalculate = false;
	}
}

void ComponentLabel::WorldDraw(math::float3 * parentCorners, math::float4 corners[4], uint * rectParent, const uint x, const uint y, math::float2 characterSize, float sizeNorm)
{
	math::float3 xDirection = (parentCorners[CORNER_TOP_LEFT] - parentCorners[CORNER_TOP_RIGHT]).Normalized();
	math::float3 yDirection = (parentCorners[2] - parentCorners[CORNER_TOP_LEFT]).Normalized();

	math::float3 pos = parentCorners[CORNER_TOP_RIGHT];
	pos = pos + (xDirection * ((float)(x - rectParent[X_UI_RECT]) / WORLDTORECT)) + (yDirection * ((float)(y - rectParent[Y_UI_RECT]) / WORLDTORECT));
	corners[CORNER_TOP_RIGHT] = { pos, 1.0f };
	pos = corners[CORNER_TOP_RIGHT].xyz();
	pos = pos + (xDirection * (characterSize.x * sizeNorm / WORLDTORECT));
	corners[CORNER_TOP_LEFT] = { pos, 1.0f };
	pos = corners[CORNER_TOP_LEFT].xyz();
	pos = pos + (yDirection * (characterSize.y * sizeNorm / WORLDTORECT));
	corners[CORNER_BOTTOM_LEFT] = { pos, 1.0f };
	pos = corners[CORNER_BOTTOM_LEFT].xyz();
	pos = pos - (xDirection * (characterSize.x * sizeNorm / WORLDTORECT));
	corners[CORNER_BOTTOM_RIGHT] = { pos, 1.0f };

	math::float3 zDirection = xDirection.Cross(yDirection);
	float z = ZSEPARATOR + parent->cmp_rectTransform->GetZ();
	for (uint i = 0; i < 4; ++i) //Change All Corners (TopLeft / TopRight / BottomLeft / BottomRight)
		corners[i] -= { zDirection * z , 0.0f };
}

void ComponentLabel::ScreenDraw(math::float4 corners[4], const uint x, const uint y, math::float2 characterSize, float sizeNorm)
{
	uint* screen = App->ui->GetScreen();
	uint w_width = screen[ModuleUI::Screen::WIDTH];
	uint w_height = screen[ModuleUI::Screen::HEIGHT];

	corners[ComponentRectTransform::Rect::RBOTTOMLEFT] = { math::Frustum::ScreenToViewportSpace({ (float)x, (float)y }, w_width, w_height), 0.0f, 1.0f };
	corners[ComponentRectTransform::Rect::RBOTTOMRIGHT] = { math::Frustum::ScreenToViewportSpace({ (float)x + (float)characterSize.x * sizeNorm, (float)y }, w_width, w_height), 0.0f, 1.0f };
	corners[ComponentRectTransform::Rect::RTOPLEFT] = { math::Frustum::ScreenToViewportSpace({ (float)x, (float)y + (float)characterSize.y * sizeNorm }, w_width, w_height), 0.0f, 1.0f };
	corners[ComponentRectTransform::Rect::RTOPRIGHT] = { math::Frustum::ScreenToViewportSpace({ (float)x + (float)characterSize.x * sizeNorm, (float)y + (float)characterSize.y * sizeNorm }, w_width, w_height), 0.0f, 1.0f };
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

	needed_recalculate = true;
}

void ComponentLabel::OnUniqueEditor()
{
#ifndef GAMEMODE
	if (ImGui::CollapsingHeader("Label", ImGuiTreeNodeFlags_DefaultOpen))
	{
		float sizeX = ImGui::GetWindowWidth();
		if (ImGui::InputTextMultiline("##source", &finalText, ImVec2(sizeX, ImGui::GetTextLineHeight() * 7), ImGuiInputTextFlags_AllowTabInput))
		{
			needed_recalculate = true;
		}

		ImGui::PushItemWidth(200.0f);
		ImGui::ColorEdit4("Color", &color.x, ImGuiColorEditFlags_AlphaBar);

		if (ImGui::DragInt("Load new size", &size, 1.0f, 0, 72))
			needed_recalculate = true;

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
					needed_recalculate = true;
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
	std::string originalText = "Waiting font...";
	if (fontUuid > 0 && font)
		originalText = name;
	else if (!labelWord.empty())
		labelWord.clear();
	
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

void ComponentLabel::FIllBuffer()
{
	if (labelWord.size() != last_word_size)
	{
		if (buffer)
			RELEASE_ARRAY(buffer);
		buffer_size = labelWord.size() * sizeof(float) * 16;
		buffer = new char[buffer_size];
	}
	char* cursor = buffer;
	size_t bytes = sizeof(float) * 4;
	for (uint i = 0; i < labelWord.size(); i++)
	{
		memcpy(cursor, &labelWord[i].corners[ComponentRectTransform::Rect::RTOPLEFT], bytes);
		cursor += bytes;
		memcpy(cursor, &labelWord[i].corners[ComponentRectTransform::Rect::RTOPRIGHT], bytes);
		cursor += bytes;
		memcpy(cursor, &labelWord[i].corners[ComponentRectTransform::Rect::RBOTTOMLEFT], bytes);
		cursor += bytes;
		memcpy(cursor, &labelWord[i].corners[ComponentRectTransform::Rect::RBOTTOMRIGHT], bytes);
		cursor += bytes;
	}
	last_word_size = labelWord.size();
}

void ComponentLabel::SetFinalText(const char * newText)
{
	finalText = newText;
	needed_recalculate = true;
}

const char * ComponentLabel::GetFinalText() const
{
	return finalText.data();
}

void ComponentLabel::SetColor(math::float4 newColor)
{
	color = newColor;
}

math::float4 ComponentLabel::GetColor() const
{
	return color;
}

char* ComponentLabel::GetBuffer()
{
	return buffer;
}

uint ComponentLabel::GetBufferSize() const
{
	return buffer_size;
}

uint ComponentLabel::GetWordSize() const
{
	return labelWord.size();
}

std::vector<uint>* ComponentLabel::GetWordTextureIDs()
{
	return &textureWord;
}
