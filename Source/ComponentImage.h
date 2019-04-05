#ifndef __COMPONENT_IMAGE_H__
#define __COMPONENT_IMAGE_H__

#include "ModuleUI.h"

#include "Component.h"
#include <string>

class ComponentImage : public Component
{
public:
	enum Color
	{
		R,
		G,
		B,
		A
	};

	ComponentImage(GameObject* parent, ComponentTypes componentType = ComponentTypes::ImageComponent, bool includeComponents = true);
	ComponentImage(const ComponentImage& componentImage, GameObject* parent, bool includeComponents = true);
	~ComponentImage();

	void Update();

	//NOTE: If you override this method, make sure to call the base class method. 
	//(Component::OnSystemEvent(event); at start)
	void OnSystemEvent(System_Event event);

	uint GetResImage()const;

	bool useMask() const;
	float* GetMask();

	uint GetResImageUuid() const;
	void SetResImageUuid(uint res_image_uuid);

	int GetBufferIndex()const;
	void SetBufferRangeAndFIll(uint offs, int index);

	//Scripting
	float* GetColor();
	void SetColor(float r, float g, float b, float a);
	void ResetColor();
	std::string GetResImageName() const;
	void SetResImageName(const std::string& name);
	void SetMask();

private:
	uint GetInternalSerializationBytes();
	void OnInternalSave(char*& cursor);
	void OnInternalLoad(char*& cursor);
	void OnUniqueEditor();

private:
	void FillBuffer();

private:
	uint res_image = 0;
	float color[4] = { 1.0f,1.0f,1.0f,1.0f };
	bool mask = false;
	float mask_values[2] = { 1.0f, 0.0f };
	float rect_initValues[2] = { 0.0f,0.0f };

	char buffer[UI_BYTES_IMAGE];
	int index = -1;
	uint offset = 0;

	bool needed_recalculate = false;
};

#endif