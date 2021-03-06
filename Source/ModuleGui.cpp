#include "ModuleGui.h"

#ifndef GAMEMODE

#include "Application.h"
#include "Globals.h"
#include "ModuleWindow.h"
#include "ModuleRenderer3D.h"
#include "ModuleTimeManager.h"
#include "ModuleScene.h"
#include "ModuleGOs.h"
#include "ModuleInput.h"
#include "MaterialImporter.h"
#include "ResourceTexture.h"
#include "ModuleResourceManager.h"

#include "Panel.h"
#include "PanelInspector.h"
#include "PanelAbout.h"
#include "PanelConsole.h"
#include "PanelSettings.h"
#include "PanelHierarchy.h"
#include "PanelAssets.h"
#include "PanelDebugDraw.h"
#include "PanelEdit.h"
#include "PanelCodeEditor.h"
#include "PanelShaderEditor.h"
#include "PanelNavigation.h"
#include "PanelSimulatedTime.h"
#include "PanelPhysics.h"
#include "PanelLayers.h"
#include "PanelUI.h"
#include "PanelUIAnimation.h"

#include "imgui\imgui.h"
#include "imgui\imgui_impl_sdl.h"
#include "imgui\imgui_impl_opengl3.h"
#include "imgui\imgui_internal.h"

#include "Optick/include/optick.h"
#include "ImGuizmo\ImGuizmo.h"

ModuleGui::ModuleGui(bool start_enabled) : Module(start_enabled)
{
	name = "GUI";
}

ModuleGui::~ModuleGui() {}

bool ModuleGui::Init(JSON_Object* jObject)
{
	panelInspector = new PanelInspector("Inspector");
	panelAbout = new PanelAbout("About");
	panelSettings = new PanelSettings("Settings");
	panelHierarchy = new PanelHierarchy("Hierarchy");
	panelConsole = new PanelConsole("Console");
	panelAssets = new PanelAssets("Assets");
	panelEdit = new PanelEdit("Edit");
	panelDebugDraw = new PanelDebugDraw("Debug Draw");
	panelCodeEditor = new PanelCodeEditor("Code Editor");
	panelShaderEditor = new PanelShaderEditor("Shader Editor");
	panelNavigation = new PanelNavigation("Navigation");
	panelSimulatedTime = new PanelSimulatedTime("Simulated Time");
	panelPhysics = new PanelPhysics("Physics");
	panelLayers = new PanelLayers("Layers");
	panelUI = new PanelUI("UI");
	panelUIAnimation = new PanelUIAnimation("UI Animation");

	panels.push_back(panelInspector);
	panels.push_back(panelAbout);
	panels.push_back(panelSettings);
	panels.push_back(panelHierarchy);
	panels.push_back(panelConsole);
	panels.push_back(panelAssets);
	panels.push_back(panelEdit);
	panels.push_back(panelDebugDraw);
	panels.push_back(panelCodeEditor);
	panels.push_back(panelShaderEditor);
	panels.push_back(panelNavigation);
	panels.push_back(panelSimulatedTime);
	panels.push_back(panelPhysics);
	panels.push_back(panelLayers);
	panels.push_back(panelUI);
	panels.push_back(panelUIAnimation);

	LoadStatus(jObject);

	return true;
}

bool ModuleGui::Start()
{
	bool ret = true;

	CONSOLE_LOG(LogTypes::Normal, "Starting ImGui");

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable keyboard controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
	io.IniFilename = nullptr;
	io.ConfigResizeWindowsFromEdges = true;

	ImGui_ImplSDL2_InitForOpenGL(App->window->window, App->renderer3D->context);	
	ImGui_ImplOpenGL3_Init();

	// Setup style
	ImGui::StyleColorsLight();

	// Load atlas texture // TODO: ATLAS
	atlas = (ResourceTexture*)App->res->ImportFile("Settings/atlas.png");
	App->res->SetAsUsed(atlas->GetUuid());

	return ret;
}

update_status ModuleGui::PreUpdate() 
{
#ifndef GAMEMODE
	OPTICK_CATEGORY("ModuleGUI_PreUpdate", Optick::Category::UI);
#endif // !GAMEMODE
	// Start the frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(App->window->window);
	ImGui::NewFrame();

	ImGuizmo::BeginFrame();

	// Begin dock space
	DockSpace();

	return UPDATE_CONTINUE;
}

update_status ModuleGui::Update()
{
#ifndef GAMEMODE
	OPTICK_CATEGORY("ModuleGUI_Update", Optick::Category::UI);
#endif

#ifdef _DEBUG
	static bool imguiDemo = false;
	if (imguiDemo)
		ImGui::ShowDemoWindow();
#endif // _DEBUG

	if ((App->input->GetKey(SDL_SCANCODE_LALT) == KEY_REPEAT || App->input->GetKey(SDL_SCANCODE_RALT) == KEY_REPEAT) && App->input->GetKey(SDL_SCANCODE_E) == KEY_DOWN) { panelEdit->OnOff(); }
	if ((App->input->GetKey(SDL_SCANCODE_LALT) == KEY_REPEAT || App->input->GetKey(SDL_SCANCODE_RALT) == KEY_REPEAT) && App->input->GetKey(SDL_SCANCODE_I) == KEY_DOWN) { panelInspector->OnOff(); }
	if ((App->input->GetKey(SDL_SCANCODE_LALT) == KEY_REPEAT || App->input->GetKey(SDL_SCANCODE_RALT) == KEY_REPEAT) && App->input->GetKey(SDL_SCANCODE_S) == KEY_DOWN) { panelSettings->OnOff(); }
	if ((App->input->GetKey(SDL_SCANCODE_LALT) == KEY_REPEAT || App->input->GetKey(SDL_SCANCODE_RALT) == KEY_REPEAT) && App->input->GetKey(SDL_SCANCODE_C) == KEY_DOWN) { panelConsole->OnOff(); }
	if ((App->input->GetKey(SDL_SCANCODE_LALT) == KEY_REPEAT || App->input->GetKey(SDL_SCANCODE_RALT) == KEY_REPEAT) && App->input->GetKey(SDL_SCANCODE_H) == KEY_DOWN) { panelHierarchy->OnOff(); }
	if ((App->input->GetKey(SDL_SCANCODE_LALT) == KEY_REPEAT || App->input->GetKey(SDL_SCANCODE_RALT) == KEY_REPEAT) && App->input->GetKey(SDL_SCANCODE_A) == KEY_DOWN) { panelAssets->OnOff(); }
	if ((App->input->GetKey(SDL_SCANCODE_LALT) == KEY_REPEAT || App->input->GetKey(SDL_SCANCODE_RALT) == KEY_REPEAT) && App->input->GetKey(SDL_SCANCODE_D) == KEY_DOWN) { panelDebugDraw->OnOff(); }
	if ((App->input->GetKey(SDL_SCANCODE_LALT) == KEY_REPEAT || App->input->GetKey(SDL_SCANCODE_RALT) == KEY_REPEAT) && App->input->GetKey(SDL_SCANCODE_L) == KEY_DOWN) { panelLayers->OnOff(); }
	
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open In Explorer")) { OpenInExplorer(); }
			if (ImGui::MenuItem("Save")) { 
				App->SaveState(); }
			if (ImGui::MenuItem("Load")) { App->LoadState(); }
			ImGui::Separator();
			if (ImGui::MenuItem("Save Scene")) { showSaveScenePopUp = true; }
			if (ImGui::MenuItem("Load Scene")) { showLoadScenePopUp = true; }
			ImGui::Separator();
			if (ImGui::MenuItem("Exit"))
				App->CloseApp();

			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Edit", "ALT+E")) { panelEdit->OnOff(); }
			if (ImGui::MenuItem("Shader Editor")) { panelShaderEditor->OnOff(); }
			if (ImGui::MenuItem("Physics")) { panelPhysics->OnOff(); }

			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Window"))
		{
			if (ImGui::MenuItem("Show All Windows")) { ShowAllWindows(); }
			if (ImGui::MenuItem("Hide All Windows")) { HideAllWindows(); }
			if (ImGui::MenuItem("Inspector", "ALT+I")) { panelInspector->OnOff(); }
			if (ImGui::MenuItem("Settings", "ALT+S")) { panelSettings->OnOff(); }
			if (ImGui::MenuItem("Console", "ALT+C")) { panelConsole->OnOff(); }
			if (ImGui::MenuItem("Hierarchy", "ALT+H")) { panelHierarchy->OnOff(); }
			if (ImGui::MenuItem("Assets", "ALT+A")) { panelAssets->OnOff(); }
			if (ImGui::MenuItem("Debug Draw", "ALT+D")) { panelDebugDraw->OnOff(); }

			if (ImGui::MenuItem("Navigation")) { panelNavigation->OnOff(); }

#ifdef _DEBUG
			if (ImGui::MenuItem("ImGui Demo")) { imguiDemo = !imguiDemo; }
#endif

			if (ImGui::MenuItem("Layers", "ALT+L")) { panelLayers->OnOff(); }

			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Others"))
		{
			if (ImGui::MenuItem("Documentation")) { OpenInBrowser("https://github.com/WickedNekomata/NekoEngine"); }
			if (ImGui::MenuItem("Latest Release")) { OpenInBrowser("https://github.com/WickedNekomata/NekoEngine/releases"); }
			if (ImGui::MenuItem("Bug Report")) { OpenInBrowser("https://github.com/WickedNekomata/NekoEngine/issues"); }
			if (ImGui::MenuItem("About")) { panelAbout->OnOff(); }

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	try
	{
		for (uint i = 0; i < panels.size(); ++i)
		{
			if (panels[i]->IsEnabled())
				panels[i]->Draw();
		}
	}
	catch(...)
	{

	}

	if (showSaveScenePopUp)
	{
		ImGui::OpenPopup("Save Scene as");
		SaveScenePopUp();
	}

	if (showLoadScenePopUp)
	{
		ImGui::OpenPopup("Load Scene");
		LoadScenePopUp();
	}


	return UPDATE_CONTINUE;
}

update_status ModuleGui::PostUpdate()
{
#ifndef GAMEMODE
	OPTICK_CATEGORY("ModuleGUI_PostUpdate", Optick::Category::UI);
#endif // !GAMEMODE
	// End dock space
	ImGui::End();

	return UPDATE_CONTINUE;
}

bool ModuleGui::CleanUp()
{
	for (uint i = 0; i < panels.size(); ++i)
		RELEASE(panels[i]);

	panelInspector = nullptr;
	panelAbout = nullptr;
	panelConsole = nullptr;
	panelSettings = nullptr;
	panelHierarchy = nullptr;
	panelAssets = nullptr;
	panelDebugDraw = nullptr;
	panelEdit = nullptr;
	panelCodeEditor = nullptr;
	panelShaderEditor = nullptr;
	panelSimulatedTime = nullptr;
	panelPhysics = nullptr;
	panelLayers = nullptr;
	panelUI = nullptr;
	panelUIAnimation = nullptr;
	
	App->res->SetAsUnused(atlas->GetUuid());

	CONSOLE_LOG(LogTypes::Normal, "Cleaning up ImGui");

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	return true;
}

void ModuleGui::SaveStatus(JSON_Object* jObject) const
{
	for (int i = 0; i < panels.size(); ++i)
		json_object_set_boolean(jObject, panels[i]->GetName(), panels[i]->IsEnabled());
}

void ModuleGui::LoadStatus(const JSON_Object* jObject)
{
	for (int i = 0; i < panels.size(); ++i)
		panels[i]->SetOnOff(json_object_get_boolean(jObject, panels[i]->GetName()));
}

void ModuleGui::Draw() const 
{
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
}

void ModuleGui::DockSpace() const
{
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::SetNextWindowBgAlpha(0.0f);

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	static bool p_open = true;
	ImGui::Begin("DockSpace Demo", &p_open, window_flags);
	ImGui::PopStyleVar(3);
	
	if (ImGui::DockBuilderGetNode(ImGui::GetID("MyDockspace")) == NULL)
	{		
		ImGuiID dockspace_id = ImGui::GetID("MyDockspace");
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::DockBuilderRemoveNode(dockspace_id); // Clear out existing layout
		ImGui::DockBuilderAddNode(dockspace_id, viewport->Size); // Add empty node

		ImGuiID dock_main_id = dockspace_id; // This variable will track the document node, however we are not using it here as we aren't docking anything into it.
		ImGuiID dock_id_up = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Up, 0.1f, NULL, &dock_main_id);
		ImGuiID dock_id_bottom = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.2f, NULL, &dock_main_id);
		ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.2f, NULL, &dock_main_id);
		ImGuiID dock_id_right = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.3f, NULL, &dock_main_id);	
				
		ImGui::DockBuilderDockWindow(panelDebugDraw->GetName(), dock_id_up);
		ImGui::DockBuilderDockWindow(panelEdit->GetName(), dock_id_up);
		ImGui::DockBuilderDockWindow(panelUI->GetName(), dock_id_up);
		ImGui::DockBuilderDockWindow(panelHierarchy->GetName(), dock_id_left);
		ImGui::DockBuilderDockWindow(panelNavigation->GetName(), dock_id_right);
		ImGui::DockBuilderDockWindow(panelInspector->GetName(), dock_id_right);
		ImGui::DockBuilderDockWindow(panelConsole->GetName(), dock_id_bottom);
		ImGui::DockBuilderDockWindow(panelAssets->GetName(), dock_id_bottom);

		ImGui::DockBuilderFinish(dockspace_id);
	}

	ImGuiID dockspace_id = ImGui::GetID("MyDockspace");
	ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruDockspace;
	ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
}

void ModuleGui::SaveScenePopUp()
{
	if (ImGui::BeginPopupModal("Save Scene as", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Scene will be saved to the following directory:");
		ImGui::Separator();

		ImGui::Text("Assets/Scenes/");

		ImGui::PushItemWidth(200.0f);
		ImGui::InputText("##sceneName", (char*)App->GOs->nameScene, DEFAULT_BUF_SIZE);
		
		if (ImGui::Button("Save", ImVec2(120, 0))) {
			ImGui::CloseCurrentPopup(); 

			System_Event newEvent;
			newEvent.sceneEvent.type = System_Event_Type::SaveScene;
			memcpy(newEvent.sceneEvent.nameScene, App->GOs->nameScene, sizeof(char) * DEFAULT_BUF_SIZE);
			App->PushSystemEvent(newEvent);
			
			showSaveScenePopUp = false;
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); showSaveScenePopUp = false; }
		ImGui::EndPopup();
	}
}

void ModuleGui::LoadScenePopUp()
{
	if (ImGui::BeginPopupModal("Load Scene", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("The scene will be searched in the following directory:");
		ImGui::Separator();

		ImGui::Text("Assets/Scenes/");

		static char sceneToLoad[DEFAULT_BUF_SIZE];
		ImGuiInputTextFlags inputFlag = ImGuiInputTextFlags_EnterReturnsTrue;
		ImGui::PushItemWidth(100.0f);
		ImGui::InputText("##sceneName", sceneToLoad, IM_ARRAYSIZE(sceneToLoad), inputFlag);	

		if (ImGui::Button("Load", ImVec2(120, 0))) {
			ImGui::CloseCurrentPopup();
			System_Event newEvent;
			newEvent.sceneEvent.type = System_Event_Type::LoadScene;
			memcpy(newEvent.sceneEvent.nameScene, sceneToLoad, sizeof(char) * DEFAULT_BUF_SIZE);
			App->PushSystemEvent(newEvent);
			showLoadScenePopUp = false;
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); showLoadScenePopUp = false; }
		ImGui::EndPopup();
	}
}

void ModuleGui::ShowAllWindows()
{
	for (uint i = 0; i < panels.size(); ++i)
	{
		if (panels[i] != panelCodeEditor)
			panels[i]->SetOnOff(true);
	}
}

void ModuleGui::HideAllWindows()
{
	for (uint i = 0; i < panels.size(); ++i)
		panels[i]->SetOnOff(false);
}

void ModuleGui::LogConsole(const char* log) const
{
	if (panelConsole != nullptr)
		panelConsole->AddLog(log);
}

void ModuleGui::ClearConsole() const
{
	if (panelConsole != nullptr)
		panelConsole->Clear();
}

void ModuleGui::AddInput(uint key, uint state) const
{
	static char input[INPUT_BUF_SIZE];
	static const char* states[] = { "IDLE", "DOWN", "REPEAT", "UP" };

	if (panelSettings != nullptr)
	{
		if (key < 1000)
			sprintf_s(input, INPUT_BUF_SIZE, "Keybr: %02u - %s\n", key, states[state]);
		else
			sprintf_s(input, INPUT_BUF_SIZE, "Mouse: %02u - %s\n", key - 1000, states[state]);
		panelSettings->AddInput(input);
	}
}

bool ModuleGui::IsMouseHoveringAnyWindow()
{
	return ImGui::IsMouseHoveringAnyWindow() || ImGui::IsAnyItemHovered() || ImGui::IsItemDeactivatedAfterChange() || ImGui::IsAnyWindowHovered();
}

bool ModuleGui::IsAnyItemFocused()
{
	return ImGui::IsAnyItemFocused();
}

bool ModuleGui::WantTextInput()
{
	ImGuiContext& g = *GImGui;
	return g.IO.WantTextInput;
}

#endif // GAME