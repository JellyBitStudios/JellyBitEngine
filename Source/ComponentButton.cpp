#include "ComponentButton.h"

#include "ComponentRectTransform.h"
#include "ComponentImage.h"
#include "ComponentScript.h"

#include "ModuleUI.h"
#include "ModuleInput.h"
#include "ScriptingModule.h"
#include "ModuleGOs.h"
#include "ModulePhysics.h"

#include "GameObject.h"
#include "Application.h"
#include "ModuleResourceManager.h"

#include "ResourceTexture.h"

#include "imgui\imgui.h"
#include "imgui\imgui_internal.h"
#include "imgui\imgui_stl.h"

#include <mono/metadata/attrdefs.h>

ComponentButton::ComponentButton(GameObject* parent, ComponentTypes componentType, bool includeComponents) : Component(parent, ComponentTypes::ButtonComponent)
{
	state = UIState::IDLE;
	input = "z";
	button_blinded = (uint)SDL_GetScancodeFromKey(SDL_GetKeyFromName(input.c_str()));

	if (includeComponents)
	{
		if (parent->cmp_rectTransform == nullptr)
			parent->AddComponent(ComponentTypes::RectTransformComponent);

		if (!parent->cmp_image)
			parent->AddComponent(ComponentTypes::ImageComponent);

		App->ui->buttons_ui.push_back(this);
	}
}

ComponentButton::ComponentButton(const ComponentButton & componentButton, GameObject* parent, bool includeComponents) : Component(parent, ComponentTypes::ButtonComponent)
{
	state = componentButton.state;
	button_blinded = componentButton.button_blinded;
	input = componentButton.input;
	scriptInstance = componentButton.scriptInstance;
	methodToCall = componentButton.methodToCall;
	idleTexture = componentButton.idleTexture;
	hoveredTexture = componentButton.hoveredTexture;
	clickTexture = componentButton.clickTexture;

	if (includeComponents)
		App->ui->buttons_ui.push_back(this);
}

ComponentButton::~ComponentButton()
{
	parent->cmp_button = nullptr;
	App->ui->buttons_ui.erase(std::remove(App->ui->buttons_ui.begin(), App->ui->buttons_ui.end(), this), App->ui->buttons_ui.end());
}

void ComponentButton::OnSystemEvent(System_Event event)
{
	Component::OnSystemEvent(event);

	switch (event.type)
	{
		case System_Event_Type::ScriptingDomainReloaded:
		{
			methodToCall = nullptr;
			scriptInstance = nullptr;

			break;
		}
		case System_Event_Type::LoadFinished:
		{
			OnLoadOnClick(tempBuffer);

			delete[] tempBuffer;
			tempBuffer = nullptr;

			break;
		}
	}
}

void ComponentButton::Update()
{
	if (IsTreeActive())
	{
		const int* rect = parent->cmp_rectTransform->GetRect();

		if (App->input->GetKey(button_blinded) == KEY_DOWN)
			KeyPressed();

		switch (state)
		{
		case IDLE:
			if (MouseInScreen(rect))
			{
				state = HOVERED;
				if (hoveredTexture > 0) parent->cmp_image->SetResImageUuid(hoveredTexture);
			}
			break;
		case HOVERED:
			if (!MouseInScreen(rect))
			{
				state = IDLE;
				if (idleTexture > 0) parent->cmp_image->SetResImageUuid(idleTexture);
			}
			else if (App->input->GetMouseButton(SDL_BUTTON_RIGHT) == KEY_DOWN)
			{
				state = R_CLICK;
				RightClickPressed();
			}
			else if (App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_DOWN)
			{
				if (clickTexture > 0) parent->cmp_image->SetResImageUuid(clickTexture);
				state = L_CLICK;
				KeyPressed();
			}
			break;
		case R_CLICK:
			state = HOVERED;
			break;
		case L_CLICK:
			if (App->input->GetMouseButton(SDL_BUTTON_LEFT) != KEY_REPEAT)
			{
				state = HOVERED;
				if (hoveredTexture > 0) parent->cmp_image->SetResImageUuid(hoveredTexture);
			}
			break;
		default:
			break;
		}
	}
}

uint ComponentButton::GetInternalSerializationBytes()
{
	return sizeof(uint) * 4 + BytesToOnClick();
}

uint ComponentButton::BytesToOnClick()
{
	if (methodToCall && scriptInstance)
	{
		//Something serialized + GameObject UID + Component UID + methodNameSize + methodName
		return sizeof(bool) + sizeof(uint) * 3 + std::string(mono_method_get_name(methodToCall)).size();
	}
	else
	{
		//Something serialized
		return sizeof(bool);
	}
}

uint ComponentButton::BytesToOnClickFromBuffer(char*& cursor)
{
	uint serializedBytes = 0u;

	char* tempCursor = cursor;

	uint bytes = sizeof(bool);

	bool somethingSerialized;
	memcpy(&somethingSerialized, tempCursor, bytes);
	tempCursor += bytes;
	serializedBytes += bytes;

	if (somethingSerialized)
	{
		bytes = sizeof(uint);
		tempCursor += bytes;
		serializedBytes += bytes;

		tempCursor += bytes;
		serializedBytes += bytes;

		uint nameSize;
		memcpy(&nameSize, tempCursor, bytes);
		tempCursor += bytes;
		serializedBytes += bytes;

		tempCursor += nameSize;
		serializedBytes += nameSize;
	}

	return serializedBytes;
}

void ComponentButton::OnInternalSave(char *& cursor)
{
	size_t bytes = sizeof(uint);
	memcpy(cursor, &button_blinded, bytes);
	cursor += bytes;

	memcpy(cursor, &idleTexture, bytes);
	cursor += bytes;

	memcpy(cursor, &hoveredTexture, bytes);
	cursor += bytes;

	memcpy(cursor, &clickTexture, bytes);
	cursor += bytes;

	OnSaveOnClick(cursor);
}

void ComponentButton::OnSaveOnClick(char*& cursor)
{
	if (methodToCall && scriptInstance)
	{
		uint bytes = sizeof(bool);
		bool temp = true;
		memcpy(cursor, &temp, bytes);
		cursor += bytes;

		ComponentScript* methodComponent = (ComponentScript*)App->scripting->ComponentFrom(scriptInstance);
		GameObject* methodGO = methodComponent->GetParent();

		bytes = sizeof(uint);
		uint goUID = methodGO->GetUUID();
		memcpy(cursor, &goUID, bytes);
		cursor += bytes;

		memcpy(cursor, &methodComponent->UUID, bytes);
		cursor += bytes;

		std::string methodName = mono_method_get_name(methodToCall);
		uint nameSize = methodName.size();
		memcpy(cursor, &nameSize, bytes);
		cursor += bytes;

		memcpy(cursor, methodName.data(), nameSize);
		cursor += nameSize;
	}
	else
	{
		bool temp = false;
		memcpy(cursor, &temp, sizeof(bool));
		cursor += sizeof(bool);
	}
}

void ComponentButton::OnInternalLoad(char *& cursor)
{
	size_t bytes = sizeof(uint);
	memcpy(&button_blinded, cursor, bytes);
	cursor += bytes;

	SetNewKey(button_blinded);

	memcpy(&idleTexture, cursor, bytes);
	cursor += bytes;

	memcpy(&hoveredTexture, cursor, bytes);
	cursor += bytes;

	memcpy(&clickTexture, cursor, bytes);
	cursor += bytes;

	if (idleTexture > 0) parent->cmp_image->SetResImageUuid(idleTexture);

	uint bytesOnClick = BytesToOnClickFromBuffer(cursor);
	tempBuffer = new char[bytesOnClick];
	memcpy(tempBuffer, cursor, bytesOnClick);
	cursor += bytesOnClick;
}

void ComponentButton::OnLoadOnClick(char*& tempBuffer)
{
	char* cursor = tempBuffer;

	uint bytes = sizeof(bool);

	bool somethingSerialized;
	memcpy(&somethingSerialized, cursor, bytes);
	cursor += bytes;

	if (somethingSerialized)
	{
		bytes = sizeof(uint);
		uint goUID;
		memcpy(&goUID, cursor, bytes);
		cursor += bytes;

		uint scriptUID;
		memcpy(&scriptUID, cursor, bytes);
		cursor += bytes;

		uint nameSize;
		memcpy(&nameSize, cursor, bytes);
		cursor += bytes;

		std::string name;
		name.resize(nameSize);
		memcpy((char*)name.data(), cursor, nameSize);
		name.resize(nameSize);

		cursor += nameSize;

		//Search for the references
		GameObject* go = App->GOs->GetGameObjectByUID(goUID);
		if (go)
		{
			for (int i = 0; i < go->components.size(); ++i)
			{
				if (go->components[i]->UUID == scriptUID)
				{
					scriptInstance = App->scripting->MonoComponentFrom(go->components[i]);
					onClickGameObjectUUID = go->components[i]->GetParent()->GetUUID();
					onClickScriptUUID = go->components[i]->UUID;
					break;
				}
			}

			if (scriptInstance)
			{
				MonoClass* monoClass = mono_object_get_class(scriptInstance);

				void* iterator = 0;
				MonoMethod* method = mono_class_get_methods(monoClass, &iterator);
				while (method)
				{
					uint32_t flags = 0;
					uint32_t another = mono_method_get_flags(method, &flags);
					if (another & MONO_METHOD_ATTR_PUBLIC && !(another & MONO_METHOD_ATTR_STATIC))
					{
						std::string savedName = mono_method_get_name(method);
						if (savedName == name)
						{
							methodToCall = method;
							methodToCallName = savedName;
							break;
						}
					}
					method = mono_class_get_methods(monoClass, &iterator);
				}
			}
		}
	}
}

void ComponentButton::OnUniqueEditor()
{
#ifndef GAMEMODE
	if (ImGui::CollapsingHeader("Button", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (ImGui::InputText("Blind key", &input), ImGuiInputTextFlags_EnterReturnsTrue)
		{
			if (input[0] != '\0')
			{
				input[1] = '\0';
				button_blinded = (uint)SDL_GetScancodeFromKey(SDL_GetKeyFromName(input.data()));
			}
			else
				button_blinded = 0;
		}

		ImGui::Text(SDL_GetKeyName(SDL_GetKeyFromScancode((SDL_Scancode)button_blinded)));

		ResourceTexture* texture = nullptr;
		if (idleTexture != 0)
			texture = (ResourceTexture*)App->res->GetResource(idleTexture);
		ImGui::Button(texture == nullptr ? "Empty Idle Texture" : texture->GetName(), ImVec2(100.0f, 0.0f));
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::Text("%u", idleTexture);
			ImGui::EndTooltip();
		}
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TEXTURE_INSPECTOR_SELECTOR"))
			{
				idleTexture = *(uint*)payload->Data;

				parent->cmp_image->SetResImageUuid(idleTexture);
			}
			ImGui::EndDragDropTarget();
		}
		ImGui::SameLine();
		if (ImGui::Button("RT Idle"))
		{
			idleTexture = 0;

			parent->cmp_image->SetResImageUuid(idleTexture);
		}

		texture = nullptr;
		if (hoveredTexture != 0)
			texture = (ResourceTexture*)App->res->GetResource(hoveredTexture);
		ImGui::Button(texture == nullptr ? "Empty Hovered Texture" : texture->GetName(), ImVec2(100.0f, 0.0f));
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::Text("%u", hoveredTexture);
			ImGui::EndTooltip();
		}
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TEXTURE_INSPECTOR_SELECTOR"))
				hoveredTexture = *(uint*)payload->Data;
			ImGui::EndDragDropTarget();
		}
		ImGui::SameLine();
		if (ImGui::Button("RT Hovered"))
			hoveredTexture = 0;

		texture = nullptr;
		if (clickTexture != 0)
			texture = (ResourceTexture*)App->res->GetResource(clickTexture);
		ImGui::Button(texture == nullptr ? "Empty Click Texture" : texture->GetName(), ImVec2(100.0f, 0.0f));
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::Text("%u", clickTexture);
			ImGui::EndTooltip();
		}
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TEXTURE_INSPECTOR_SELECTOR"))
				clickTexture = *(uint*)payload->Data;
			ImGui::EndDragDropTarget();
		}
		ImGui::SameLine();
		if (ImGui::Button("RT Click"))
			clickTexture = 0;

		switch (state)
		{
		case IDLE:
			ImGui::Text("This button is IDLE.");
			break;
		case HOVERED:
			ImGui::Text("This button is Hovered.");
			break;
		case R_CLICK:
			ImGui::Text("This button is Right Clicked.");
			break;
		case L_CLICK:
			ImGui::Text("This button is Left Clicked.");
			break;
		}

		ImVec2 cursorPos = ImGui::GetCursorScreenPos();

		ImGui::SetCursorScreenPos({ cursorPos.x, cursorPos.y + 4 });

		ImGui::Text("OnClick: "); ImGui::SameLine();

		cursorPos = ImGui::GetCursorScreenPos();

		ImGui::SetCursorScreenPos({ cursorPos.x, cursorPos.y - 4 });

		uint buttonWidth = 0.65 * ImGui::GetWindowWidth();
		cursorPos = ImGui::GetCursorScreenPos();
		ImGui::ButtonEx(("##" + std::to_string(UUID)).data(), { (float)buttonWidth, 20 }, ImGuiButtonFlags_::ImGuiButtonFlags_Disabled);

		bool dragged = false;

		//Dragging GameObjects
		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GAMEOBJECTS_HIERARCHY", ImGuiDragDropFlags_AcceptBeforeDelivery | ImGuiDragDropFlags_AcceptNoDrawDefaultRect);
			if (payload)
			{
				GameObject* go = *(GameObject**)payload->Data;

				if (ImGui::IsMouseReleased(0))
				{
					draggedGO = go;
					dragged = true;
				}
			}
			ImGui::EndDragDropTarget();
		}

		if (dragged)
			ImGui::OpenPopup("OnClick Method");

		if (ImGui::BeginPopup("OnClick Method", 0))
		{
			std::vector<Component*> scripts = draggedGO->GetComponents(ComponentTypes::ScriptComponent);
			for (int i = 0; i < scripts.size(); ++i)
			{
				ComponentScript* script = (ComponentScript*)scripts[i];
				if (ImGui::BeginMenu(script->scriptName.data()))
				{
					MonoObject* compInstance = script->GetMonoComponent();
					MonoClass* compClass = mono_object_get_class(compInstance);

					const char* className = mono_class_get_name(compClass);

					bool somethingClicked = false;

					void* iterator = 0;
					MonoMethod* method = mono_class_get_methods(compClass, &iterator);
					while (method)
					{
						uint32_t flags = 0;
						uint32_t another = mono_method_get_flags(method, &flags);
						if (another & MONO_METHOD_ATTR_PUBLIC && !(another & MONO_METHOD_ATTR_STATIC))
						{
							std::string name = mono_method_get_name(method);
							if (name != ".ctor")
							{
								if (ImGui::MenuItem((name + "()").data()))
								{
									//Set this method and instance to be called
									methodToCall = method;
									scriptInstance = compInstance;

									methodToCallName = name;
									onClickScriptUUID = script->UUID;
									onClickGameObjectUUID = script->GetParent()->GetUUID();

									somethingClicked = true;
									break;
								}
							}
						}
						method = mono_class_get_methods(compClass, &iterator);
					}

					ImGui::EndMenu();

					if (somethingClicked)
						break;
				}
			}

			ImGui::EndPopup();
		}

		if (ImGui::IsItemClicked(0))
		{
			ImDrawList* drawList = ImGui::GetWindowDrawList();
			drawList->AddRectFilled(cursorPos, { cursorPos.x + buttonWidth, cursorPos.y + 20 }, ImGui::GetColorU32(ImGuiCol_::ImGuiCol_ButtonActive));
		}
		else if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
		{
			ImDrawList* drawList = ImGui::GetWindowDrawList();
			drawList->AddRectFilled(cursorPos, { cursorPos.x + buttonWidth, cursorPos.y + 20 }, ImGui::GetColorU32(ImGuiCol_::ImGuiCol_ButtonHovered));

			if (App->input->GetKey(SDL_SCANCODE_BACKSPACE) == KEY_DOWN)
			{
				methodToCall = nullptr;
				scriptInstance = nullptr;
			}
		}

		//Button text

		std::string text;

		if (methodToCall)
		{
			text = mono_method_get_name(methodToCall) + std::string("()");

			ImGui::SetCursorScreenPos({ cursorPos.x + 7, cursorPos.y + 3 });

			ImGui::Text(text.data());

			cursorPos = ImGui::GetCursorScreenPos();
			ImGui::SetCursorScreenPos({ cursorPos.x, cursorPos.y + 4 });

		}
	}
#endif
}

bool ComponentButton::MouseInScreen(const int* rect)
{
	uint mouseX = App->input->GetMouseX();
	uint mouseY = App->input->GetMouseY();

	return mouseX > rect[ComponentRectTransform::Rect::X] && mouseX < rect[ComponentRectTransform::Rect::X] + rect[ComponentRectTransform::Rect::XDIST]
		&& mouseY > rect[ComponentRectTransform::Rect::Y] && mouseY < rect[ComponentRectTransform::Rect::Y] + rect[ComponentRectTransform::Rect::YDIST];
}

void ComponentButton::SetOnClick(MonoObject* scriptInstance, std::string methodToCall)
{
	Component* script = App->scripting->ComponentFrom(scriptInstance);
	if (!script)
		return;

	//Set this method and instance to be called
	void* iterator = 0;
	MonoClass* scriptClass = mono_object_get_class(scriptInstance);
	MonoMethod* method = mono_class_get_methods(scriptClass, &iterator);

	while (method)
	{
		uint32_t flags = 0;
		uint32_t another = mono_method_get_flags(method, &flags);
		if (another & MONO_METHOD_ATTR_PUBLIC && !(another & MONO_METHOD_ATTR_STATIC))
		{
			std::string name = mono_method_get_name(method);
			if (name == methodToCall)
			{
				//Set this method and instance to be called
				this->methodToCall = method;
				this->scriptInstance = scriptInstance;

				methodToCallName = name;
				onClickScriptUUID = script->UUID;
				onClickGameObjectUUID = script->GetParent()->GetUUID();
				break;
			}
		}
		method = mono_class_get_methods(scriptClass, &iterator);
	}
}

void ComponentButton::SetIdleTexture(uint uuid)
{
	idleTexture = uuid;
	if(state == UIState::IDLE)
		parent->cmp_image->SetResImageUuid(idleTexture, true);
}

void ComponentButton::SetHoverTexture(uint uuid)
{
	hoveredTexture = uuid;
	if (state == UIState::HOVERED)
		parent->cmp_image->SetResImageUuid(hoveredTexture);
}

void ComponentButton::SetClickTexture(uint uuid)
{
	clickTexture = uuid;
	if (state == UIState::L_CLICK || state == UIState::R_CLICK)
		parent->cmp_image->SetResImageUuid(clickTexture);
}

void ComponentButton::SetNewKey(const char * key)
{
	input = key;
	button_blinded = (uint)SDL_GetScancodeFromKey(SDL_GetKeyFromName(key));
}

void ComponentButton::SetNewKey(uint key)
{
	input = SDL_GetKeyName(SDL_GetKeyFromScancode((SDL_Scancode)key));
	button_blinded = key;
}

void ComponentButton::LoadOnClickReference()
{
	if (onClickGameObjectUUID == 0 || onClickScriptUUID == 0)
		return;

	//Search for the references
	GameObject* go = App->GOs->GetGameObjectByUID(onClickGameObjectUUID);
	if (go)
	{
		for (int i = 0; i < go->components.size(); ++i)
		{
			if (go->components[i]->UUID == onClickScriptUUID)
			{
				scriptInstance = App->scripting->MonoComponentFrom(go->components[i]);
				break;
			}
		}

		if (scriptInstance)
		{
			MonoClass* monoClass = mono_object_get_class(scriptInstance);

			void* iterator = 0;
			MonoMethod* method = mono_class_get_methods(monoClass, &iterator);
			while (method)
			{
				uint32_t flags = 0;
				uint32_t another = mono_method_get_flags(method, &flags);
				if (another & MONO_METHOD_ATTR_PUBLIC && !(another & MONO_METHOD_ATTR_STATIC))
				{
					std::string savedName = mono_method_get_name(method);
					if (savedName == methodToCallName)
					{
						methodToCall = method;
						break;
					}
				}
				method = mono_class_get_methods(monoClass, &iterator);
			}
		}
	}
}

void ComponentButton::KeyPressed()
{
	if (methodToCall && scriptInstance)
	{
		MonoObject* exc = nullptr;
		if (IsTreeActive())
		{
			mono_runtime_invoke(methodToCall, scriptInstance, NULL, &exc);
			if (exc)
			{
				System_Event event;
				event.type = System_Event_Type::Pause;
				App->PushSystemEvent(event);
				App->Pause();

				MonoString* exceptionMessage = mono_object_to_string(exc, NULL);
				char* toLogMessage = mono_string_to_utf8(exceptionMessage);
				CONSOLE_LOG(LogTypes::Error, toLogMessage);
				mono_free(toLogMessage);
			}
		}
	}
}

void ComponentButton::RightClickPressed()
{
}

UIState ComponentButton::GetState() const
{
	return state;
}
