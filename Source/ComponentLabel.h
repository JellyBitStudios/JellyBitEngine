#ifndef __COMPONENT_LABEL_H__
#define __COMPONENT_LABEL_H__

#include "ModuleUI.h"

#include "Component.h"
#include <string>
#include <map>
#include <vector>

#include "MathGeoLib/include/Math/float4.h"
#include "MathGeoLib/include/Math/float3.h"

struct Character;
class ResourceFont;

#define X_UI_RECT 0
#define Y_UI_RECT 1
#define W_UI_RECT 2
#define H_UI_RECT 3

#define CORNER_TOP_LEFT 0
#define CORNER_TOP_RIGHT 1
#define CORNER_BOTTOM_LEFT 2
#define CORNER_BOTTOM_RIGHT 3

enum VerticalLabelAlign
{
	V_Top,
	V_Middle,
	V_Bottom
};
enum HorizontalLabelAlign
{
	H_Left,
	H_Middle,
	H_Right
};
class ComponentLabel : public Component
{
public:
	struct LabelLetter
	{
		math::float4 corners[4] = { { 0.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f, 1.0f } };
		uint rect[4];
		uint textureID = 0;
		math::float2 size;
	};

public:
	ComponentLabel(GameObject* parent, ComponentTypes componentType = ComponentTypes::LabelComponent, bool includeComponents = true);
	ComponentLabel(const ComponentLabel& componentLabel, GameObject* parent, bool includeComponents = true);
	~ComponentLabel();

	//NOTE: If you override this method, make sure to call the base class method.
	//(Component::OnSystemEvent(event); at start)
	void OnSystemEvent(System_Event event);

	void Update();

	void VerticalAlignment(const uint parentHeight, const VerticalLabelAlign alignFrom);
	void HorizontalAlignment(const uint parentWidth, const HorizontalLabelAlign alignFrom);
	void RowAlignment(const uint firstLabelRow, const uint lastLabelRow, const uint diference, const HorizontalLabelAlign alignFrom);

	void WorldDraw(math::float3 * parentCorners, math::float4 corners[4], uint * rectParent, uint rect[4]);

	void ScreenDraw(math::float4 corners[4], uint rect[4]);

	void SetFinalText(const char* newText);
	const char* GetFinalText() const;

	void SetColor(math::float4 newColor);
	math::float4 GetColor() const;
	char* GetBuffer();
	uint GetBufferSize()const;
	uint GetWordSize()const;
	std::vector<uint>* GetWordTextureIDs();

	int GetBufferIndex()const;
	void SetBufferRangeAndFIll(uint offs, int index);

	void SetFontResource(uint uuid);
	void SetFontResource(std::string fontName);
	ResourceFont* GetFontResource();

private:
	uint GetInternalSerializationBytes();
	void OnInternalSave(char*& cursor);
	void OnInternalLoad(char*& cursor);
	void OnUniqueEditor();
	void SetVerticalAligment(const VerticalLabelAlign nextAlignement);
	void SetHorizontalAligment(const HorizontalLabelAlign nextAlignement);
	void DragDropFont();
	void FIllBuffer();
	void FillCorners();
private:
	uint fontUuid = 0u;

	int size = 72;

	std::string finalText = "Edit Text";

	math::float4 color = math::float4::one;

	std::vector<LabelLetter> labelWord;
	std::vector<uint> textureWord;
	uint last_word_size = 0;

	bool needed_recalculate = false;
	bool aligment = false;
	bool hAligment = false;

	//Buffer
	char buffer[UI_BYTES_LABEL];
	uint buffer_size = 0;
	uint offset = 0;
	int index = -1;

	//Align
	VerticalLabelAlign vAlign = V_Top;
	HorizontalLabelAlign hAlign = H_Left;
};

#endif
