#include "Application.h"
#include "ModuleGui.h"
#include "ModuleWindow.h"
#include "ModuleRenderer3D.h"
#include "Panel.h"
#include "PanelInspector.h"
#include "PanelRandomNumber.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_opengl2.h"

ModuleGui::ModuleGui(Application* app, bool start_enabled) : Module(app, start_enabled)
{}

ModuleGui::~ModuleGui()
{}

bool ModuleGui::Start()
{
	bool ret = true;

	_LOG("Starting ImGui");

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable keyboard controls

	ImGui_ImplSDL2_InitForOpenGL(App->window->window, App->renderer3D->context);	
	ImGui_ImplOpenGL2_Init();

	// Setup style
	ImGui::StyleColorsDark();

	pInspector = new PanelInspector("Inspector");
	pRandomNumber = new PanelRandomNumber("Random Generator");
	panels.push_back(pInspector);
	panels.push_back(pRandomNumber);

	return ret;
}

update_status ModuleGui::PreUpdate(float dt) 
{
	// Start the frame
	ImGui_ImplOpenGL2_NewFrame();
	ImGui_ImplSDL2_NewFrame(App->window->window);
	ImGui::NewFrame();

	return UPDATE_CONTINUE;
}

update_status ModuleGui::Update(float dt)
{
	if ((App->input->GetKey(SDL_SCANCODE_LCTRL) == KEY_REPEAT || App->input->GetKey(SDL_SCANCODE_RCTRL) == KEY_REPEAT) && App->input->GetKey(SDL_SCANCODE_I) == KEY_DOWN) { pInspector->OnOff(); }
	if ((App->input->GetKey(SDL_SCANCODE_LCTRL) == KEY_REPEAT || App->input->GetKey(SDL_SCANCODE_RCTRL) == KEY_REPEAT) && App->input->GetKey(SDL_SCANCODE_D) == KEY_DOWN) {  }
	if ((App->input->GetKey(SDL_SCANCODE_LCTRL) == KEY_REPEAT || App->input->GetKey(SDL_SCANCODE_RCTRL) == KEY_REPEAT) && App->input->GetKey(SDL_SCANCODE_R) == KEY_DOWN) { pRandomNumber->OnOff(); }
	if ((App->input->GetKey(SDL_SCANCODE_LCTRL) == KEY_REPEAT || App->input->GetKey(SDL_SCANCODE_RCTRL) == KEY_REPEAT) && App->input->GetKey(SDL_SCANCODE_T) == KEY_DOWN) {  }
	if ((App->input->GetKey(SDL_SCANCODE_LCTRL) == KEY_REPEAT || App->input->GetKey(SDL_SCANCODE_RCTRL) == KEY_REPEAT) && App->input->GetKey(SDL_SCANCODE_P) == KEY_DOWN) {  }


	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New")) {}
			if (ImGui::MenuItem("Open")) {}
			ImGui::Separator();
			if (ImGui::MenuItem("Exit"))
				App->CloseApp();

			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Window"))
		{
			if (ImGui::MenuItem("Inspector Window", "CTRL+I")) { pInspector->OnOff(); }
			if (ImGui::MenuItem("Demo Window", "CTRL+D")) { }

			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Tools"))
		{
			if (ImGui::MenuItem("Random Generator", "CTRL+R" )) { pRandomNumber->OnOff(); }
			if (ImGui::MenuItem("Test Intersections", "CTRL+T")) { }
			if (ImGui::MenuItem("Performance", "CTRL+P")) { }

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("About"))
		{
			if (ImGui::MenuItem("GitHub", "CTRL+R")) {}
			if (ImGui::MenuItem("Issues", "CTRL+T")) {}
			if (ImGui::MenuItem("Performance", "CTRL+P")) {}

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	for (int i = 0; i < panels.size(); ++i)
	{
		if (panels[i]->IsEnabled())
			panels[i]->Draw();
	}

	return UPDATE_CONTINUE;
}

update_status ModuleGui::PostUpdate(float dt) 
{
	// Render
	ImGui::Render();
	ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

	return UPDATE_CONTINUE;
}

bool ModuleGui::CleanUp()
{
	bool ret = true;

	for (int i = 0; i < panels.size(); ++i)
		delete panels[i];

	_LOG("Cleaning up ImGui");

	ImGui_ImplOpenGL2_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	return ret;
}