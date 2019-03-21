#ifndef __COMPONENT_IMAGE_H__
#define __COMPONENT_IMAGE_H__

#include "Component.h"

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

	const float* GetColor()const;
	void SetResImageUuid(uint res_image_uuid);
	uint GetResImageUuid() const;
	uint GetResImage()const;

	void SetMask();
	void RectChanged();

	bool useMask() const;
	float* GetMask();

private:
	uint GetInternalSerializationBytes();
	void OnInternalSave(char*& cursor);
	void OnInternalLoad(char*& cursor);
	void OnUniqueEditor();

private:
	uint res_image = 0;
	float color[4] = { 1.0f,1.0f,1.0f,1.0f };
	bool mask = false;
	float mask_values[2] = { 1.0f, 0.0f };
	float rect_initValues[2] = { 0.0f,0.0f };
};

#endif