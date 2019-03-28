#ifndef __COMPONENT_LABEL_H__
#define __COMPONENT_LABEL_H__

#include "Component.h"
#include <string>
#include <map>

#include "MathGeoLib/include/Math/float4.h"

struct Character;
class ComponentRectTransform;

struct LabelLetter
{
	math::float4 rect  = math::float4::zero;
	char letter = NULL;
	uint textureID = 0;
};

class ComponentLabel : public Component
{
public:
	ComponentLabel(GameObject* parent, ComponentTypes componentType = ComponentTypes::LabelComponent, bool includeComponents = true);
	ComponentLabel(const ComponentLabel& componentLabel, GameObject* parent, bool includeComponents = true);
	~ComponentLabel();

	void Draw(uint ui_shader, uint VAO);
	void Update();

	const char* GetFinalText() const;

private:
	uint GetInternalSerializationBytes();
	void OnInternalSave(char*& cursor);
	void OnInternalLoad(char*& cursor);
	void OnUniqueEditor();

	void SetRectToShader(uint shader);
private:
	int size = 72;
	int sizeLoaded = 72;
	uint maxLabelSize = 0u;

	std::map<char, Character> charactersBitmap;

	std::string finalText;
	char text[300] = "Edit Text";
	math::float4 color = math::float4::one;

	std::vector<LabelLetter> labelWord;

	bool needed_recaclculate = false;
};

#endif
