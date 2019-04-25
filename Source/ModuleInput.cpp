#include "Globals.h"
#include "Application.h"
#include "ModuleInput.h"
#include "ModuleGui.h"
#include "SceneImporter.h"
#include "ModuleWindow.h"
#include "MaterialImporter.h"
#include "ModuleRenderer3D.h"
#include "ModuleResourceManager.h"
#include "ResourceTexture.h"
#include "GLCache.h"

#include "imgui\imgui.h"
#include "imgui\imgui_impl_sdl.h"
#include "imgui\imgui_impl_opengl3.h"

#include "glew\include\GL\glew.h"

#define MAX_KEYS 300

ModuleInput::ModuleInput(bool start_enabled) : Module(start_enabled)
{
	name = "Input";

	keyboard = new KEY_STATE[MAX_KEYS];
	memset(keyboard, KEY_IDLE, sizeof(KEY_STATE) * MAX_KEYS);
	memset(mouse_buttons, KEY_IDLE, sizeof(KEY_STATE) * MAX_MOUSE_BUTTONS);
}

ModuleInput::~ModuleInput()
{
	delete[] keyboard;
}

bool ModuleInput::Init(JSON_Object* jObject)
{
	CONSOLE_LOG(LogTypes::Normal, "Init SDL input event system");

	bool ret = true;

	SDL_Init(0);

	if (SDL_InitSubSystem(SDL_INIT_EVENTS) < 0)
	{
		CONSOLE_LOG(LogTypes::Error, "SDL_EVENTS could not initialize! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}

	LoadStatus(jObject);

	return ret;
}

update_status ModuleInput::PreUpdate()
{
	SDL_PumpEvents();

	const Uint8* keys = SDL_GetKeyboardState(NULL);
	
	for (int i = 0; i < MAX_KEYS; ++i)
	{
		if (keys[i] == 1)
		{
			if (keyboard[i] == KEY_IDLE)
			{
				keyboard[i] = KEY_DOWN;
#ifndef GAMEMODE
				App->gui->AddInput(i, KEY_DOWN);
#endif
			}
			else
			{
				keyboard[i] = KEY_REPEAT;
#ifndef GAMEMODE
				App->gui->AddInput(i, KEY_REPEAT);
#endif
			}
		}
		else
		{
			if (keyboard[i] == KEY_REPEAT || keyboard[i] == KEY_DOWN)
			{
				keyboard[i] = KEY_UP;
#ifndef GAMEMODE
				App->gui->AddInput(i, KEY_UP);
#endif
			}
			else
				keyboard[i] = KEY_IDLE;
		}
	}

	Uint32 buttons = SDL_GetMouseState(&mouse_x, &mouse_y);

	uint screenSize = App->window->GetScreenSize();
	mouse_x /= screenSize;
	mouse_y /= screenSize;
	mouse_z = 0;

	for (int i = 0; i < 5; ++i)
	{
		if (buttons & SDL_BUTTON(i))
		{
			if (mouse_buttons[i] == KEY_IDLE)
			{
				mouse_buttons[i] = KEY_DOWN;
#ifndef GAMEMODE
				App->gui->AddInput(i, KEY_DOWN);
#endif
			}
			else
			{
				mouse_buttons[i] = KEY_REPEAT;
#ifndef GAMEMODE
				App->gui->AddInput(i, KEY_REPEAT);
#endif
			}
		}
		else
		{
			if (mouse_buttons[i] == KEY_REPEAT || mouse_buttons[i] == KEY_DOWN)
			{
				mouse_buttons[i] = KEY_UP;
#ifndef GAMEMODE
				App->gui->AddInput(i, KEY_UP);
#endif
			}
			else
				mouse_buttons[i] = KEY_IDLE;
		}
	}

	mouse_x_motion = mouse_y_motion = 0;

	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
#ifndef GAMEMODE	
		ImGui_ImplSDL2_ProcessEvent(&event);
#endif
		switch (event.type)
		{
		case SDL_MOUSEWHEEL:
			mouse_z = event.wheel.y;
			break;

		case SDL_MOUSEMOTION:
			mouse_x = event.motion.x / screenSize;
			mouse_y = event.motion.y / screenSize;

			mouse_x_motion = event.motion.xrel / screenSize;
			mouse_y_motion = event.motion.yrel / screenSize;
			break;

		case SDL_QUIT:
			App->CloseApp();
			break;

		case SDL_WINDOWEVENT:
		{
			if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
				App->window->SetWindowWidth(event.window.data1);
				App->window->SetWindowHeight(event.window.data2);
				App->window->UpdateWindowSize();
			}
			break;
		}
		case (SDL_DROPFILE):
		{	
			System_Event newEvent;
			newEvent.type = System_Event_Type::FileDropped;			
			strcpy_s(newEvent.fileEvent.file, DEFAULT_BUF_SIZE, event.drop.file);
			App->PushSystemEvent(newEvent);

			SDL_free(event.drop.file);

			break;
		}
		}
	}

	return UPDATE_CONTINUE;
}

bool ModuleInput::CleanUp()
{
	bool ret = true;

	CONSOLE_LOG(LogTypes::Normal, "Quitting SDL input event subsystem");
	SDL_QuitSubSystem(SDL_INIT_EVENTS);

	return ret;
}

void ModuleInput::DrawCursor()
{
	if (CursorTextureUUID != 0 && CursorTextureID == 0)
	{
		Resource* res = App->res->GetResource(CursorTextureUUID);
		if (res)
		{
			App->res->SetAsUsed(CursorTextureUUID);
			CursorTextureID = ((ResourceTexture*)res)->GetId();
		}
	}

#ifndef GAMEMODE
	if (App->GetEngineState() == engine_states::ENGINE_PLAY && CursorTextureID != 0u)
		ImGui::SetMouseCursor(ImGuiMouseCursor_::ImGuiMouseCursor_None);
#else
	if (App->GetEngineState() == engine_states::ENGINE_PLAY && CursorTextureID != 0u)
		SDL_ShowCursor(SDL_DISABLE);
#endif

	if (App->GetEngineState() == engine_states::ENGINE_PLAY)
	{
		if (CursorTextureID != 0)
		{
			uint windowWidth = App->window->GetWindowWidth();
			uint windowHeight = App->window->GetWindowHeight();
			
			glDisable(GL_LIGHTING);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			
			glMatrixMode(GL_PROJECTION);
			glPushMatrix();
			glLoadIdentity();
			glOrtho(0.0f, windowWidth, windowHeight, 0.0f, -1.0f, 1.0f);

			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glLoadIdentity();
			glTranslatef(mouse_x, mouse_y, -0.5);

			App->glCache->SwitchShader(0);
			glEnable(GL_TEXTURE_2D);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, CursorTextureID);
			GLenum error = glGetError();

			bool sad = error == GL_INVALID_OPERATION;

			glColor4f(1.0, 1.0f, 1.0f, 1.0f);

			glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0.0f);
			glVertex3f(0.0f, cursorSize, 0.0f);
			glTexCoord2f(1.0f, 0.0f);
			glVertex3f(cursorSize, cursorSize, 0.0f);
			glTexCoord2f(1.0f, 1.0f);
			glVertex3f(cursorSize, 0.0f, 0.0f);
			glTexCoord2f(0.0f, 1.0f);
			glVertex3f(0.0f, 0.0f, 0.0f);
			glEnd();

			glMatrixMode(GL_MODELVIEW);
			glPopMatrix();
			glMatrixMode(GL_PROJECTION);
			glPopMatrix();

			glBindTexture(GL_TEXTURE_2D, 0);
			
			glEnable(GL_LIGHTING);
			glDisable(GL_BLEND);

			glMatrixMode(GL_MODELVIEW);
		}
	}
}

void ModuleInput::SaveStatus(JSON_Object* node) const
{
	json_object_set_number(node, "DefCursor", CursorTextureUUID);
}

void ModuleInput::LoadStatus(const JSON_Object* node)
{
	if (CursorTextureUUID != 0)
	{
		App->res->SetAsUnused(CursorTextureUUID);
		CursorTextureUUID = 0u;
	}

	CursorTextureUUID = (uint)json_object_get_number(node, "DefCursor");	
	if (CursorTextureUUID != 0)
	{
		Resource* res = App->res->GetResource(CursorTextureUUID);
		if (res)
		{
			App->res->SetAsUsed(CursorTextureUUID);
			CursorTextureID = ((ResourceTexture*)res)->GetId();
		}
	}
}

std::string ModuleInput::GetCursorTexture() const
{
	ResourceTexture* texture = CursorTextureUUID != 0u ? (ResourceTexture*)App->res->GetResource(CursorTextureUUID) : nullptr;
	if (texture)
	{
		return texture->GetData().name;
	}
	return "";
}

void ModuleInput::SetCursorTexture(std::string& textureName)
{
	std::vector<Resource*> resources = App->res->GetResourcesByType(ResourceTypes::TextureResource);
	for (Resource* res : resources)
	{
		if (res->GetData().name == textureName)
		{
			if (CursorTextureUUID != 0)
				App->res->SetAsUnused(CursorTextureUUID);

			CursorTextureUUID = res->GetUuid();
			App->res->SetAsUsed(CursorTextureUUID);
			CursorTextureID = ((ResourceTexture*)res)->GetId();

			break;
		}
	}
}

void ModuleInput::SetCursorTexture(uint textureUUID)
{	
	ResourceTexture* textureRes = (ResourceTexture*)App->res->GetResource(textureUUID);
	if (!textureRes)
		return;

	if (CursorTextureUUID != 0)
		App->res->SetAsUnused(CursorTextureUUID);

	CursorTextureUUID = textureUUID;
	App->res->SetAsUsed(CursorTextureUUID);
	CursorTextureID = textureRes->GetId();
}

void ModuleInput::SetDefaultCursorTexture(ResourceTexture* textureRes)
{
	if (CursorTextureUUID != 0)
		App->res->SetAsUnused(CursorTextureUUID);

	CursorTextureUUID = textureRes->GetUuid();
	App->res->SetAsUsed(CursorTextureUUID);
	CursorTextureID = textureRes->GetId();

	App->SaveState();
}
