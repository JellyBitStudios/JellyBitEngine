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
	struct LetterBuffer {
		float topLeft[3];
		float offsetFloat1;
		float topRight[3];
		float offsetFloat2;
		float bottomLeft[3];
		float offsetFloat3;
		float bottomRight[3];
		float offsetFloat4;
	};
	struct LabelBuffer {
		float aligment[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		float offsetFloat[4];
		LetterBuffer* word = nullptr;
	};
	struct LabelLetter
	{
		math::float3 corners[4] = { math::float3::zero, math::float3::zero, math::float3::zero, math::float3::zero };
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

	void WorldDraw(math::float3 * parentCorners, math::float3 corners[4], uint * rectParent, const uint x, const uint y, math::float2 characterSize, float sizeNorm);

	void ScreenDraw(math::float3 corners[4], const uint x, const uint y, math::float2 characterSize, float sizeNorm);

	void SetFinalText(const char* newText);
	const char* GetFinalText() const;

	void SetColor(math::float4 newColor);
	math::float4 GetColor() const;
	void* GetBuffer();
	uint GetBufferSize()const;
	uint GetWordSize()const;
	std::vector<uint>* GetWordTextureIDs();

private:
	uint GetInternalSerializationBytes();
	void OnInternalSave(char*& cursor);
	void OnInternalLoad(char*& cursor);
	void OnUniqueEditor();
	void DragDropFont();

private:
	uint fontUuid = 0u;

	int size = 72;

	std::string finalText = "Edit Text";

	math::float4 color = math::float4::one;

	std::vector<LabelLetter> labelWord;
	std::vector<uint> textureWord;
	LabelBuffer buffer;
	uint last_word_size = 0;
	uint buffer_size = 0;

	bool needed_recalculate = false;
};

#endif
