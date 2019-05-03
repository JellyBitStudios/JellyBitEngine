#include "ComponentLabel.h"
#include "ComponentRectTransform.h"
#include "ComponentCanvas.h"
#include "ModuleResourceManager.h"

#include "ResourceFont.h"

#include "GameObject.h"
#include "Application.h"
#include "ModuleUI.h"

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

		App->glCache->RegisterBufferIndex(&offset, &index, ComponentTypes::LabelComponent, this);

		internalGO = new GameObject("Label", nullptr, true);
		internalGO->SetParent(parent);

		new_word = true;
	}
}

ComponentLabel::ComponentLabel(const ComponentLabel & componentLabel, GameObject* parent, bool includeComponents) : Component(parent, ComponentTypes::LabelComponent)
{
	color = componentLabel.color;
	size = componentLabel.size;
	finalText = componentLabel.finalText;
	fontUuid = componentLabel.fontUuid;

	if (includeComponents)
	{
		internalGO = new GameObject("Label", nullptr, true);
		internalGO->SetParent(parent);
	}

	if (!componentLabel.labelWord.empty())
	{
		for (LabelLetter* cLetter : componentLabel.labelWord)
		{
			LabelLetter* l = new LabelLetter();
			l->rect = new ComponentRectTransform(internalGO, ComponentTypes::RectTransformComponent, includeComponents);
			l->rect->FromLabel();
			l->rect->SetRect(cLetter->rect->GetRect());
			if (includeComponents)
				l->rect->Update();
			l->letter = cLetter->letter;
			l->textureID = cLetter->textureID;
			labelWord.push_back(l);
		}
	}

	vAlign = componentLabel.vAlign;
	hAlign = componentLabel.hAlign;

	if (includeComponents)
	{
		App->glCache->RegisterBufferIndex(&offset, &index, ComponentTypes::LabelComponent, this);
		if (App->glCache->isShaderStorage() && index != -1)
			FillBuffer();

		if (fontUuid > 0u) needed_findTextreID = true;
		needed_recalculateWord = true;
	}
}

ComponentLabel::~ComponentLabel()
{
	if (index != -1)
		App->glCache->UnRegisterBufferIndex(offset, ComponentTypes::LabelComponent);

	if (!labelWord.empty())
	{
		for (LabelLetter* letter : labelWord)
		{
			if (letter->rect != nullptr)
				RELEASE(letter->rect);
			RELEASE(letter);
		}
		labelWord.clear();
	}

	if (internalGO) RELEASE(internalGO);

	parent->cmp_label = nullptr;
}

void ComponentLabel::OnSystemEvent(System_Event event)
{
	switch (event.type)
	{
	case System_Event_Type::ScreenChanged:
		for (LabelLetter* letter : labelWord)
			letter->rect->OnSystemEvent(event);
		needed_recalculateWord = true;
		break;
	case System_Event_Type::CanvasChanged:
	case System_Event_Type::RectTransformUpdated:
		new_word = true;
			break;
	}
}

void ComponentLabel::Update()
{
	if (new_word)
	{
		if (!labelWord.empty())
		{
			for (LabelLetter* letter : labelWord)
			{
				if (letter->rect != nullptr)
					RELEASE(letter->rect);
				RELEASE(letter);
			}
			labelWord.clear();
		}

		ResourceFont* fontRes = fontUuid != 0 ? (ResourceFont*)App->res->GetResource(fontUuid) : nullptr;

		if (fontRes && !fontRes->fontData.charactersMap.empty())
		{
			int x_moving = 0;
			int* rectParent = parent->cmp_rectTransform->GetRect();
			x_moving = rectParent[X_UI_RECT];
			float sizeNorm = size / (float)fontRes->fontData.fontSize;
			uint contRows = 0;
			for (std::string::const_iterator c = finalText.begin(); c != finalText.end(); ++c)
			{
				if ((int)(*c) >= 32 && (int)(*c) < 128)//ASCII TABLE
				{
					LabelLetter* l = new LabelLetter();

					Character character;
					character = fontRes->fontData.charactersMap.find(*c)->second;

					memcpy(&l->letter, c._Ptr, sizeof(char));
					l->textureID = character.textureID;

					int x = x_moving + character.bearing.x * sizeNorm;
					//								Normalize pos with all heights	 //	Check Y-ofset for letters that write below origin "p" //	 Control lines enters
					int y = rectParent[Y_UI_RECT] + ((fontRes->fontData.maxCharHeight - character.size.y) + ((character.size.y) - character.bearing.y)) * sizeNorm + contRows * fontRes->fontData.maxCharHeight * sizeNorm;

					if (x + character.size.x * sizeNorm > rectParent[X_UI_RECT] + rectParent[W_UI_RECT])
					{
						y += fontRes->fontData.maxCharHeight * sizeNorm;
						x = rectParent[X_UI_RECT] + character.bearing.x * sizeNorm;
						x_moving = x;
						contRows++;
					}

					l->rect = new ComponentRectTransform(internalGO);
					l->rect->FromLabel();
					l->rect->SetRect(x,y, character.size.x * sizeNorm, character.size.y * sizeNorm);
					l->rect->Update();

					if (y + character.size.y * sizeNorm < rectParent[Y_UI_RECT] + rectParent[H_UI_RECT])
					{
						if (labelWord.size() < UI_MAX_LABEL_LETTERS)
							labelWord.push_back(l);
						else
						{
							CONSOLE_LOG(LogTypes::Warning, "Label can't draw more than %i letters.", UI_MAX_LABEL_LETTERS);
							RELEASE(l->rect);
							RELEASE(l);
							break;
						}
					}
					else
					{
						RELEASE(l->rect);
						RELEASE(l);
						break;
					}

					x_moving += character.advance * sizeNorm;
				}
				else if ((int)(*c) == 10)//"\n"
				{
					contRows++;
					x_moving = rectParent[X_UI_RECT];
				}
			}
			//Get quats for each row. Need it for align
			if (!labelWord.empty())
			{
				if (hAlign != H_Left)
					HorizontalAlignment(rectParent[W_UI_RECT], H_Left);

				if (vAlign != V_Top)
					VerticalAlignment(rectParent[H_UI_RECT], V_Top);

				if (App->glCache->isShaderStorage() && index != -1)
					FillBuffer();
			}
		}

		new_word = false;
	}

	if (needed_findTextreID)
	{
		if (!labelWord.empty())
		{
			ResourceFont* fontRes = fontUuid != 0 ? (ResourceFont*)App->res->GetResource(fontUuid) : nullptr;
			if (fontRes && !fontRes->fontData.charactersMap.empty())
			{
				for (LabelLetter* letter : labelWord)
				{
					if ((int)(letter->letter) >= 32 && (int)(letter->letter) < 128)//ASCII TABLE
					{
						Character character;
						character = fontRes->fontData.charactersMap.find(letter->letter)->second;
						letter->textureID = character.textureID;
					}
				}
			}			
		}
		needed_findTextreID = false;
	}

	if (needed_recalculateWord && !labelWord.empty())
	{
		for (LabelLetter* letter : labelWord)
			letter->rect->Update();
	
		if (App->glCache->isShaderStorage() && index != -1)
			FillBuffer();

		needed_recalculateWord = false;
	}
}

void ComponentLabel::VerticalAlignment(const int parentHeight, const VerticalLabelAlign alignFrom)
{
	uint posMaxHeight = 0u;
	for (uint i = 0; i < labelWord.size() - 1; ++i)
	{
		if (labelWord[i]->rect->GetRect()[H_UI_RECT] > labelWord[posMaxHeight]->rect->GetRect()[H_UI_RECT])
			posMaxHeight = i;
	}
	int heightRect = labelWord.back()->rect->GetRect()[H_UI_RECT] + labelWord.back()->rect->GetRect()[Y_UI_RECT] - labelWord.front()->rect->GetRect()[Y_UI_RECT] + (labelWord[posMaxHeight]->rect->GetRect()[H_UI_RECT] / 2);
	int diference = parentHeight - heightRect;

	for (uint i = 0; i < labelWord.size(); ++i)
	{
		int rect[4];
		memcpy(rect, labelWord[i]->rect->GetRect(), sizeof(int) * 4);
		switch (alignFrom)
		{
		case V_Top:
			if (vAlign == V_Middle)
				rect[Y_UI_RECT] += (diference / 2);
			else
				rect[Y_UI_RECT] += diference;
			break;
		case V_Middle:
			if (vAlign == V_Top)
				rect[Y_UI_RECT] -= (diference / 2);
			else
				rect[Y_UI_RECT] += (diference / 2);
			break;
		case V_Bottom:
			if (vAlign == V_Middle)
				rect[Y_UI_RECT] -= (diference / 2);
			else
				rect[Y_UI_RECT] -= diference;
			break;
		default:
			break;
		}
		labelWord[i]->rect->SetRect(rect);
		labelWord[i]->rect->Update();
	}
}

void ComponentLabel::HorizontalAlignment(const int parentWidth, const HorizontalLabelAlign alignFrom)
{
	//Horizontal Alignment
	uint firstLabelRow = 0u;
	int rowWidth;
	uint posMaxWidth = 0u;
	for (uint i = 0; i < labelWord.size() - 1; ++i)
	{
		if (labelWord[i]->rect->GetRect()[W_UI_RECT] > labelWord[posMaxWidth]->rect->GetRect()[W_UI_RECT])
			posMaxWidth = i;
		if (labelWord[i]->rect->GetRect()[X_UI_RECT] > labelWord[i + 1]->rect->GetRect()[X_UI_RECT])
		{
			rowWidth = labelWord[i]->rect->GetRect()[W_UI_RECT] + labelWord[i]->rect->GetRect()[X_UI_RECT] - labelWord[firstLabelRow]->rect->GetRect()[X_UI_RECT] + (labelWord[posMaxWidth]->rect->GetRect()[H_UI_RECT] / 2);
			int diference = (parentWidth - rowWidth);

			RowAlignment(firstLabelRow, i, diference, alignFrom);
			firstLabelRow = i + 1;
		}
	}
	rowWidth = labelWord.back()->rect->GetRect()[W_UI_RECT] + labelWord.back()->rect->GetRect()[X_UI_RECT] - labelWord[firstLabelRow]->rect->GetRect()[X_UI_RECT] + (labelWord[posMaxWidth]->rect->GetRect()[W_UI_RECT] / 2);
	//LastRow
	RowAlignment(firstLabelRow, labelWord.size() - 1, (parentWidth - rowWidth), alignFrom);
}

void ComponentLabel::RowAlignment(const uint firstLabelRow, const uint lastLabelRow, const int diference, const HorizontalLabelAlign alignFrom)
{
	for (uint j = firstLabelRow; j <= lastLabelRow; ++j)
	{
		int rect[4];
		memcpy(rect, labelWord[j]->rect->GetRect(), sizeof(int) * 4);
		switch (alignFrom)
		{
		case H_Left:
			if (hAlign == H_Middle)
				rect[X_UI_RECT] += (diference / 2);
			else
				rect[X_UI_RECT] += diference;
			break;
		case H_Middle:
			if (hAlign == H_Left)
				rect[X_UI_RECT] -= (diference / 2);
			else
				rect[X_UI_RECT] += (diference / 2);
			break;
		case H_Right:
			if (hAlign == H_Middle)
				rect[X_UI_RECT] -= (diference / 2);
			else
				rect[X_UI_RECT] -= diference;
			break;
		default:
			break;
		}
		labelWord[j]->rect->SetRect(rect);
		labelWord[j]->rect->Update();
	}
}

void ComponentLabel::WorldDraw(math::float3 * parentCorners, math::float4 corners[4], int * rectParent, int rect[4])
{
	math::float3 xDirection = (parentCorners[CORNER_TOP_LEFT] - parentCorners[CORNER_TOP_RIGHT]).Normalized();
	math::float3 yDirection = (parentCorners[2] - parentCorners[CORNER_TOP_LEFT]).Normalized();

	math::float3 pos = parentCorners[CORNER_TOP_RIGHT];
	pos = pos + (xDirection * ((float)(rect[X_UI_RECT] - rectParent[X_UI_RECT]) / WORLDTORECT)) + (yDirection * ((float)(rect[Y_UI_RECT] - rectParent[Y_UI_RECT]) / WORLDTORECT));
	corners[CORNER_TOP_RIGHT] = { pos, 1.0f };
	pos = corners[CORNER_TOP_RIGHT].xyz();
	pos = pos + (xDirection * (rect[W_UI_RECT] / WORLDTORECT));
	corners[CORNER_TOP_LEFT] = { pos, 1.0f };
	pos = corners[CORNER_TOP_LEFT].xyz();
	pos = pos + (yDirection * (rect[H_UI_RECT] / WORLDTORECT));
	corners[CORNER_BOTTOM_LEFT] = { pos, 1.0f };
	pos = corners[CORNER_BOTTOM_LEFT].xyz();
	pos = pos - (xDirection * (rect[W_UI_RECT] / WORLDTORECT));
	corners[CORNER_BOTTOM_RIGHT] = { pos, 1.0f };

	math::float3 zDirection = xDirection.Cross(yDirection);
	uint tmp;
	float z = ModuleUI::FindCanvas(parent, tmp)->cmp_canvas->GetZ(parent, GetType());
	for (uint i = 0; i < 4; ++i) //Change All Corners (TopLeft / TopRight / BottomLeft / BottomRight)
		corners[i] -= { zDirection * z , 0.0f };
}

void ComponentLabel::ScreenDraw(math::float4 corners[4], int rect[4])
{
	int* screen = App->ui->GetScreen();
	uint w_width = screen[ModuleUI::Screen::WIDTH];
	uint w_height = screen[ModuleUI::Screen::HEIGHT];

	corners[ComponentRectTransform::Rect::RBOTTOMLEFT] = { math::Frustum::ScreenToViewportSpace({ (float)rect[X_UI_RECT], (float)rect[Y_UI_RECT] }, w_width, w_height), 0.0f, 1.0f };
	corners[ComponentRectTransform::Rect::RBOTTOMRIGHT] = { math::Frustum::ScreenToViewportSpace({ (float)rect[X_UI_RECT] + (float)rect[W_UI_RECT], (float)rect[Y_UI_RECT] }, w_width, w_height), 0.0f, 1.0f };
	corners[ComponentRectTransform::Rect::RTOPLEFT] = { math::Frustum::ScreenToViewportSpace({ (float)rect[X_UI_RECT], (float)rect[Y_UI_RECT] + (float)rect[H_UI_RECT] }, w_width, w_height), 0.0f, 1.0f };
	corners[ComponentRectTransform::Rect::RTOPRIGHT] = { math::Frustum::ScreenToViewportSpace({ (float)rect[X_UI_RECT] + (float)rect[W_UI_RECT], (float)rect[Y_UI_RECT] + (float)rect[H_UI_RECT] }, w_width, w_height), 0.0f, 1.0f };
}

uint ComponentLabel::GetInternalSerializationBytes()
{
	uint labelWorldSize = 0;
	if(!labelWord.empty()) labelWorldSize = labelWord[0]->GetInternalSerializationBytes() * labelWord.size();
	return sizeof(color) + sizeof(int) + sizeof(uint) * 3 + sizeof(char) * finalText.length() + sizeof(vAlign) + sizeof(hAlign) + labelWorldSize;
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

	bytes = sizeof(vAlign);
	memcpy(cursor, &vAlign, bytes);
	cursor += bytes;

	bytes = sizeof(hAlign);
	memcpy(cursor, &hAlign, bytes);
	cursor += bytes;

	bytes = sizeof(uint);
	uint wordLenght = labelWord.size();
	memcpy(cursor, &wordLenght, bytes);
	cursor += bytes;

	if (wordLenght > 0)
	{
		for (LabelLetter* letter : labelWord)
			letter->OnInternalSave(cursor);
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

	bytes = sizeof(uint);
	memcpy(&fontUuid, cursor, bytes);
	cursor += bytes;

	//Load lenght + string
	uint nameLenght;
	memcpy(&nameLenght, cursor, bytes);
	cursor += bytes;

	bytes = nameLenght;
	finalText.resize(nameLenght);
	memcpy((void*)finalText.c_str(), cursor, bytes);
	finalText.resize(nameLenght);
	cursor += bytes;

	bytes = sizeof(vAlign);
	memcpy(&vAlign, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(hAlign);
	memcpy(&hAlign, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(uint);
	uint wordLenght = 0;
	memcpy(&wordLenght, cursor, bytes);
	cursor += bytes;

	if (wordLenght > 0)
	{
		for (uint i = 0; i < wordLenght; i++)
		{
			LabelLetter* l = new LabelLetter();
			l->rect = new ComponentRectTransform(internalGO, ComponentTypes::RectTransformComponent, parent->includeModuleComponent);
			l->rect->FromLabel();
			l->OnInternalLoad(cursor, parent->includeModuleComponent);
			labelWord.push_back(l);
		}
	}

	if (fontUuid > 0u) needed_findTextreID = true;
	needed_recalculateWord = true;
}

void ComponentLabel::OnUniqueEditor()
{
#ifndef GAMEMODE
	if (ImGui::CollapsingHeader("Label", ImGuiTreeNodeFlags_DefaultOpen))
	{
		float sizeX = ImGui::GetWindowWidth();
		if (ImGui::InputTextMultiline("##source", &finalText, ImVec2(sizeX, ImGui::GetTextLineHeight() * 7), ImGuiInputTextFlags_AllowTabInput))
			new_word = true;

		ImGui::PushItemWidth(200.0f);
		ImGui::ColorEdit4("Color", &color.x, ImGuiColorEditFlags_AlphaBar);

		if (ImGui::DragInt("Load new size", &size, 1.0f, 0, 72))
			new_word = true;

		ImGui::Separator();
		ImGui::Text("Horizontal Alignment");
		bool neededFillBuffer = false;
		if (ImGui::RadioButton("Left", hAlign == H_Left))
		{
			SetHorizontalAligment(H_Left);
			neededFillBuffer = true;
		}

		ImGui::SameLine();
		if (ImGui::RadioButton("Center", hAlign == H_Middle))
		{
			SetHorizontalAligment(H_Middle);
			neededFillBuffer = true;
		}

		ImGui::SameLine();
		if (ImGui::RadioButton("Border", hAlign == H_Right))
		{
			SetHorizontalAligment(H_Right);
			neededFillBuffer = true;
		}


		ImGui::Spacing();
		ImGui::Text("Vertical Alignment");
		if (ImGui::RadioButton("Top", vAlign == V_Top))
		{
			SetVerticalAligment(V_Top);
			neededFillBuffer = true;
		}

		ImGui::SameLine();
		if (ImGui::RadioButton("Middle", vAlign == V_Middle))
		{
			SetVerticalAligment(V_Middle);
			neededFillBuffer = true;
		}

		ImGui::SameLine();
		if (ImGui::RadioButton("Bottom", vAlign == V_Bottom))
		{
			SetVerticalAligment(V_Bottom);
			neededFillBuffer = true;
		}

		if (neededFillBuffer) needed_recalculateWord = true;
			//-----------------------------------------

		DragDropFont();
	}
#endif
}

void ComponentLabel::SetVerticalAligment(const VerticalLabelAlign nextAlignement)
{
	VerticalLabelAlign comeFrom = vAlign;
	if (vAlign != nextAlignement)
	{
		vAlign = nextAlignement;
		VerticalAlignment(parent->cmp_rectTransform->GetRect()[H_UI_RECT], comeFrom);
	}
}

void ComponentLabel::SetHorizontalAligment(const HorizontalLabelAlign nextAlignement)
{
	HorizontalLabelAlign comeFrom = hAlign;
	if (hAlign != nextAlignement)
	{
		hAlign = nextAlignement;
		HorizontalAlignment(parent->cmp_rectTransform->GetRect()[W_UI_RECT], comeFrom);
	}
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
				new_word = true;
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
		if (font)
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

void ComponentLabel::FillBuffer()
{
	uint slabelWord = (labelWord.size() <= UI_MAX_LABEL_LETTERS) ? labelWord.size() : UI_MAX_LABEL_LETTERS;
	buffer_size = slabelWord * sizeof(float) * 16;
	char* cursor = buffer;
	float one = 1.0f;
	size_t bytes = sizeof(float) * 3;
	

	for (uint i = 0; i < slabelWord; i++)
	{
		if (parent->cmp_rectTransform->GetFrom() != ComponentRectTransform::RectFrom::RECT || App->ui->ScreenOnWorld())
		{
			math::float3* rCorners = labelWord[i]->rect->GetCorners();
			memcpy(cursor, &rCorners[ComponentRectTransform::Rect::RTOPLEFT], bytes);
			cursor += bytes; memcpy(cursor, &one, sizeof(float)); cursor += sizeof(float);
			memcpy(cursor, &rCorners[ComponentRectTransform::Rect::RTOPRIGHT], bytes);
			cursor += bytes; memcpy(cursor, &one, sizeof(float)); cursor += sizeof(float);
			memcpy(cursor, &rCorners[ComponentRectTransform::Rect::RBOTTOMLEFT], bytes);
			cursor += bytes; memcpy(cursor, &one, sizeof(float)); cursor += sizeof(float);
			memcpy(cursor, &rCorners[ComponentRectTransform::Rect::RBOTTOMRIGHT], bytes);
			cursor += bytes; memcpy(cursor, &one, sizeof(float)); cursor += sizeof(float);
		}
		else
		{
			math::float3* rCorners = labelWord[i]->rect->GetCorners();
			memcpy(cursor, &rCorners[ComponentRectTransform::Rect::RBOTTOMLEFT], bytes);
			cursor += bytes; memcpy(cursor, &one, sizeof(float)); cursor += sizeof(float);
			memcpy(cursor, &rCorners[ComponentRectTransform::Rect::RBOTTOMRIGHT], bytes);
			cursor += bytes; memcpy(cursor, &one, sizeof(float)); cursor += sizeof(float);
			memcpy(cursor, &rCorners[ComponentRectTransform::Rect::RTOPLEFT], bytes);
			cursor += bytes; memcpy(cursor, &one, sizeof(float)); cursor += sizeof(float);
			memcpy(cursor, &rCorners[ComponentRectTransform::Rect::RTOPRIGHT], bytes);
			cursor += bytes; memcpy(cursor, &one, sizeof(float)); cursor += sizeof(float);
		}
	}

	App->glCache->FillBufferRange(offset, buffer_size, buffer);
}

void ComponentLabel::SetFinalText(const char * newText)
{
	finalText = newText;
	new_word = true;
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

std::vector<LabelLetter*>* ComponentLabel::GetWord()
{
	return &labelWord;
}

int ComponentLabel::GetBufferIndex() const
{
	return index;
}

void ComponentLabel::SetBufferRangeAndFIll(uint offs, int index)
{
	offset = offs;
	this->index = index;
	if(!labelWord.empty())
		FillBuffer();
}

void ComponentLabel::SetFontResource(uint uuid)
{
	fontUuid = uuid;
	new_word = true;
	size = ((ResourceFont*)App->res->GetResource(uuid))->fontData.fontSize;
}

void ComponentLabel::SetFontResource(std::string fontName)
{
	std::vector<Resource*> fonts = App->res->GetResourcesByType(ResourceTypes::FontResource);
	for (Resource* resource : fonts)
	{
		if (resource->GetData().name == fontName)
		{
			SetFontResource(resource->GetUuid());
			break;
		}
	}
}

ResourceFont* ComponentLabel::GetFontResource()
{
	return fontUuid != 0 ? (ResourceFont*)App->res->GetResource(fontUuid) : nullptr;
}

uint LabelLetter::GetInternalSerializationBytes()
{
	return sizeof(char) + sizeof(int) * 4;
}

void LabelLetter::OnInternalSave(char *& cursor)
{
	size_t bytes = sizeof(char);
	memcpy(cursor, &letter, bytes);
	cursor += bytes;

	bytes = sizeof(int) * 4;
	memcpy(cursor, rect->GetRect(), bytes);
	cursor += bytes;
}

void LabelLetter::OnInternalLoad(char *& cursor, bool include)
{
	size_t bytes = sizeof(char);
	memcpy(&letter, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(int) * 4;
	int rectp[4];
	memcpy(&rectp, cursor, bytes);
	cursor += bytes;

	rect->SetRect(rectp);
	if(include)
		rect->Update();
}
