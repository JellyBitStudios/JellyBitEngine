#ifndef __COMPONENT_LABEL_H__
#define __COMPONENT_LABEL_H__

#include "Component.h"
#include <string>
#include <map>
#include <vector>

#include "MathGeoLib/include/Math/float4.h"
#include "MathGeoLib/include/Math/float3.h"

struct Character;


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

	void WorldDraw(math::float3 * parentCorners, math::float3 corners[4], uint * rectParent, const uint x, const uint y, math::float2 characterSize, float sizeNorm);

	void ScreenDraw(uint rect[4], const uint x, const uint y, math::float2 characterSize, float sizeNorm);

	const char* GetFinalText() const;

	std::vector<LabelLetter>* GetLetterQueue();
	math::float4 GetColor() const;

	void RectChanged();

private:
	uint GetInternalSerializationBytes();
	void OnInternalSave(char*& cursor);
	void OnInternalLoad(char*& cursor);
	void OnUniqueEditor();

private:
	uint fontUuid = 0u;

	int size = 72;

	std::string finalText;
	char text[300] = "Edit Text";
	math::float4 color = math::float4::one;

	std::vector<LabelLetter> labelWord;

	bool needed_recaclculate = false;
};

#endif
