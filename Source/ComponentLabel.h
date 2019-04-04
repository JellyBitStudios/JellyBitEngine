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


class ComponentLabel : public Component
{
public:
	struct LabelLetter
	{
		math::float4 corners[4] = { { 0.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f, 1.0f } };
		uint textureID = 0;
	};

public:
	ComponentLabel(GameObject* parent, ComponentTypes componentType = ComponentTypes::LabelComponent, bool includeComponents = true);
	ComponentLabel(const ComponentLabel& componentLabel, GameObject* parent, bool includeComponents = true);
	~ComponentLabel();

	//NOTE: If you override this method, make sure to call the base class method. 
	//(Component::OnSystemEvent(event); at start)
	void OnSystemEvent(System_Event event);

	void Update();

	void WorldDraw(math::float3 * parentCorners, math::float4 corners[4], uint * rectParent, const uint x, const uint y, math::float2 characterSize, float sizeNorm);

	void ScreenDraw(math::float4 corners[4], const uint x, const uint y, math::float2 characterSize, float sizeNorm);

	void SetFinalText(const char* newText);
	const char* GetFinalText() const;

	void SetColor(math::float4 newColor);
	math::float4 GetColor() const;
	char* GetBuffer();
	uint GetBufferSize()const;
	uint GetWordSize()const;
	std::vector<uint>* GetWordTextureIDs();

private:
	uint GetInternalSerializationBytes();
	void OnInternalSave(char*& cursor);
	void OnInternalLoad(char*& cursor);
	void OnUniqueEditor();
	void DragDropFont();
	void FIllBuffer();
private:
	uint fontUuid = 0u;

	int size = 72;

	std::string finalText = "Edit Text";

	math::float4 color = math::float4::one;

	std::vector<LabelLetter> labelWord;
	std::vector<uint> textureWord;
	char* buffer = nullptr;
	uint last_word_size = 0;
	uint buffer_size = 0;

	bool needed_recalculate = false;
};

#endif
