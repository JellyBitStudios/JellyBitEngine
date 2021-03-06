#include "PanelInspector.h"

#ifndef GAMEMODE

#include "Globals.h"
#include "ModuleLayers.h"

#include "Application.h"
#include "ModuleScene.h"
#include "ModuleResourceManager.h"
#include "ModuleInternalResHandler.h"
#include "ModuleGui.h"
#include "ModuleGOs.h"
#include "PanelShaderEditor.h"
#include "PanelCodeEditor.h"
#include "SceneImporter.h"
#include "MaterialImporter.h"
#include "ShaderImporter.h"
#include "ScriptingModule.h"
#include "ModuleFileSystem.h"

#include "GameObject.h"
#include "Component.h"
#include "ComponentScript.h"
#include "ComponentRigidActor.h"
#include "ComponentCollider.h"
#include "ComponentRectTransform.h"
#include "ComponentBone.h"

#include "Resource.h"
#include "ResourceMesh.h"
#include "ResourceTexture.h"
#include "ResourceShaderObject.h"
#include "ResourceShaderProgram.h"
#include "ResourceScript.h"
#include "ResourceMaterial.h"
#include "ResourceAvatar.h"
#include "ResourceAnimation.h"
#include "ResourceBone.h"

#include "imgui\imgui.h"
#include "imgui\imgui_internal.h"
#include "imgui\imgui_stl.h"

PanelInspector::PanelInspector(const char* name) : Panel(name) {}

PanelInspector::~PanelInspector() {}

bool PanelInspector::Draw()
{
	ImGuiWindowFlags inspectorFlags = 0;
	inspectorFlags |= ImGuiWindowFlags_NoFocusOnAppearing;

	if (ImGui::Begin(name, &enabled, inspectorFlags))
	{
		switch (App->scene->selectedObject.GetType())
		{
			case CurrentSelection::SelectedType::gameObject:
				ShowGameObjectInspector();
				break;
			case CurrentSelection::SelectedType::resource:
			{
				switch (((Resource*)App->scene->selectedObject.Get())->GetType())
				{
					case ResourceTypes::MeshResource:
						ShowMeshResourceInspector();
						break;
					case ResourceTypes::TextureResource:
						ShowTextureResourceInspector();
						ShowTextureImportSettingsInspector();
						break;
					case ResourceTypes::ShaderObjectResource:
						ShowShaderObjectInspector();
						break;
					case ResourceTypes::ShaderProgramResource:
						ShowShaderProgramInspector();
						break;
					case ResourceTypes::MaterialResource:
						ShowMaterialInspector();
						break;
					case ResourceTypes::AvatarResource:
						ShowAvatarInspector();
						break;
					case ResourceTypes::AnimationResource:
						ShowAnimationInspector();
						break;
				}
			break;
			}
		case CurrentSelection::SelectedType::meshImportSettings:
			ShowMeshImportSettingsInspector();
			break;
		case CurrentSelection::SelectedType::fontImportSettings:
			ShowFontImportSettingsInspector();
			break;
		}
	}
	ImGui::End();

	return true;
}

void PanelInspector::ShowGameObjectInspector() const
{
	GameObject* gameObject = App->scene->selectedObject.GetCurrGameObject();

	if (gameObject == nullptr)
	{
		assert(gameObject != nullptr);
		return;
	}

	bool isActive = gameObject->IsActive();
	if (ImGui::Checkbox("##Active", &isActive))
	{
		for (std::list<uint>::iterator iter = App->scene->multipleSelection.begin(); iter != App->scene->multipleSelection.end(); ++iter)
		{
			GameObject* go = App->GOs->GetGameObjectByUID(*iter);
			if (go)
				go->SetIsActive(isActive);
		}
	}
	ImGui::SameLine();

	static char objName[INPUT_BUF_SIZE];
	if (gameObject->GetName() != nullptr)
		strcpy_s(objName, IM_ARRAYSIZE(objName), gameObject->GetName());

	ImGui::PushItemWidth(100.0f);
	ImGuiInputTextFlags inputFlag = ImGuiInputTextFlags_EnterReturnsTrue;
	if (ImGui::InputText("##objName", objName, IM_ARRAYSIZE(objName), inputFlag))
		for (std::list<uint>::const_iterator iter = App->scene->multipleSelection.begin(); iter != App->scene->multipleSelection.end(); ++iter)
		{
			GameObject* go = App->GOs->GetGameObjectByUID(*iter);
			if (go)
				go->SetName(objName);
		}
	ImGui::PopItemWidth();


	bool isStatic = gameObject->IsStatic();
	if (ImGui::Checkbox("Static", &isStatic))
	{
		for (std::list<uint>::iterator iter = App->scene->multipleSelection.begin(); iter != App->scene->multipleSelection.end(); ++iter)
		{
			GameObject* go = App->GOs->GetGameObjectByUID(*iter);
			if (go)
				go->SetIsStatic(isStatic);
		}
	}


	ImGui::SameLine();
	if (ImGui::Button("Everybody Static"))
	{
		for (std::list<uint>::iterator iter = App->scene->multipleSelection.begin(); iter != App->scene->multipleSelection.end(); ++iter)
		{
			GameObject* go = App->GOs->GetGameObjectByUID(*iter);
			if (go)
				go->ToggleChildrenAndThisStatic(!isStatic);
		}
	}

	// Layer
	std::vector<const char*> layers;
	int currentLayer = 0;
	for (uint i = 0; i < MAX_NUM_LAYERS; ++i)
	{
		const char* layerName = App->layers->NumberToName(i);
		if (strcmp(layerName, "") == 0)
			continue;

		layers.push_back(layerName);
		if (i == gameObject->GetLayer())
			currentLayer = layers.size() - 1;
	}
	layers.shrink_to_fit();

	ImGui::PushItemWidth(150.0f);
	if (ImGui::Combo("Layer", &currentLayer, &layers[0], layers.size()))
	{
		for (std::list<uint>::const_iterator iter = App->scene->multipleSelection.begin(); iter != App->scene->multipleSelection.end(); ++iter)
		{
			GameObject* go = App->GOs->GetGameObjectByUID(*iter);
			if (go)
				go->SetLayer(App->layers->NameToNumber(layers[currentLayer]));
		}
	}
	ImGui::PopItemWidth();


	if (ImGui::Button("Layerify children"))
		gameObject->ApplyLayerChildren(gameObject->GetLayer());

	// -----

	ImVec2 inspectorSize = ImGui::GetWindowSize();
	ImVec2 cursorPos = ImGui::GetCursorScreenPos();

	ImGui::SetCursorScreenPos(ImGui::GetWindowPos());

	ImGui::SetCursorScreenPos(cursorPos);

	ImGui::Separator();
	if (App->scene->multipleSelection.size() == 1)
	{
		for (int i = 0; i < gameObject->GetComponentsLength(); ++i)
		{
			ImGui::Separator();
			DragnDropSeparatorTarget(gameObject->GetComponent(i));
			gameObject->GetComponent(i)->OnEditor();
		}
	}
	else
	{

		DragnDropSeparatorTarget(gameObject->GetComponent(TransformComponent));
		gameObject->GetComponent(TransformComponent)->OnEditor();
	}

	ImGui::Separator();
	DragnDropSeparatorTarget(gameObject->GetComponent(gameObject->GetComponentsLength() - 1));

	ImGui::Button("Add Component");
	bool scriptSelected = false;
	if (ImGui::BeginPopupContextItem((const char*)0, 0))
	{
		bool uiLayer = true;
		for (std::list<uint>::const_iterator iter = App->scene->multipleSelection.begin(); iter != App->scene->multipleSelection.end(); ++iter)
		{
			GameObject* go = App->GOs->GetGameObjectByUID(*iter);
			if (go && go->GetLayer() != UILAYER)
			{
				uiLayer = false;
				break;
			}
		}
		if (uiLayer)
		{
			if (CheckIsComponent(ImageComponent))
				if (ImGui::Selectable("Image UI")) {
					AddComponentInGroup(ComponentTypes::ImageComponent);
					ImGui::CloseCurrentPopup();
				}
			if (CheckIsComponent(ButtonComponent))
				if (ImGui::Selectable("Button UI")) {
					AddComponentInGroup(ComponentTypes::ButtonComponent);
					ImGui::CloseCurrentPopup();
				}
			if (CheckIsComponent(LabelComponent))
				if (ImGui::Selectable("Label UI")) {
					AddComponentInGroup(ComponentTypes::LabelComponent);
					ImGui::CloseCurrentPopup();
				}
			if (CheckIsComponent(SliderComponent))
				if (ImGui::Selectable("Slider UI")) {
					AddComponentInGroup(ComponentTypes::SliderComponent);
					ImGui::CloseCurrentPopup();
				}
			if (CheckIsComponent(UIAnimationComponent))
				if (ImGui::Selectable("Animation UI")) {
					AddComponentInGroup(ComponentTypes::UIAnimationComponent);
					ImGui::CloseCurrentPopup();
				}
		}
		if (ImGui::Selectable("Script")) {
			//Open new Popup, with input text and autocompletion to select scripts by name
			scriptSelected = true;
			ImGui::CloseCurrentPopup();
		}

		if (gameObject->transform)
		{
			if (CheckIsComponent(MeshComponent))
				if (ImGui::Selectable("Mesh")) {
					AddComponentInGroup(ComponentTypes::MeshComponent);
					ImGui::CloseCurrentPopup();
				}
			if (CheckIsComponent(CameraComponent))
				if (ImGui::Selectable("Camera")) {
					AddComponentInGroup(ComponentTypes::CameraComponent);
					ImGui::CloseCurrentPopup();
				}
			if (CheckIsComponent(EmitterComponent))
				if (ImGui::Selectable("Particle Emitter")) {
					AddComponentInGroup(ComponentTypes::EmitterComponent);
					ImGui::CloseCurrentPopup();
				}
			if (CheckIsComponent(LightComponent))
				if (ImGui::Selectable("Light")) {
					AddComponentInGroup(ComponentTypes::LightComponent);
					ImGui::CloseCurrentPopup();
				}
			if (CheckIsComponent(ProjectorComponent))
				if (ImGui::Selectable("Projector")) {
					AddComponentInGroup(ComponentTypes::ProjectorComponent);
					ImGui::CloseCurrentPopup();
				}
			if (CheckIsComponent(AnimatorComponent))
				if (ImGui::Selectable("Animator")) {
					AddComponentInGroup(ComponentTypes::AnimatorComponent);
					ImGui::CloseCurrentPopup();
				}

			if (CheckIsComponent(NavAgentComponent))
				if (ImGui::Selectable("Nav Agent")) {
					AddComponentInGroup(ComponentTypes::NavAgentComponent);
					ImGui::CloseCurrentPopup();
				}


			if (CheckIsComponent(RigidStaticComponent) && CheckIsComponent(RigidDynamicComponent)) {
				if (ImGui::Selectable("Rigid Static")) {
					AddComponentInGroup(ComponentTypes::RigidStaticComponent);
					ImGui::CloseCurrentPopup();
				}
				else if ((CheckIsComponentCollider() || CheckIsComponent(PlaneColliderComponent))
					&& ImGui::Selectable("Rigid Dynamic")) {
					AddComponentInGroup(ComponentTypes::RigidDynamicComponent);
					ImGui::CloseCurrentPopup();
				}
			}
			if (CheckIsComponentCollider()) {
				if (ImGui::Selectable("Box Collider")) {
					AddComponentInGroup(ComponentTypes::BoxColliderComponent);
					ImGui::CloseCurrentPopup();
				}
				else if (ImGui::Selectable("Sphere Collider")) {
					AddComponentInGroup(ComponentTypes::SphereColliderComponent);
					ImGui::CloseCurrentPopup();
				}
				else if (ImGui::Selectable("Capsule Collider")) {
					AddComponentInGroup(ComponentTypes::CapsuleColliderComponent);
					ImGui::CloseCurrentPopup();
				}
				else if (((CheckIsComponent(RigidStaticComponent) && CheckIsComponent(RigidDynamicComponent)) || CheckIsComponent(RigidStaticComponent, true))
					&& ImGui::Selectable("Plane Collider")) {
					AddComponentInGroup(ComponentTypes::PlaneColliderComponent);
					ImGui::CloseCurrentPopup();
				}
			}
			if (CheckIsComponent(AudioListenerComponent))
				if (ImGui::Selectable("Audio Listener")) {
					AddComponentInGroup(ComponentTypes::AudioListenerComponent);
					ImGui::CloseCurrentPopup();
				}

			if (CheckIsComponent(AudioSourceComponent))
				if (ImGui::Selectable("Audio Source")) {
					AddComponentInGroup(ComponentTypes::AudioSourceComponent);
					ImGui::CloseCurrentPopup();
				}

			if (CheckIsComponent(TrailComponent))
			{
				if (ImGui::Selectable("Trail")) {
					AddComponentInGroup(ComponentTypes::TrailComponent);
					ImGui::CloseCurrentPopup();
				}
			}

			if (CheckIsComponent(InterpolationComponent))
			{
				if (ImGui::Selectable("Interpolation")) {
					AddComponentInGroup(ComponentTypes::InterpolationComponent);
					ImGui::CloseCurrentPopup();
				}
			}
		}
		ImGui::EndPopup();
	}

	if (scriptSelected)
	{
		ImGui::OpenPopup("AddingScript");
	}

	inspectorSize = ImGui::GetWindowSize();
	ImGui::SetNextWindowPos({ ImGui::GetWindowPos().x, ImGui::GetCursorScreenPos().y });
	ImGui::SetNextWindowSize({ inspectorSize.x, 0.0f });
	if (ImGui::BeginPopup("AddingScript"))
	{
		std::vector<std::string> scriptNames = ResourceScript::getScriptNames();

		float totalHeight = 0;

		float windowPaddingY = ImGui::GetCurrentWindow()->WindowPadding.y - 2;

		for (int i = 0; i < scriptNames.size(); ++i)
		{
			totalHeight += ImGui::CalcTextSize(scriptNames[i].data()).y + 4;
		}

		ImGui::SetNextWindowContentSize({ 0, totalHeight });

		//TODO: Add a maximum height, fix the totalHeight calculation

		totalHeight = totalHeight > 300 ? 300 : totalHeight;

		ImGui::BeginChild("Names Available", { inspectorSize.x - 15, totalHeight + windowPaddingY * 2 }, true);

		for (int i = 0; i < scriptNames.size(); ++i)
		{
			if (ImGui::Selectable(scriptNames[i].data()))
			{
				ResourceScript* res = nullptr;

				std::vector<Resource*> scriptResources = App->res->GetResourcesByType(ResourceTypes::ScriptResource);
				for (Resource* resource : scriptResources)
				{
					if (resource->GetData().name == scriptNames[i])
					{
						res = (ResourceScript*)resource;
					}
				}

				CONSOLE_LOG(LogTypes::Normal, "New Script Created: %s", scriptNames[i].data());
				for (std::list<uint>::const_iterator iter = App->scene->multipleSelection.begin(); iter != App->scene->multipleSelection.end(); ++iter)
				{
					ComponentScript* script = App->scripting->CreateScriptComponent(scriptNames[i], res);
					GameObject* go = App->GOs->GetGameObjectByUID(*iter);
					if (go)
					{
						go->AddComponent(script);
						script->SetParent(go);
						script->InstanceClass();
					}
				}

				ImGui::CloseCurrentPopup();
			}
		}

		ImGui::EndChild();

		static std::string scriptName;

		ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_FrameBg, { 0.26f, 0.59f, 0.98f, 0.5f });

		ImGui::PushItemWidth(inspectorSize.x - ImGui::CalcTextSize("Script Name").x - 30);
		if (ImGui::InputText("Script Name", &scriptName, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CharsNoBlank))
		{
			App->scripting->clearSpaces(scriptName);

			//Find the ResourceScript with this name, extracting the UUID from the .meta

			ResourceScript* res = nullptr;

			if (App->fs->Exists("Assets/Scripts/" + scriptName + ".cs.meta"))
			{
				char* metaBuffer;
				uint size = App->fs->Load("Assets/Scripts/" + scriptName + ".cs.meta", &metaBuffer);
				if (size > 0)
				{
					uint32_t UUID;
					memcpy(&UUID, metaBuffer, sizeof(uint32_t));

					res = (ResourceScript*)App->res->GetResource(UUID);

					delete[] metaBuffer;
				}
			}

			CONSOLE_LOG(LogTypes::Normal, "New Script Created: %s", scriptName.data());
			for (std::list<uint>::const_iterator iter = App->scene->multipleSelection.begin(); iter != App->scene->multipleSelection.end(); ++iter)
			{
				ComponentScript* script = App->scripting->CreateScriptComponent(scriptName, res);
				GameObject* go = App->GOs->GetGameObjectByUID(*iter);
				if (go)
				{
					go->AddComponent(script);
					script->SetParent(go);
					script->InstanceClass();
				}
			}
			scriptName = "";
			ImGui::CloseCurrentPopup();
		}
		ImGui::PopItemWidth();

		ImGui::PopStyleColor();
		ImGui::EndPopup();
	}
}

bool PanelInspector::CheckIsComponent(ComponentTypes type, bool allHaveIt) const
{
	bool canContinue = true;
	for (std::list<uint>::const_iterator iter = App->scene->multipleSelection.begin(); iter != App->scene->multipleSelection.end(); ++iter)
	{
		GameObject* go = App->GOs->GetGameObjectByUID(*iter);
		if (go)
		{
			if (!allHaveIt)
			{
				if (go->GetComponent(type))
				{
					canContinue = false;
					break;
				}
			}
			else if (go->GetComponent(type) == nullptr)
			{
				canContinue = false;
				break;
			}
		}
	}
	return canContinue;
}

bool PanelInspector::CheckIsComponentCollider() const
{
	bool canContinue = true;
	for (std::list<uint>::const_iterator iter = App->scene->multipleSelection.begin(); iter != App->scene->multipleSelection.end(); ++iter)
	{
		GameObject* go = App->GOs->GetGameObjectByUID(*iter);
		if (go && go->cmp_collider)
		{
			canContinue = false;
			break;
		}
	}
	return canContinue;
}
void PanelInspector::AddComponentInGroup(ComponentTypes type) const
{
	for (std::list<uint>::const_iterator iter = App->scene->multipleSelection.begin(); iter != App->scene->multipleSelection.end(); ++iter)
	{
		GameObject* go = App->GOs->GetGameObjectByUID(*iter);
		if (go)
			go->AddComponent(type);
	}
}

void PanelInspector::DragnDropSeparatorTarget(Component* target) const
{
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("COMPONENTS_INSPECTOR"))
		{
			Component* payload_n = *(Component**)payload->Data;
			target->GetParent()->ReorderComponents(payload_n, target);
		}
		ImGui::EndDragDropTarget();
	}
}

void PanelInspector::ShowMeshResourceInspector() const
{
	ImGui::Text("Mesh Resource");
	ImGui::Separator();
	ImGui::Spacing();

	const ResourceMesh* resourceMesh = (const ResourceMesh*)App->scene->selectedObject.Get();
	ImGui::Text("File:"); ImGui::SameLine();
	ImGui::TextColored(BLUE, "%s", resourceMesh->GetFile());
	ImGui::Text("Exported file:"); ImGui::SameLine();
	ImGui::TextColored(BLUE, "%s", resourceMesh->GetExportedFile());
	ImGui::Text("UUID:"); ImGui::SameLine();
	ImGui::TextColored(BLUE, "%u", resourceMesh->GetUuid());

	bool inMemory = resourceMesh->IsInMemory();
	ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
	ImGui::Checkbox("In memory", &inMemory);
	ImGui::PushItemFlag(ImGuiItemFlags_Disabled, false);
	if (inMemory)
	{
		ImGui::Text("References:"); ImGui::SameLine();
		ImGui::TextColored(BLUE, "%u", resourceMesh->GetReferencesCount());
	}

	ImGui::Spacing();

	ImGui::Text("VBO ID:"); ImGui::SameLine();
	ImGui::TextColored(BLUE, "%u", resourceMesh->GetVBO());
	ImGui::Text(""); ImGui::SameLine(); ImGui::Text("Vertices:"); ImGui::SameLine();
	float nVerts = resourceMesh->GetVerticesCount();
	ImGui::TextColored(BLUE, "%f", nVerts);
	ImGui::Text(""); ImGui::SameLine(); ImGui::Text("Normals:"); ImGui::SameLine();
	ImGui::TextColored(BLUE, "Not available yet");
	ImGui::Text(""); ImGui::SameLine(); ImGui::Text("Colors:"); ImGui::SameLine();
	ImGui::TextColored(BLUE, "Not available yet");
	ImGui::Text(""); ImGui::SameLine(); ImGui::Text("Texture Coordinates:"); ImGui::SameLine();
	ImGui::TextColored(BLUE, "Not available yet");

	ImGui::Spacing();

	ImGui::Text("VAO ID:"); ImGui::SameLine();
	ImGui::TextColored(BLUE, "%u", resourceMesh->GetVBO());

	ImGui::Spacing();

	ImGui::Text("IBO ID:"); ImGui::SameLine();
	ImGui::TextColored(BLUE, "%u", resourceMesh->GetIBO());
	float nIndices = resourceMesh->GetIndicesCount();
	ImGui::Text(""); ImGui::SameLine(); ImGui::Text("Indices:"); ImGui::SameLine();
	ImGui::TextColored(BLUE, "%f", nIndices);

	ImGui::Spacing();

	ImGui::Text("Triangles:"); ImGui::SameLine();
	ImGui::TextColored(BLUE, "%f", nIndices / 3);
}

void PanelInspector::ShowTextureResourceInspector() const
{
	ImGui::Text("Texture Resource");
	ImGui::Separator();
	ImGui::Spacing();

	const ResourceTexture* resourceTexture = (const ResourceTexture*)App->scene->selectedObject.Get();
	ImGui::Text("File:"); ImGui::SameLine();
	ImGui::TextColored(BLUE, "%s", resourceTexture->GetFile());
	ImGui::Text("Exported file:"); ImGui::SameLine();
	ImGui::TextColored(BLUE, "%s", resourceTexture->GetExportedFile());
	ImGui::Text("UUID:"); ImGui::SameLine();
	ImGui::TextColored(BLUE, "%u", resourceTexture->GetUuid());

	ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
	bool inMemory = resourceTexture->IsInMemory();
	ImGui::Checkbox("In memory", &inMemory);
	ImGui::PushItemFlag(ImGuiItemFlags_Disabled, false);
	if (inMemory)
	{
		ImGui::Text("References:"); ImGui::SameLine();
		ImGui::TextColored(BLUE, "%u", resourceTexture->GetReferencesCount());
	}

	ImGui::Spacing();
	uint id = resourceTexture->GetId();
	ImGui::Text("ID:"); ImGui::SameLine();
	ImGui::TextColored(BLUE, "%u", id);
	ImGui::Image((void*)(intptr_t)id, ImVec2(128, 128), ImVec2(0, 1), ImVec2(1, 0));
	ImGui::TextColored(BLUE, "%u x %u", resourceTexture->GetWidth(), resourceTexture->GetHeight());
}

void PanelInspector::ShowMeshImportSettingsInspector()
{
	ImGui::Text("Mesh Import Settings");
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::Spacing();
	ImGui::Text("Import Settings");
	ImGui::Spacing();

	ImGui::Text("Scale"); ImGui::PushItemWidth(50.0f);
	ImGui::DragFloat("##Scale", &m_is.scale, 0.01f, 0.0f, FLT_MAX, "%.2f", 1.0f);
	ImGui::Checkbox("Adjacency", &m_is.adjacency);

	ImGui::Spacing();

	// IBO ATR
	ImGui::Text("IBO Attributes");
	ImGui::CheckboxFlags("Positions", &(uint)m_is.attributes, ResourceMeshImportSettings::AttrConfiguration::ATTR_POSITION);
	ImGui::CheckboxFlags("Normals", &(uint)m_is.attributes, ResourceMeshImportSettings::AttrConfiguration::ATTR_NORMAL);
	ImGui::CheckboxFlags("Colors", &(uint)m_is.attributes, ResourceMeshImportSettings::AttrConfiguration::ATTR_COLOR);
	ImGui::CheckboxFlags("Texture coordinates", &(uint)m_is.attributes, ResourceMeshImportSettings::AttrConfiguration::ATTR_TEXCOORD);
	ImGui::CheckboxFlags("Tangents", &(uint)m_is.attributes, ResourceMeshImportSettings::AttrConfiguration::ATTR_TANGENT);
	ImGui::CheckboxFlags("Bitangents", &(uint)m_is.attributes, ResourceMeshImportSettings::AttrConfiguration::ATTR_BITANGENT);
	ImGui::CheckboxFlags("Animations", &(uint)m_is.attributes, ResourceMeshImportSettings::AttrConfiguration::ATTR_ANIMATION);

	const char* postProcessConfiguration[] = { "Target Realtime Fast", "Target Realtime Quality", "Target Realtime Max Quality", "Custom" };
	
	ImGui::PushItemWidth(100.0f);
	if (ImGui::Combo("Configuration", (int*)&m_is.postProcessConfigurationFlags, postProcessConfiguration, IM_ARRAYSIZE(postProcessConfiguration)))
	ImGui::PopItemWidth();
	
	if (m_is.postProcessConfigurationFlags == ResourceMeshImportSettings::PostProcessConfigurationFlags::CUSTOM)
	{
		ImGui::CheckboxFlags("Tangent space", &(uint)m_is.customConfigurationFlags, ResourceMeshImportSettings::CustomConfigurationFlags::CALC_TANGENT_SPACE);
		if (ImGui::CheckboxFlags("Normals", &(uint)m_is.customConfigurationFlags, ResourceMeshImportSettings::CustomConfigurationFlags::GEN_NORMALS))
		{
			if (m_is.customConfigurationFlags & ResourceMeshImportSettings::CustomConfigurationFlags::GEN_NORMALS)
				m_is.customConfigurationFlags &= ~ResourceMeshImportSettings::CustomConfigurationFlags::GEN_SMOOTH_NORMALS;
		}
		if (ImGui::CheckboxFlags("Smooth normals", &(uint)m_is.customConfigurationFlags, ResourceMeshImportSettings::CustomConfigurationFlags::GEN_SMOOTH_NORMALS))
		{
			if (m_is.customConfigurationFlags & ResourceMeshImportSettings::CustomConfigurationFlags::GEN_SMOOTH_NORMALS)
				m_is.customConfigurationFlags &= ~ResourceMeshImportSettings::CustomConfigurationFlags::GEN_NORMALS;
		}
		ImGui::CheckboxFlags("Join identical vertices", &(uint)m_is.customConfigurationFlags, ResourceMeshImportSettings::CustomConfigurationFlags::JOIN_IDENTICAL_VERTICES);
		ImGui::CheckboxFlags("Triangulate", &(uint)m_is.customConfigurationFlags, ResourceMeshImportSettings::CustomConfigurationFlags::TRIANGULATE);
		ImGui::CheckboxFlags("Sort by type", &(uint)m_is.customConfigurationFlags, ResourceMeshImportSettings::CustomConfigurationFlags::GEN_UV_COORDS);
		ImGui::CheckboxFlags("Improve cache locality", &(uint)m_is.customConfigurationFlags, ResourceMeshImportSettings::CustomConfigurationFlags::IMPROVE_CACHE_LOCALITY);
		ImGui::CheckboxFlags("Limit bone weights", &(uint)m_is.customConfigurationFlags, ResourceMeshImportSettings::CustomConfigurationFlags::LIMIT_BONE_WEIGHTS);
		ImGui::CheckboxFlags("Remove redundant materials", &(uint)m_is.customConfigurationFlags, ResourceMeshImportSettings::CustomConfigurationFlags::REMOVE_REDUNDANT_MATERIALS);
		ImGui::CheckboxFlags("Split large meshes", &(uint)m_is.customConfigurationFlags, ResourceMeshImportSettings::CustomConfigurationFlags::SPLIT_LARGE_MESHES);
		ImGui::CheckboxFlags("Find degenerates", &(uint)m_is.customConfigurationFlags, ResourceMeshImportSettings::CustomConfigurationFlags::FIND_DEGENERATES);
		ImGui::CheckboxFlags("Find invalid data", &(uint)m_is.customConfigurationFlags, ResourceMeshImportSettings::CustomConfigurationFlags::FIND_INVALID_DATA);
		ImGui::CheckboxFlags("Find instances", &(uint)m_is.customConfigurationFlags, ResourceMeshImportSettings::CustomConfigurationFlags::FIND_INSTANCES);
		ImGui::CheckboxFlags("Validate data structures", &(uint)m_is.customConfigurationFlags, ResourceMeshImportSettings::CustomConfigurationFlags::VALIDATE_DATA_STRUCTURE);
		ImGui::CheckboxFlags("Optimize Meshes", &(uint)m_is.customConfigurationFlags, ResourceMeshImportSettings::CustomConfigurationFlags::OPTIMIZE_MESHES);
	}
	ImGui::Spacing();
	if (ImGui::Button("REIMPORT"))
	{
		// Search for the meta associated to the file
		char metaFile[DEFAULT_BUF_SIZE];
		strcpy_s(metaFile, strlen(m_is.modelPath) + 1, m_is.modelPath); // file
		strcat_s(metaFile, strlen(metaFile) + strlen(EXTENSION_META) + 1, EXTENSION_META); // extension

		// Update meta
		ResourceMesh::SetMeshImportSettingsToMeta(metaFile, m_is);

		// Reimport Mesh file
		System_Event newEvent;
		newEvent.fileEvent.type = System_Event_Type::ReImportFile;
		strcpy(newEvent.fileEvent.file, m_is.modelPath);
		App->PushSystemEvent(newEvent);
	}
}

void PanelInspector::ShowFontImportSettingsInspector()
{
	ImGui::Text("Font Import Settings");
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::Spacing();
	ImGui::Text("Import Settings");
	ImGui::Spacing();

	int sizesSize = f_is.sizes.size();

	if (ImGui::InputInt("Sizes Amount", &sizesSize))
		f_is.sizes.resize(sizesSize);
	ImGui::Spacing();

	for (uint i = 0; i < sizesSize; ++i)
	{
		std::string name = "Size " + std::to_string(i);
		ImGui::InputInt(name.data(), (int*)&f_is.sizes[i]);
	}

	if (ImGui::Button("APPLY"))
	{
		ResourceFont::UpdateImportSettings(f_is);
	}
}

void PanelInspector::ShowTextureImportSettingsInspector() const
{
	ImGui::Text("Texture Import Settings");
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::Spacing();
	ImGui::Text("Import Settings");
	ImGui::Spacing();

	const char* compression[] = { "DXT1", "DXT3", "DXT5" };
	ImGui::PushItemWidth(100.0f);
	if (ImGui::Combo("Compression", (int*)&t_is.compression, compression, IM_ARRAYSIZE(compression)))

	ImGui::Spacing();
	ImGui::Text("Load Settings");
	ImGui::Spacing();

	ImGui::Text("Wrap Mode");
	const char* wrap[] = { "Repeat", "Mirrored Repeat", "Clamp To Edge", "Clamp To Border" };
	
	ImGui::Combo("Wrap S", (int*)&t_is.wrapS, wrap, IM_ARRAYSIZE(wrap));
	ImGui::Combo("Wrap T", (int*)&t_is.wrapT, wrap, IM_ARRAYSIZE(wrap));

	ImGui::Text("Filter Mode");
	const char* filter[] = { "Nearest", "Linear",
		"Nearest Mipmap Nearest", "Linear Mipmap Nearest", "Nearest Mipmap Linear", "Linear Mipmap Linear" };
	ImGui::Combo("Min Filter", (int*)&t_is.minFilter, filter, IM_ARRAYSIZE(filter));
	ImGui::Combo("Mag Filter", (int*)&t_is.magFilter, filter, IM_ARRAYSIZE(filter));
	ImGui::PopItemWidth();

	if (t_is.UseMipmap())
		ImGui::TextColored(BLUE, "Mip Maps will be generated");

	if (App->materialImporter->IsAnisotropySupported())
		ImGui::SliderFloat("Anisotropy", &(float)t_is.anisotropy, 0.0f, App->materialImporter->GetLargestSupportedAnisotropy());

	ImGui::Spacing();
	if (ImGui::Button("REIMPORT"))
	{
		ResourceTexture* res = (ResourceTexture*)App->scene->selectedObject.Get();
		
		// Search for the meta associated to the file
		char metaFile[DEFAULT_BUF_SIZE];
		strcpy_s(metaFile, strlen(res->GetFile()) + 1, res->GetFile()); // file
		strcat_s(metaFile, strlen(metaFile) + strlen(EXTENSION_META) + 1, EXTENSION_META); // extension

		// Update meta
		ResourceTexture::SetTextureImportSettingsToMeta(metaFile, t_is);

		// Reimport Mesh file
		System_Event newEvent;
		newEvent.fileEvent.type = System_Event_Type::ReImportFile;
		strcpy(newEvent.fileEvent.file, res->GetFile());
		App->PushSystemEvent(newEvent);
	}
}

void PanelInspector::ShowShaderObjectInspector() const
{
	ResourceShaderObject* shaderObject = (ResourceShaderObject*)App->scene->selectedObject.Get();

	switch (shaderObject->GetShaderObjectType())
	{
	case ShaderObjectTypes::VertexType:
		ImGui::Text("Vertex Shader Object");
		break;
	case ShaderObjectTypes::FragmentType:
		ImGui::Text("Fragment Shader Object");
		break;
	case ShaderObjectTypes::GeometryType:
		ImGui::Text("Geometry Shader Object");
		break;
	}
	ImGui::Separator();
	ImGui::Spacing();

	// Name
	ImGui::Text("Name:"); ImGui::SameLine();
	static char name[INPUT_BUF_SIZE];
	strcpy_s(name, INPUT_BUF_SIZE, shaderObject->GetName());
	ImGui::PushItemWidth(150.0f);
	ImGuiInputTextFlags inputFlag = ImGuiInputTextFlags_EnterReturnsTrue;
	if (ImGui::InputText("##name", name, INPUT_BUF_SIZE, inputFlag))
	{
		// Search for the meta associated to the file
		char metaFile[DEFAULT_BUF_SIZE];
		strcpy_s(metaFile, strlen(shaderObject->GetFile()) + 1, shaderObject->GetFile()); // file
		strcat_s(metaFile, strlen(metaFile) + strlen(EXTENSION_META) + 1, EXTENSION_META); // extension

		shaderObject->SetName(name);
		std::string shaderName = name;
		ResourceShaderObject::SetNameToMeta(metaFile, shaderName);
	}

	ImGui::Spacing();

	// Data
	ImGui::Text("File:"); ImGui::SameLine();
	ImGui::TextColored(BLUE, "%s", shaderObject->GetFile());
	ImGui::Text("UUID:"); ImGui::SameLine();
	ImGui::TextColored(BLUE, "%u", shaderObject->GetUuid());
	ImGui::Text("References:"); ImGui::SameLine();
	ImGui::TextColored(BLUE, "%u", shaderObject->GetReferencesCount());

	ImGui::Spacing();

	ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
	bool isValid = shaderObject->isValid;
	ImGui::Checkbox("Is valid", &isValid);
	ImGui::PushItemFlag(ImGuiItemFlags_Disabled, false);

	ImGui::Spacing();

	if (ImGui::Button("EDIT SHADER OBJECT"))
		App->gui->panelCodeEditor->OpenShaderInCodeEditor(shaderObject->GetUuid());
}

void PanelInspector::ShowShaderProgramInspector() const
{
	ImGui::Text("Shader Program");
	ImGui::Separator();
	ImGui::Spacing();

	ResourceShaderProgram* shaderProgram = (ResourceShaderProgram*)App->scene->selectedObject.Get();

	// Name
	ImGui::Text("Name:"); ImGui::SameLine();
	static char name[INPUT_BUF_SIZE];
	strcpy_s(name, INPUT_BUF_SIZE, shaderProgram->GetName());
	ImGui::PushItemWidth(150.0f);
	ImGuiInputTextFlags inputFlag = ImGuiInputTextFlags_EnterReturnsTrue;
	if (ImGui::InputText("##name", name, INPUT_BUF_SIZE, inputFlag))
	{
		// Search for the meta associated to the file
		char metaFile[DEFAULT_BUF_SIZE];
		strcpy_s(metaFile, strlen(shaderProgram->GetFile()) + 1, shaderProgram->GetFile()); // file
		strcat_s(metaFile, strlen(metaFile) + strlen(EXTENSION_META) + 1, EXTENSION_META); // extension

		shaderProgram->SetName(name);
		std::string shaderName = name;
		ResourceShaderProgram::SetNameToMeta(metaFile, shaderName);
	}

	ImGui::Spacing();

	// Data
	ImGui::Text("File:"); ImGui::SameLine();
	ImGui::TextColored(BLUE, "%s", shaderProgram->GetFile());
	ImGui::Text("UUID:"); ImGui::SameLine();
	ImGui::TextColored(BLUE, "%u", shaderProgram->GetUuid());

	ImGui::Spacing();

	ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
	bool isValid = shaderProgram->isValid;
	ImGui::Checkbox("Is valid", &isValid);
	ImGui::PushItemFlag(ImGuiItemFlags_Disabled, false);

	ImGui::Spacing();

	// Info
	ImGui::Text("Shader Objects:");
	char shaderObjectName[DEFAULT_BUF_SIZE];
	std::vector<uint> shaderObjects;
	shaderProgram->GetShaderObjects(shaderObjects);
	for (uint i = 0; i < shaderObjects.size(); ++i)
	{
		ResourceShaderObject* shaderObject = (ResourceShaderObject*)App->res->GetResource(shaderObjects[i]);
		ImGui::TextColored(BLUE, "%s", shaderObject->GetName()); ImGui::SameLine();
		sprintf_s(shaderObjectName, DEFAULT_BUF_SIZE, "EDIT##%i", i);

		if (ImGui::Button(shaderObjectName))
			App->gui->panelCodeEditor->OpenShaderInCodeEditor(shaderObjects[i]);
	}

	ImGui::Spacing();

	if (ImGui::Button("EDIT SHADER PROGRAM"))
		App->gui->panelShaderEditor->OpenShaderInShaderEditor(shaderProgram->GetUuid());
}

void PanelInspector::ShowAnimationInspector() const
{
	ImGui::Text("Animation");
	ImGui::Separator();
	ImGui::Spacing();

	ResourceAnimation* animation = (ResourceAnimation*)App->scene->selectedObject.Get();

	// Name
	ImGui::Text("Name:"); ImGui::SameLine(); ImGui::Text("%s", animation->animationData.name.data());
}

void PanelInspector::ShowMaterialInspector() const
{
	ImGui::Text("Material");
	ImGui::Separator();
	ImGui::Spacing();

	ResourceMaterial* material = (ResourceMaterial*)App->scene->selectedObject.Get();

	// Name
	ImGui::Text("Name:"); ImGui::SameLine();
	static char name[INPUT_BUF_SIZE];
	strcpy_s(name, INPUT_BUF_SIZE, material->GetName());
	ImGui::PushItemWidth(150.0f);
	ImGuiInputTextFlags inputFlag = ImGuiInputTextFlags_EnterReturnsTrue;
	if (ImGui::InputText("##name", name, INPUT_BUF_SIZE, inputFlag))
	{
		// Search for the meta associated to the file
		char metaFile[DEFAULT_BUF_SIZE];
		strcpy_s(metaFile, strlen(material->GetFile()) + 1, material->GetFile()); // file
		strcat_s(metaFile, strlen(metaFile) + strlen(EXTENSION_META) + 1, EXTENSION_META); // extension

		material->SetName(name);
		std::string materialName = name;
		ResourceMaterial::SetNameToMeta(metaFile, materialName);
	}

	ImGui::Spacing();

	// Data
	ImGui::Text("File:"); ImGui::SameLine();
	ImGui::TextColored(BLUE, "%s", material->GetFile());
	ImGui::Text("UUID:"); ImGui::SameLine();
	ImGui::TextColored(BLUE, "%u", material->GetUuid());
	ImGui::Text("References:"); ImGui::SameLine();
	ImGui::TextColored(BLUE, "%u", material->GetReferencesCount());
	ImGui::Spacing();

	char id[DEFAULT_BUF_SIZE];

	// Shader
	ResourceShaderProgram* shader = (ResourceShaderProgram*)App->res->GetResource(material->GetShaderUuid());
	assert(shader != nullptr);

	const char* shaderTypes[] = { "Standard", "Projector", "Effects", "UI", "Source", "Custom" };

	if (ImGui::Button("Shader"))
		ImGui::OpenPopup("shader_popup");
	if (ImGui::BeginPopup("shader_popup"))
	{
		std::vector<Resource*> shaderResources = App->res->GetResourcesByType(ResourceTypes::ShaderProgramResource);
		std::vector<Resource*> shaderResourcesByType;

		for (uint i = 0; i < IM_ARRAYSIZE(shaderTypes); ++i)
		{
			if (strcmp(shaderTypes[i], "Source") == 0)
				continue;

			for (uint j = 0; j < shaderResources.size(); ++j)
			{
				ResourceShaderProgram* shaderResource = (ResourceShaderProgram*)shaderResources[j];
				if (shaderResource->GetShaderProgramType() == i)
					shaderResourcesByType.push_back(shaderResource);
			}

			if (shaderResourcesByType.empty())
				continue;

			if (ImGui::BeginMenu(shaderTypes[i]))
			{
				for (uint j = 0; j < shaderResourcesByType.size(); ++j)
				{
					sprintf(id, "%s##%u", shaderResourcesByType[j]->GetName(), shaderResourcesByType[j]->GetUuid());
					bool selected = shader == shaderResourcesByType[j];
					if (ImGui::MenuItem(id, "", selected))
					{
						// Update the existing material
						material->SetResourceShader(shaderResourcesByType[j]->GetUuid());
						
						// Export the existing file
						std::string outputFile;
						App->res->ExportFile(ResourceTypes::MaterialResource, material->GetData(), &material->GetSpecificData(), outputFile, true, false);
					}
				}
				ImGui::EndMenu();
			}

			shaderResourcesByType.clear();
		}
		ImGui::EndPopup();
	}

	ImGui::SameLine(); ImGui::Text("%s", shader->GetName());

	if (ImGui::Button("EDIT"))
		App->gui->panelShaderEditor->OpenShaderInShaderEditor(shader->GetUuid());

	// Uniforms
	std::vector<Uniform>& uniforms = material->GetUniforms();
	if (!uniforms.empty())
	{
		std::vector<const char*> ignoreUniforms;
		if (shader->GetShaderProgramType() == ShaderProgramTypes::Custom)
		{
			// Standard ignore uniforms
			ignoreUniforms.push_back("animate");

			ignoreUniforms.push_back("fog.color");
			ignoreUniforms.push_back("fog.density");

			ignoreUniforms.push_back("gInfoTexture");

			ignoreUniforms.push_back("dot");
			ignoreUniforms.push_back("screenSize");

			ignoreUniforms.push_back("Time");
		}
		else
			material->GetIgnoreUniforms(ignoreUniforms);

		ImGui::Spacing();
		ImGui::Text("UNIFORMS");

		for (uint i = 0; i < uniforms.size(); ++i)
		{
			Uniform& uniform = uniforms[i];

			for (uint j = 0; j < ignoreUniforms.size(); ++j)
			{
				if (strcmp(uniform.common.name, ignoreUniforms[j]) == 0)
					goto hereWeGo;
			}

			ImGui::Text(uniform.common.name);
			ImGui::SameLine();

			bool exportFile = false;
			sprintf(id, "##uniform%u", i);
			ImGui::PushItemWidth(100.0f);
			switch (uniform.common.type)
			{
			case Uniforms_Values::FloatU_value:
				if (ImGui::InputFloat(id, &uniform.floatU.value))
					exportFile = true;
				break;
			case Uniforms_Values::IntU_value:
				if (ImGui::InputInt(id, (int*)&uniform.intU.value))
					exportFile = true;
				break;
			case Uniforms_Values::Vec2FU_value:
			{
				float v[] = { uniform.vec2FU.value.x, uniform.vec2FU.value.y };
				if (ImGui::InputFloat2(id, v))
				{
					uniform.vec2FU.value.x = v[0];
					uniform.vec2FU.value.y = v[1];

					exportFile = true;
				}
				break;
			}
			case Uniforms_Values::Vec3FU_value:
			{
				float v[] = { uniform.vec3FU.value.x, uniform.vec3FU.value.y , uniform.vec3FU.value.z };
				if (ImGui::InputFloat3(id, v))
				{
					uniform.vec3FU.value.x = v[0];
					uniform.vec3FU.value.y = v[1];
					uniform.vec3FU.value.z = v[2];

					exportFile = true;
				}
				break;
			}
			case Uniforms_Values::Vec4FU_value:
			{
				float v[] = { uniform.vec4FU.value.x, uniform.vec4FU.value.y , uniform.vec4FU.value.z, uniform.vec4FU.value.w };
				if (ImGui::InputFloat4(id, v))
				{
					uniform.vec4FU.value.x = v[0];
					uniform.vec4FU.value.y = v[1];
					uniform.vec4FU.value.z = v[2];
					uniform.vec4FU.value.w = v[3];

					exportFile = true;
				}
				break;
			}
			case Uniforms_Values::Vec2IU_value:
			{
				int v[] = { uniform.vec2IU.value.x, uniform.vec2IU.value.y };
				if (ImGui::InputInt2(id, v))
				{
					uniform.vec2IU.value.x = v[0];
					uniform.vec2IU.value.y = v[1];

					exportFile = true;
				}
				break;
			}
			case Uniforms_Values::Vec3IU_value:
			{
				int v[] = { uniform.vec3IU.value.x, uniform.vec3IU.value.y , uniform.vec3IU.value.z };
				if (ImGui::InputInt3(id, v))
				{
					uniform.vec3IU.value.x = v[0];
					uniform.vec3IU.value.y = v[1];
					uniform.vec3IU.value.z = v[2];

					exportFile = true;
				}
				break;
			}
			case Uniforms_Values::Vec4IU_value:
			{
				int v[] = { uniform.vec4IU.value.x, uniform.vec4IU.value.y , uniform.vec4IU.value.z, uniform.vec4IU.value.w };
				if (ImGui::InputInt4(id, v))
				{
					uniform.vec4IU.value.x = v[0];
					uniform.vec4IU.value.y = v[1];
					uniform.vec4IU.value.z = v[2];
					uniform.vec4IU.value.w = v[3];

					exportFile = true;
				}
				break;
			}
			case Uniforms_Values::Sampler2U_value:
			{
				ImGui::PushID("texture");
				ResourceTexture* texture = (ResourceTexture*)App->res->GetResource(uniform.sampler2DU.value.uuid);
				ImGui::Button(texture == nullptr ? "Empty texture" : texture->GetName(), ImVec2(150.0f, 0.0f));
				ImGui::PopID();

				if (ImGui::IsItemHovered())
				{
					ImGui::BeginTooltip();
					ImGui::Text("%u", uniform.sampler2DU.value.id);
					ImGui::EndTooltip();
				}

				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TEXTURE_INSPECTOR_SELECTOR"))
					{
						uint payload_n = *(uint*)payload->Data;

						// Update the existing material
						material->SetResourceTexture(payload_n, uniform.sampler2DU.value.uuid, uniform.sampler2DU.value.id);

						exportFile = true;
					}
					ImGui::EndDragDropTarget();
				}

				break;
			}
			}
			ImGui::PopItemWidth();

			if (exportFile)
			{
				// Export the existing file
				std::string outputFile;
				ResourceMaterial::ExportFile(material->GetData(), material->GetSpecificData(), outputFile, true);
			}

		hereWeGo:;
		}
	}
}

void PanelInspector::ShowAvatarInspector() const
{
	ImGui::Text("Avatar");
	ImGui::Separator();
	ImGui::Spacing();

	ResourceAvatar* avatar = (ResourceAvatar*)App->scene->selectedObject.Get();

	// Name
	ImGui::Text("Name:"); ImGui::SameLine();
	static char name[INPUT_BUF_SIZE];
	strcpy_s(name, INPUT_BUF_SIZE, avatar->GetName());
	ImGui::PushItemWidth(150.0f);
	ImGuiInputTextFlags inputFlag = ImGuiInputTextFlags_EnterReturnsTrue;
	if (ImGui::InputText("##name", name, INPUT_BUF_SIZE, inputFlag))
	{
		// Search for the meta associated to the file
		char metaFile[DEFAULT_BUF_SIZE];
		strcpy_s(metaFile, strlen(avatar->GetFile()) + 1, avatar->GetFile()); // file
		strcat_s(metaFile, strlen(metaFile) + strlen(EXTENSION_META) + 1, EXTENSION_META); // extension

		avatar->SetName(name);
		std::string avatarName = name;
		ResourceAvatar::SetNameToMeta(metaFile, avatarName);
	}

	ImGui::Spacing();

	// Data
	ImGui::Text("File:"); ImGui::SameLine();
	ImGui::TextColored(BLUE, "%s", avatar->GetFile());
	ImGui::Text("UUID:"); ImGui::SameLine();
	ImGui::TextColored(BLUE, "%u", avatar->GetUuid());
	ImGui::Text("References:"); ImGui::SameLine();
	ImGui::TextColored(BLUE, "%u", avatar->GetReferencesCount());

	ImGui::Spacing();

	bool exportFile = false;

	// Hips
	uint hipsUuid = avatar->GetHipsUuid();
	const GameObject* hipsGameObject = App->GOs->GetGameObjectByUID(hipsUuid);
	const ComponentBone* hipsComponent = hipsGameObject != nullptr ? hipsGameObject->cmp_bone : nullptr;
	const ResourceBone* hipsResource = hipsComponent != nullptr ? ((ResourceBone*)App->res->GetResource(hipsComponent->res)) : nullptr;

	ImGui::PushID("hips");
	ImGui::Button(hipsResource != nullptr ? hipsResource->boneData.name.data() : "Empty Bone", ImVec2(150.0f, 0.0f));
	ImGui::PopID();

	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::Text("%u", hipsUuid);
		ImGui::EndTooltip();
	}

	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GAMEOBJECTS_HIERARCHY"))
		{
			const GameObject* hipsGameObject = *(GameObject**)payload->Data;
			const ComponentBone* hipsComponent = hipsGameObject != nullptr ? hipsGameObject->cmp_bone : nullptr;
			const ResourceBone* hipsResource = hipsComponent != nullptr ? ((ResourceBone*)App->res->GetResource(hipsComponent->res)) : nullptr;
			
			if (hipsResource != nullptr)
			{
				avatar->SetHipsUuid(hipsGameObject->GetUUID());

				exportFile = true;
			}
			else
				CONSOLE_LOG(LogTypes::Warning, "This game object is not valid");
		}
		ImGui::EndDragDropTarget();
	}

	ImGui::SameLine();

	if (ImGui::SmallButton("REMOVE##hips"))
	{
		avatar->SetHipsUuid(0);

		exportFile = true;
	}

	// Root
	uint rootUuid = avatar->GetRootUuid();
	const GameObject* rootGameObject = App->GOs->GetGameObjectByUID(rootUuid);

	ImGui::PushID("root");
	ImGui::Button(rootGameObject != nullptr ? rootGameObject->GetName() : "Empty Root", ImVec2(150.0f, 0.0f));
	ImGui::PopID();

	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::Text("%u", rootUuid);
		ImGui::EndTooltip();
	}

	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GAMEOBJECTS_HIERARCHY"))
		{
			const GameObject* rootGameObject = *(GameObject**)payload->Data;
			
			if (rootGameObject != nullptr)
			{
				avatar->SetRootUuid(rootGameObject->GetUUID());

				exportFile = true;
			}
			else
				CONSOLE_LOG(LogTypes::Warning, "This game object is not valid");
		}
		ImGui::EndDragDropTarget();
	}

	ImGui::SameLine();

	if (ImGui::SmallButton("REMOVE##root"))
	{
		avatar->SetRootUuid(0);

		exportFile = true;
	}

	if (exportFile)
	{
		// Export the existing file
		std::string outputFile;
		ResourceAvatar::ExportFile(avatar->GetData(), avatar->GetSpecificData(), outputFile, true);
	}
}

#endif // GAME