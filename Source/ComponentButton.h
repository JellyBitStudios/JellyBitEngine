#ifndef __COMPONENT_BUTTON_H__
#define __COMPONENT_BUTTON_H__

//#include "ComponentRigidActor.h"
#include "Component.h"

#include <string>
#include <mono/metadata/class.h>

enum UIState;

class ComponentButton : public Component
{
public:
	ComponentButton(GameObject * parent, ComponentTypes componentType = ComponentTypes::ButtonComponent, bool includeComponents = true);
	ComponentButton(const ComponentButton & componentButton, GameObject* parent, bool includeComponents = true);
	
	~ComponentButton();
	
	//NOTE: If you override this method, make sure to call the base class method. 
	//(Component::OnSystemEvent(event); at start)
	void OnSystemEvent(System_Event event);

	void Update();

	void KeyPressed();
	void RightClickPressed();
	UIState GetState()const;

	void SetNewKey(uint key);

	void LoadOnClickReference();

	static bool MouseInScreen(const int* rect);

private:
	uint GetInternalSerializationBytes();
	uint BytesToOnClick();
	uint BytesToOnClickFromBuffer(char*& cursor);

	void OnInternalSave(char*& cursor);
	void OnSaveOnClick(char*& cursor);

	void OnInternalLoad(char*& cursor);
	void OnLoadOnClick(char*& cursor);

	void OnUniqueEditor();

private:

	char* tempBuffer = nullptr;

	UIState state;

	std::string input;
	uint button_blinded;

	void SetNewKey(const char* key);

	MonoMethod* methodToCall = nullptr;
	MonoObject* scriptInstance = nullptr;

	uint onClickScriptUUID = 0;
	uint onClickGameObjectUUID = 0;
	std::string methodToCallName;

	GameObject* draggedGO = nullptr;

	uint idleTexture = 0u;
	uint hoveredTexture = 0u;
	uint clickTexture = 0u;

	//Physix plane for button when canvas is world
	bool isWorld = false;
	//physx::PxRigidActor* gActor = nullptr;
};

#endif

