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
	std::string finalText;
	int size = 72;
	int sizeLoaded = 72;
	ComponentRectTransform* rect = nullptr;
	char text[300] = "Edit Text";
	std::map<char, Character> charactersBitmap;
	math::float4 color = math::float4::one;

};

#endif