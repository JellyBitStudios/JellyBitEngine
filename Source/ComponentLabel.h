#ifndef __COMPONENT_LABEL_H__
#define __COMPONENT_LABEL_H__

#include "Component.h"
#include <string>
#include <map>

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
	int size = 16;
	ComponentRectTransform* rect = nullptr;
	std::map<char, Character> charactersBitmap;

};

#endif