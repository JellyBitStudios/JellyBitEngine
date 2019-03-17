#include "PanelHierarchy.h"

#ifndef GAMEMODE

#include "Application.h"
#include "GameObject.h"
#include "ComponentMesh.h"
#include "ModuleScene.h"
#include "ModuleInput.h"
#include "ModuleGui.h"
#include "ModuleInternalResHandler.h"
#include "ModuleResourceManager.h"
#include "ModuleUI.h"

#include "SDL\include\SDL_scancode.h"
#include "ModuleGOs.h"
#include "ImGui\imgui.h"
#include "imgui\imgui_internal.h"

#include "ComponentTransform.h"
#include "ComponentRectTransform.h"

#include "ResourcePrefab.h"

PanelHierarchy::PanelHierarchy(const char* name) : Panel(name) {}

PanelHierarchy::~PanelHierarchy() {}

bool PanelHierarchy::Draw()
{
	ImGuiWindowFlags hierarchyFlags = 0;
	hierarchyFlags |= ImGuiWindowFlags_NoFocusOnAppearing;

	if (ImGui::Begin(name, &enabled, hierarchyFlags))
	{
		ImGui::Text("Scene:"); ImGui::SameLine();

		static char sceneName[INPUT_BUF_SIZE];
		strcpy_s(sceneName, IM_ARRAYSIZE(sceneName), App->GOs->nameScene);
		ImGuiInputTextFlags inputFlag = ImGuiInputTextFlags_EnterReturnsTrue;
		ImGui::PushItemWidth(100.0f);
		if (ImGui::InputText("##sceneName", sceneName, IM_ARRAYSIZE(sceneName), inputFlag))
			strcpy_s((char*)App->GOs->nameScene, DEFAULT_BUF_SIZE, sceneName);

		ImGui::Separator();
		ImGui::Dummy(ImVec2(ImGui::GetWindowSize().x, 2.0f));
		GameObject* root = App->scene->root;

		// DRAGNDROP FOR ROOT AT SEPARATOR
		SetGameObjectDragAndDropTarget(App->scene->root);

		if (ImGui::BeginPopupContextWindow())
		{
			if (ImGui::Selectable("Create Empty"))
			{
				GameObject* newGO = App->GOs->CreateGameObject("GameObject", root);
				ImGui::CloseCurrentPopup();
				SELECT(newGO);
			}
			if(!App->GOs->ExistCanvas())
			{
				if (ImGui::Selectable("Create Screen Canvas"))
				{
					GameObject* newGO = App->GOs->CreateCanvas("Canvas", root);
					ImGui::CloseCurrentPopup();
					SELECT(newGO);
				}
			}
			if (ImGui::Selectable("Create Cube"))
			{
				GameObject* go = App->GOs->CreateGameObject("Cube", root);
				go->AddComponent(ComponentTypes::MeshComponent);
				go->cmp_mesh->SetResource(App->resHandler->cube);
				SELECT(go);
			}
			if (ImGui::Selectable("Create Plane"))
			{
				GameObject* go = App->GOs->CreateGameObject("Plane", root);
				go->AddComponent(ComponentTypes::MeshComponent);
				go->cmp_mesh->SetResource(App->resHandler->plane);
				SELECT(go);
			}
			if (ImGui::Selectable("Create World Canvas"))
			{
				GameObject* go = App->GOs->CreateGameObject("WoldCanvas", root);
				ComponentRectTransform* new_rect = new ComponentRectTransform(go, ComponentTypes::RectTransformComponent, ComponentRectTransform::RectFrom::WORLD);
				go->AddComponent(go->cmp_rectTransform = new_rect);
				SELECT(go);
			}

			ImGui::EndPopup();
		}

		IterateAllChildren(root);
	}
	ImGui::End();
	ImRect rect(ImGui::GetWindowPos(), ImGui::GetWindowSize());
	if (ImGui::BeginDragDropTargetCustom(rect, ImGui::GetID(name)))
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("PREFAB_RESOURCE", 0))
		{
			ResourcePrefab* prefab = *(ResourcePrefab**)payload->Data;
			App->res->SetAsUsed(prefab->GetUuid());

			GameObject* tmp_go = App->GOs->Instanciate(prefab->GetRoot(), App->scene->root);

			App->res->SetAsUnused(prefab->GetUuid());
		}

		ImGui::EndDragDropTarget();
	}
	if (App->input->GetKey(SDL_SCANCODE_LCTRL) == KEY_REPEAT && //You Found a Easter EGG!
		App->input->GetKey(SDL_SCANCODE_D) == KEY_DOWN)
	{
		GameObject* toDuplicate = App->scene->selectedObject.GetCurrGameObject();
		if (toDuplicate) App->GOs->Instanciate(toDuplicate, toDuplicate->GetParent());
	}


	return true;
}

void PanelHierarchy::IterateAllChildren(GameObject* root) const
{
	ImGuiTreeNodeFlags treeNodeFlags;

	if (root->HasChildren())
	{
		for (int i = 0; i < root->GetChildrenLength(); ++i)
		{
			GameObject* child = root->GetChild(i);

			char name[DEFAULT_BUF_SIZE];
			sprintf_s(name, DEFAULT_BUF_SIZE, "%s##%u", child->GetName(), child->GetUUID());

			if (child->HasChildren())
			{
				treeNodeFlags = 0;
				treeNodeFlags |= ImGuiTreeNodeFlags_OpenOnArrow;

				if (App->scene->selectedObject == child)
					treeNodeFlags |= ImGuiTreeNodeFlags_Selected;

				bool treeNodeOpened = false;
				if (child->openHierarchy)
				{
					treeNodeOpened = true;
					ImGui::SetNextTreeNodeOpen(true);

					child->openHierarchy = false;
				}

				if (ImGui::TreeNodeEx(name, treeNodeFlags))
					treeNodeOpened = true;

				SetGameObjectDragAndDrop(child);
				AtGameObjectPopUp(child);

				if (App->scene->selectedObject == child
					&& !App->gui->WantTextInput() && App->input->GetKey(SDL_SCANCODE_DELETE) == KEY_DOWN)
				{
					if (std::strcmp(child->GetName(), "Canvas") == 0)
						App->GOs->DeleteCanvasPointer();

					App->scene->selectedObject = CurrentSelection::SelectedType::null;
					App->GOs->DeleteGameObject(child);
				}

				if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(0) && (ImGui::GetMousePos().x - ImGui::GetItemRectMin().x) > ImGui::GetTreeNodeToLabelSpacing())
					SELECT(child);

				if (treeNodeOpened)
				{
					IterateAllChildren(child);
					ImGui::TreePop();
				}
			}
			else
			{
				treeNodeFlags = 0;
				treeNodeFlags |= ImGuiTreeNodeFlags_Leaf;

				if (App->scene->selectedObject == child)
					treeNodeFlags |= ImGuiTreeNodeFlags_Selected;

				if (root->openHierarchy)
				{
					ImGui::SetNextTreeNodeOpen(true);
					root->openHierarchy = false;
				}

				ImGui::TreeNodeEx(name, treeNodeFlags);
				ImGui::TreePop();

				SetGameObjectDragAndDrop(child);
				AtGameObjectPopUp(child);

				if (App->scene->selectedObject == child
					&& !App->gui->WantTextInput() && App->input->GetKey(SDL_SCANCODE_DELETE) == KEY_DOWN)
				{
					App->scene->selectedObject = CurrentSelection::SelectedType::null;
					App->GOs->DeleteGameObject(child);
				}

				if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(0) && (ImGui::GetMousePos().x - ImGui::GetItemRectMin().x) > ImGui::GetTreeNodeToLabelSpacing())
					SELECT(child);
			}
		}
	}
}

void PanelHierarchy::AtGameObjectPopUp(GameObject* child) const
{
	if (ImGui::BeginPopupContextItem())
	{
		if (child->GetLayer() == UILAYER)
		{
			if (ImGui::Selectable("Create Empty"))
			{
				GameObject* go = nullptr;
				if (child->cmp_rectTransform->GetFrom() != ComponentRectTransform::RectFrom::RECT)
				{
					go = App->GOs->CreateGameObject("ChildWorldCanvas", child, true);
					go->AddComponent(go->cmp_rectTransform = new ComponentRectTransform(go, ComponentTypes::RectTransformComponent, ComponentRectTransform::RectFrom::RECT_WORLD));
				}
				else
				{
					go = App->GOs->CreateGameObject("ChildScreenCanvas", child, true);
					go->AddComponent(ComponentTypes::RectTransformComponent);
				}
				go->SetLayer(UILAYER);
				ImGui::CloseCurrentPopup();
			}
			if (ImGui::Selectable("Delete"))
			{
				if (child->EqualsToChildrenOrThis(App->scene->selectedObject.Get()))
					App->scene->selectedObject = CurrentSelection::SelectedType::null;
				if (child->GetParent()->GetParent() == nullptr)
					App->GOs->DeleteCanvasPointer();
				App->GOs->DeleteGameObject(child);
				ImGui::CloseCurrentPopup();
			}
		}
		else
		{
			if (child->cmp_rectTransform == nullptr)
			{
				if (ImGui::Selectable("Create Empty"))
				{
					App->GOs->CreateGameObject("GameObject", child);
					ImGui::CloseCurrentPopup();
				}
				if (ImGui::Selectable("Create Cube"))
				{
					GameObject* go = App->GOs->CreateGameObject("Cube", child);
					go->AddComponent(ComponentTypes::MeshComponent);
					go->cmp_mesh->SetResource(App->resHandler->cube);
					ImGui::CloseCurrentPopup();
				}
				if (ImGui::Selectable("Create World Canvas"))
				{
					GameObject* go = App->GOs->CreateGameObject("WorldCanvas", child);
					ComponentRectTransform* new_rect = new ComponentRectTransform(go, ComponentTypes::RectTransformComponent, ComponentRectTransform::RectFrom::WORLD);
					go->AddComponent(go->cmp_rectTransform = new_rect);
					SELECT(go);
				}
			}
			else
			{
				if (ImGui::Selectable("Create Empty"))
				{
					GameObject* go = App->GOs->CreateGameObject("ChildWorldCanvas", child, true);
					ComponentRectTransform* new_rect = new ComponentRectTransform(go, ComponentTypes::RectTransformComponent, ComponentRectTransform::RectFrom::RECT_WORLD);
					go->AddComponent(go->cmp_rectTransform = new_rect);
					go->SetLayer(UILAYER);
					SELECT(go);
				}
			}
			if (ImGui::Selectable("Delete"))
			{
				if (child->EqualsToChildrenOrThis(App->scene->selectedObject.Get()))
					App->scene->selectedObject = CurrentSelection::SelectedType::null;
				App->GOs->DeleteGameObject(child);
				ImGui::CloseCurrentPopup();
			}
		}

		if (ImGui::Selectable("Duplicate at same parent"))
		{
			App->GOs->Instanciate(child, child->GetParent());
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void PanelHierarchy::SetGameObjectDragAndDrop(GameObject* sourceTarget) const
{
	SetGameObjectDragAndDropSource(sourceTarget);
	SetGameObjectDragAndDropTarget(sourceTarget);
}

void PanelHierarchy::SetGameObjectDragAndDropSource(GameObject* source) const
{
	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
	{
		ImGui::SetDragDropPayload("GAMEOBJECTS_HIERARCHY", &source, sizeof(GameObject));
		ImGui::EndDragDropSource();
	}
}

void PanelHierarchy::SetGameObjectDragAndDropTarget(GameObject* target) const
{
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GAMEOBJECTS_HIERARCHY"))
		{
			GameObject* payload_n = *(GameObject**)payload->Data;

			if (!payload_n->IsChild(target, true))
			{
				//Sorry for this code :(, this is UI checks for no crash the engine. pukecode
				bool rectWorldToWortldCanvas = false;
				if (payload_n->cmp_rectTransform && target->cmp_rectTransform)
					if (payload_n->cmp_rectTransform->GetFrom() == ComponentRectTransform::RectFrom::RECT_WORLD
						&& target->cmp_rectTransform->GetFrom() == ComponentRectTransform::RectFrom::WORLD)
						rectWorldToWortldCanvas = true;
				if (payload_n->GetLayer() == target->GetLayer() || rectWorldToWortldCanvas)
				{
					if (!rectWorldToWortldCanvas)
					{
						if (App->GOs->GetCanvas() == payload_n)
						{
							ImGui::EndDragDropTarget();
							return;
						}
						if (payload_n->cmp_rectTransform && target->cmp_rectTransform)
						{
							if ((payload_n->cmp_rectTransform->GetFrom() == ComponentRectTransform::RectFrom::WORLD
								&& target->cmp_rectTransform->GetFrom() == ComponentRectTransform::RectFrom::WORLD)
								|| (payload_n->cmp_rectTransform->GetFrom() != target->cmp_rectTransform->GetFrom()))
							{
								ImGui::EndDragDropTarget();
								return;
							}
						}
					}

					math::float4x4 globalMatrix;
					if (payload_n->GetLayer() != UILAYER)
						globalMatrix = payload_n->transform->GetGlobalMatrix();
					payload_n->GetParent()->EraseChild(payload_n);

					target->AddChild(payload_n);
					payload_n->SetParent(target);

					if (payload_n->GetLayer() != UILAYER)
						payload_n->transform->SetMatrixFromGlobal(globalMatrix);
					else if (payload_n->cmp_rectTransform)
					{
						ComponentRectTransform* rect = (ComponentRectTransform*)payload_n->GetComponent(ComponentTypes::RectTransformComponent);
						rect->CheckParentRect();
						rect->ChangeChildsRect();
					}
				}
			}
			else
				CONSOLE_LOG(LogTypes::Error, "ERROR: Invalid Target. Don't be so badass ;)");
		}
		ImGui::EndDragDropTarget();
	}
}

#endif // GAME
