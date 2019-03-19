#ifndef __COMPONENT_LABEL_H__
#define __COMPONENT_LABEL_H__

#include "Component.h"
#include <string>
#include <map>

#include "MathGeoLib/include/Math/float4.h"

struct Character;
class ComponentRectTransform;

class ComponentLabel : public Component
{
public:
	ComponentLabel(GameObject* parent, ComponentTypes componentType = ComponentTypes::LabelComponent);
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
	
	void LinkToUIModule();

	void SetRectToShader(uint shader);

private:
	int size = 72;
	int sizeLoaded = 72;
	uint maxLabelSize = 0u;

	ComponentRectTransform* rect = nullptr;
	std::map<char, Character> charactersBitmap;
	
	std::string finalText;
	char text[300] = "Edit Text";
	math::float4 color = math::float4::one;


};

#endif