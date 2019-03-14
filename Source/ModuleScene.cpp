#include "Globals.h"
#include "Application.h"
#include "ModuleScene.h"
#include "ModuleWindow.h"
#include "ModuleRenderer3D.h"
#include "ModuleFileSystem.h"
#include "ModuleInput.h"
#include "Primitive.h"
#include "SceneImporter.h"
#include "ModuleGOs.h"
#include "ModuleGui.h"
#include "GameObject.h"
#include "DebugDrawer.h"
#include "ComponentTransform.h"
#include "ComponentCamera.h"
#include "ComponentMesh.h"
#include "ModuleNavigation.h"

#include "imgui/imgui.h"

#include <list>
#include <vector>

ModuleScene::ModuleScene(bool start_enabled) : Module(start_enabled)
{
	name = "Scene";
}

ModuleScene::~ModuleScene() {}

bool ModuleScene::Init(JSON_Object* jObject)
{
	LoadStatus(jObject);

	quadtree.Create();

	return true;
}

bool ModuleScene::Start()
{
	grid = new PrimitiveGrid();
	grid->ShowAxis(true);
	root = new GameObject("Root", nullptr, true);
	GameObject* directionalLight = App->GOs->CreateGameObject("Directional Light", root);
	directionalLight->AddComponent(ComponentTypes::LightComponent);

	math::float3 axis;
	float angle;
	math::Quat rotation = rotation.identity;
	rotation.ToAxisAngle(axis, angle);
	axis *= angle;
	axis *= RADTODEG;
	axis[0] = -50;
	axis[1] = 30;
	axis *= DEGTORAD;
	rotation.SetFromAxisAngle(axis.Normalized(), axis.Length());
	directionalLight->transform->SetRotation(rotation);
	return true;
}

update_status ModuleScene::Update()
{
#ifndef GAMEMODE
	if (!App->IsEditor())
		return UPDATE_CONTINUE;

	if (!App->gui->WantTextInput())
	{
		if (App->input->GetMouseButton(SDL_BUTTON_RIGHT) == KEY_IDLE)
		{
			if (App->input->GetKey(SDL_SCANCODE_Q) == KEY_DOWN)
				SetImGuizmoOperation(ImGuizmo::OPERATION::BOUNDS);//None
			if (App->input->GetKey(SDL_SCANCODE_W) == KEY_DOWN)
				SetImGuizmoOperation(ImGuizmo::OPERATION::TRANSLATE);
			if (App->input->GetKey(SDL_SCANCODE_E) == KEY_DOWN)
				SetImGuizmoOperation(ImGuizmo::OPERATION::ROTATE);
			if (App->input->GetKey(SDL_SCANCODE_R) == KEY_DOWN)
				SetImGuizmoOperation(ImGuizmo::OPERATION::SCALE);
		}

		if (App->input->GetKey(SDL_SCANCODE_T) == KEY_DOWN)
		{
			if (currentImGuizmoMode == ImGuizmo::MODE::WORLD)
				SetImGuizmoMode(ImGuizmo::MODE::LOCAL);
			else
				SetImGuizmoMode(ImGuizmo::MODE::WORLD);
		}
	}

	if (selectedObject == CurrentSelection::SelectedType::gameObject)
	{
		GameObject* currentGameObject = (GameObject*)selectedObject.Get();
		OnGizmos(currentGameObject);
	}

	if(App->IsEditor() && !App->gui->WantTextInput())
		if (App->input->GetKey(SDL_SCANCODE_LCTRL) == KEY_REPEAT || App->input->GetKey(SDL_SCANCODE_RCTRL) == KEY_REPEAT)
		{
			if (App->input->GetKey(SDL_SCANCODE_Z) == KEY_DOWN)
				GetPreviousTransform();
		}
#endif

	return UPDATE_CONTINUE;
}

bool ModuleScene::CleanUp()
{
	bool ret = true;

	RELEASE(grid);
#ifndef GAMEMODE
	SELECT(NULL);
#endif

	quadtree.Clear();

	return ret;
}

void ModuleScene::OnSystemEvent(System_Event event)
{
	switch (event.type)
	{
	case System_Event_Type::LoadGMScene:
	{
		char* buf;
		if (!App->fs->Exists("Library/Scenes/Main Scene.scn"))
			return;

		size_t size = App->fs->Load("Library/Scenes/Main Scene.scn", &buf);
		if (size > 0)
		{
			App->GOs->LoadScene(buf, size, true);
			RELEASE_ARRAY(buf);

			App->renderer3D->SetCurrentCamera();
			App->renderer3D->OnResize(App->window->GetWindowWidth(), App->window->GetWindowHeight());

			// Initialize detour with the previous loaded navmesh
			App->navigation->InitDetour();
			
			System_Event newEvent;
			newEvent.type = System_Event_Type::RecreateQuadtree;
			App->PushSystemEvent(newEvent);
		}
	}
	break;

	case System_Event_Type::RecreateQuadtree:
	{
		RecreateQuadtree();
	}
	break;

#ifndef GAMEMODE
	case System_Event_Type::GameObjectDestroyed:

		// Remove GO in list if its deleted
		if (selectedObject == event.goEvent.gameObject)
			SELECT(NULL);
		std::list<LastTransform>::iterator iterator = prevTransforms.begin();

		while (!prevTransforms.empty() && iterator != prevTransforms.end())
		{
			if ((*iterator).uuidGO == event.goEvent.gameObject->GetUUID())
			{
				prevTransforms.erase(iterator);
				iterator = prevTransforms.begin();
			}
			else
				++iterator;
		}
		break;
#endif
	}
}

void ModuleScene::SaveStatus(JSON_Object* jObject) const
{
	json_object_set_boolean(jObject, "showGrid", showGrid);
}

void ModuleScene::LoadStatus(const JSON_Object* jObject)
{
	showGrid = json_object_get_boolean(jObject, "showGrid");
}

void ModuleScene::Draw() const
{
	if (showGrid)
		grid->Render();
}

#ifndef GAMEMODE
void ModuleScene::OnGizmos(GameObject* gameObject)
{
	if (gameObject->GetLayer() != UILAYER)
	{
		ImGuiViewport* vport = ImGui::GetMainViewport();
		ImGuizmo::SetRect(vport->Pos.x, vport->Pos.y, vport->Size.x, vport->Size.y);

		math::float4x4 viewMatrix = App->renderer3D->GetCurrentCamera()->GetOpenGLViewMatrix();
		math::float4x4 projectionMatrix = App->renderer3D->GetCurrentCamera()->GetOpenGLProjectionMatrix();
		math::float4x4 transformMatrix = gameObject->transform->GetGlobalMatrix();
		transformMatrix = transformMatrix.Transposed();

		ImGuizmo::MODE mode = currentImGuizmoMode;
		if (currentImGuizmoOperation == ImGuizmo::OPERATION::SCALE && mode != ImGuizmo::MODE::LOCAL)
			mode = ImGuizmo::MODE::LOCAL;

		ImGuizmo::Manipulate(
			viewMatrix.ptr(), projectionMatrix.ptr(),
			currentImGuizmoOperation, mode, transformMatrix.ptr()
		);

		if (ImGuizmo::IsUsing())
		{
			if (!saveTransform)
			{
				saveTransform = true;
				lastMat = transformMatrix;
			}
			transformMatrix = transformMatrix.Transposed();
			gameObject->transform->SetMatrixFromGlobal(transformMatrix);
		}
		else if (saveTransform)
		{
			SaveLastTransform(lastMat.Transposed());
			saveTransform = false;
		}
	}
}


void ModuleScene::SaveLastTransform(math::float4x4 matrix)
{
	LastTransform prevTrans;
	GameObject* curr = selectedObject.GetCurrGameObject();
	if (curr)
	{
		if (prevTransforms.size() >= MAX_UNDO)
			prevTransforms.pop_back();
		if (prevTransforms.empty() || curr->transform->GetGlobalMatrix().ptr() != (*prevTransforms.begin()).matrix.ptr())
		{
			prevTrans.matrix = matrix;
			prevTrans.uuidGO = curr->GetUUID();
			prevTransforms.push_front(prevTrans);
		}
	}
}

void ModuleScene::GetPreviousTransform()
{
	if (!prevTransforms.empty())
	{
		LastTransform prevTrans = (*prevTransforms.begin());
		GameObject* transObject = App->GOs->GetGameObjectByUID(prevTrans.uuidGO);

		if (transObject)
		{
			selectedObject = transObject;
			selectedObject.GetCurrGameObject()->transform->SetMatrixFromGlobal(prevTrans.matrix);
		}
		prevTransforms.pop_front();
	}
	// Bounding box changed: recreate quadtree
	System_Event newEvent;
	newEvent.type = System_Event_Type::RecreateQuadtree;
	App->PushSystemEvent(newEvent);
}

void ModuleScene::SetImGuizmoOperation(ImGuizmo::OPERATION operation)
{
	currentImGuizmoOperation = operation;
}

ImGuizmo::OPERATION ModuleScene::GetImGuizmoOperation() const
{
	return currentImGuizmoOperation;
}

void ModuleScene::SetImGuizmoMode(ImGuizmo::MODE mode)
{
	currentImGuizmoMode = mode;
}

ImGuizmo::MODE ModuleScene::GetImGuizmoMode() const
{
	return currentImGuizmoMode;
}
#endif

bool ModuleScene::GetShowGrid() const
{
	return showGrid;
}

void ModuleScene::SetShowGrid(bool showGrid)
{
	this->showGrid = showGrid;
}

void ModuleScene::RecreateQuadtree()
{
	std::vector<GameObject*> staticGameObjects;
	App->GOs->GetStaticGameobjects(staticGameObjects);

	quadtree.ReDoQuadtree(staticGameObjects);
}

void ModuleScene::CreateRandomStaticGameObject()
{
	GameObject* random = App->GOs->CreateGameObject("Random", root);
	random->transform->SetPosition(math::float3(rand() % (50 + 50 + 1) - 50, rand() % 10, rand() % (50 + 50 + 1) - 50));

	math::float3 pos = random->transform->GetPosition();

	const math::float3 center(pos.x, pos.y, pos.z);
	const math::float3 size(2.0f, 2.0f, 2.0f);
	random->boundingBox.SetFromCenterAndSize(center, size);
	random->originalBoundingBox.SetFromCenterAndSize(center, size);

	quadtree.Insert(random);
}

#ifndef GAMEMODE
bool ModuleScene::IsGizmoValid() const
{
	return ImGuizmo::IsOver() || ImGuizmo::IsUsing();
}
#endif

void ModuleScene::FreeRoot()
{
	RELEASE(root);
}
