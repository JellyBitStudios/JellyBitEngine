#ifndef __COMPONENT_CANVASRENDERER_H__
#define __COMPONENT_CANVASRENDERER_H__

#include "Component.h"

#include "Globals.h"

#include <vector>

#include "MathGeoLib/include/Math/float4.h"
#include "MathGeoLib/include/Math/float2.h"

#include "ComponentLabel.h"

class ComponentCanvasRenderer : public Component
{
public:

	enum RenderTypes
	{
		RENDER_NULL,
		IMAGE,
		LABEL
	};

	struct ToUIRend
	{
	public:
		ToUIRend() {}

		void Set(RenderTypes t, Component* c)
		{
			type = t;
			cmp = c;
			isRendered_flag = false;
		}

		RenderTypes GetType()const
		{
			return type;
		}
		math::float4 GetColor();
		uint GetTexture();
		const char * GetText();
		math::float2 GetMaskValues();
		void* GetBufferWord();
		uint GetBufferSize()const;
		uint GetWordSize()const;
		std::vector<uint>* texturesWord();

		bool isRendered() {
			return isRendered_flag;
		}
	private:
		RenderTypes type = RenderTypes::RENDER_NULL;
		Component* cmp = nullptr;
		bool isRendered_flag = true;
	};

	ComponentCanvasRenderer(GameObject* parent, ComponentTypes componentType = ComponentTypes::CanvasRendererComponent, bool includeComponents = true);
	ComponentCanvasRenderer(const ComponentCanvasRenderer& componentRectTransform, GameObject* parent, bool includeComponents = true);
	~ComponentCanvasRenderer();

	//NOTE: If you override this method, make sure to call the base class method. 
	//(Component::OnSystemEvent(event); at start)
	void OnSystemEvent(System_Event event);

	void Update();
	void OnEditor();

	ToUIRend* GetDrawAvaiable() const;

	bool IsWorld()const;

private:
	uint GetInternalSerializationBytes();
	void OnInternalSave(char*& cursor);
	void OnInternalLoad(char*& cursor);
	void OnUniqueEditor();

	std::vector<ToUIRend* > rend_queue;

	bool fromWorld = false;
};

#endif
