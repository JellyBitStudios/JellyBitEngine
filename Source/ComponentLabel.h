#ifndef __COMPONENT_LABEL_H__
#define __COMPONENT_LABEL_H__

#include "Component.h"
#include <string>
#include <map>
#include <vector>

#include "MathGeoLib/include/Math/float4.h"
#include "MathGeoLib/include/Math/float3.h"

struct Character;

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
		uint rect[4] = { 0, 0, 0, 0 };
		math::float3 corners[4] = { math::float3::zero, math::float3::zero, math::float3::zero, math::float3::zero };
		char letter = NULL;
		uint textureID = 0;
	};

public:
	ComponentLabel(GameObject* parent, ComponentTypes componentType = ComponentTypes::LabelComponent, bool includeComponents = true);
	ComponentLabel(const ComponentLabel& componentLabel, GameObject* parent, bool includeComponents = true);
	~ComponentLabel();

	void Update();

	void VerticalAlignment(const uint parentHeight, const VerticalLabelAlign alignFrom);
	void HorizontalAlignment(const uint parentWidth, const HorizontalLabelAlign alignFrom);
	void RowAlignment(const uint firstLabelRow, const uint lastLabelRow, const uint diference, const HorizontalLabelAlign alignFrom);

	void WorldDraw(math::float3 * parentCorners, math::float3 corners[4], uint * rectParent, const uint x, const uint y, math::float2 characterSize, float sizeNorm);

	void ScreenDraw(uint rect[4], const uint x, const uint y, math::float2 characterSize, float sizeNorm);

	void SetFinalText(const char* newText);
	const char* GetFinalText() const;

	std::vector<LabelLetter>* GetLetterQueue();

	void SetColor(math::float4 newColor);
	math::float4 GetColor() const;

	void RectChanged();

private:
	uint GetInternalSerializationBytes();
	void OnInternalSave(char*& cursor);
	void OnInternalLoad(char*& cursor);
	void OnUniqueEditor();
	void SetVerticalAligment(const VerticalLabelAlign nextAlignement);
	void SetHorizontalAligment(const HorizontalLabelAlign nextAlignement);
	void DragDropFont();

private:
	uint fontUuid = 0u;

	int size = 72;

	std::string finalText = "Edit Text";

	math::float4 color = math::float4::one;

	std::vector<LabelLetter> labelWord;

	bool needed_recaclculate = false;

	VerticalLabelAlign vAlign = V_Top;
	HorizontalLabelAlign hAlign = H_Left;
};

#endif
