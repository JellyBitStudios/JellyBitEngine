#ifndef __COMPONENT_LABEL_H__
#define __COMPONENT_LABEL_H__

#include "Component.h"
#include "GLCache.h"

#include <string>
#include <map>
#include <vector>

#include "MathGeoLib/include/Math/float4.h"
#include "MathGeoLib/include/Math/float3.h"
#include "MathGeoLib/include/Math/float2.h"

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
struct LabelLetter
{
	char letter = NULL;
	class ComponentRectTransform* rect = nullptr;
	uint textureID = 0;

	uint GetInternalSerializationBytes();
	void OnInternalSave(char*& cursor);
	void OnInternalLoad(char*& cursor, bool include);
};
class ComponentLabel : public Component
{
public:
	ComponentLabel(GameObject* parent, ComponentTypes componentType = ComponentTypes::LabelComponent, bool includeComponents = true);
	ComponentLabel(const ComponentLabel& componentLabel, GameObject* parent, bool includeComponents = true);
	~ComponentLabel();

	//NOTE: If you override this method, make sure to call the base class method.
	//(Component::OnSystemEvent(event); at start)
	void OnSystemEvent(System_Event event);

	void Update();

	void VerticalAlignment(const int parentHeight, const VerticalLabelAlign alignFrom);
	void HorizontalAlignment(const int parentWidth, const HorizontalLabelAlign alignFrom);
	void RowAlignment(const uint firstLabelRow, const uint lastLabelRow, const int diference, const HorizontalLabelAlign alignFrom);

	void WorldDraw(math::float3 * parentCorners, math::float4 corners[4], int * rectParent, int rect[4]);

	void ScreenDraw(math::float4 corners[4], int rect[4]);

	void SetFinalText(const char* newText);
	const char* GetFinalText() const;

	void SetAlpha(float alpha);
	void SetColor(math::float4 newColor);
	math::float4 GetColor() const;
	char* GetBuffer();
	uint GetBufferSize()const;
	uint GetWordSize()const;
	std::vector<LabelLetter*>* GetWord();

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
	void FillBuffer();

private:
	uint fontUuid = 0u;

	int size = 72;

	std::string finalText = "Edit Text";

	math::float4 color = math::float4::one;

	std::vector<LabelLetter*> labelWord;
	uint last_word_size = 0;

	GameObject* internalGO = nullptr;

	bool new_word = false;
	bool needed_recalculateWord = false;
	bool needed_findTextreID = false;
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
