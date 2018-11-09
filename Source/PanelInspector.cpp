#include "PanelInspector.h"

#ifndef GAMEMODE

#include "Globals.h"

#include "Application.h"
#include "ModuleScene.h"

#include "GameObject.h"
#include "Component.h"
#include "ComponentTransform.h"

#include "ImGui/imgui.h"
#include "imgui/imgui_internal.h"

PanelInspector::PanelInspector(char* name) : Panel(name) {}

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
		case CurrentSelection::SelectedType::meshImportSettings:
			ShowMeshImportSettingsInspector();
			break;
		case CurrentSelection::SelectedType::textureImportSettings:
			ShowTextureImportSettingsInspector();
			break;
		}
	}	
	ImGui::End();
	
	return true;
}

void PanelInspector::ShowGameObjectInspector()
{
	GameObject* gameObject = (GameObject*)App->scene->selectedObject.Get();

	bool isActive = gameObject->IsActive();
	if (ImGui::Checkbox("##Active", &isActive)) { gameObject->ToggleIsActive(); }

	ImGui::SameLine();
	static char objName[INPUT_BUF_SIZE];
	if (gameObject->GetName() != nullptr)
		strcpy_s(objName, IM_ARRAYSIZE(objName), gameObject->GetName());

	ImGui::PushItemWidth(100.0f);
	ImGuiInputTextFlags inputFlag = ImGuiInputTextFlags_EnterReturnsTrue;
	if (ImGui::InputText("##objName", objName, IM_ARRAYSIZE(objName), inputFlag))
		gameObject->SetName(objName);
	ImGui::PopItemWidth();

	ImGui::SameLine(0.0f, 30.f);
	bool isStatic = gameObject->IsStatic();
	if (ImGui::Checkbox("##static", &isStatic)) { gameObject->ToggleIsStatic(); }
	ImGui::SameLine();
	ImGui::Text("Static");

	ImGui::Text("Tag");
	ImGui::SameLine();
	const char* tags[] = { "Untagged", "Player" };
	static int currentTag = 0;
	ImGui::PushItemWidth(75.0f);
	ImGui::Combo("##tag", &currentTag, tags, IM_ARRAYSIZE(tags));
	ImGui::PopItemWidth();

	ImGui::SameLine();
	ImGui::Text("Layer");
	ImGui::SameLine();
	const char* layers[] = { "Default", "Collider", "PostProcessing" };
	static int currentLayer = 0;
	ImGui::PushItemWidth(75.0f);
	ImGui::Combo("##layer", &currentLayer, layers, IM_ARRAYSIZE(layers));
	ImGui::PopItemWidth();

	for (int i = 0; i < gameObject->GetComponenetsLength(); ++i)
	{
		ImGui::Separator();
		DragnDropSeparatorTarget(gameObject->GetComponent(i));
		gameObject->GetComponent(i)->OnEditor();
	}
	ImGui::Separator();
	DragnDropSeparatorTarget(gameObject->GetComponent(gameObject->GetComponenetsLength() - 1));

	ImGui::Button("Add Component");
	if (ImGui::BeginPopupContextItem((const char*)0, 0))
	{
		if (gameObject->meshRenderer == nullptr) {
			if (ImGui::Selectable("Mesh")) {
				gameObject->AddComponent(ComponentType::Mesh_Component);
				ImGui::CloseCurrentPopup();
			}
		}
		if (gameObject->materialRenderer == nullptr) {
			if (ImGui::Selectable("Material")) {
				gameObject->AddComponent(ComponentType::Material_Component);
				ImGui::CloseCurrentPopup();
			}
		}
		if (gameObject->camera == nullptr)
			if (ImGui::Selectable("Camera")) {
				gameObject->AddComponent(ComponentType::Camera_Component);
				ImGui::CloseCurrentPopup();
			}
		ImGui::EndPopup();
		
	}
}

void PanelInspector::DragnDropSeparatorTarget(Component* target)
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

void PanelInspector::ShowMeshImportSettingsInspector()
{
	MeshImportSettings* meshImportSettings = (MeshImportSettings*)App->scene->selectedObject.Get();

	ImGui::Text("Mesh Import Settings");
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::Spacing();
	ImGui::Text("Import Settings");
	ImGui::Spacing();

	ImGui::Text("Scale");

	const double f64_lo_a = -1000000000000000.0, f64_hi_a = +1000000000000000.0;

	ImGui::PushItemWidth(TRANSFORMINPUTSWIDTH);
	ImGui::DragScalar("##ScaleX", ImGuiDataType_Float, (void*)&meshImportSettings->scale.x, 0.1f, &f64_lo_a, &f64_hi_a, "%f", 1.0f); ImGui::SameLine();
	ImGui::PushItemWidth(TRANSFORMINPUTSWIDTH);
	ImGui::DragScalar("##ScaleY", ImGuiDataType_Float, (void*)&meshImportSettings->scale.y, 0.1f, &f64_lo_a, &f64_hi_a, "%f", 1.0f); ImGui::SameLine();
	ImGui::PushItemWidth(TRANSFORMINPUTSWIDTH);
	ImGui::DragScalar("##ScaleZ", ImGuiDataType_Float, (void*)&meshImportSettings->scale.z, 0.1f, &f64_lo_a, &f64_hi_a, "%f", 1.0f);

	ImGui::Checkbox("Use File Scale", &meshImportSettings->useFileScale);

	// TODO: if useFileScale, show the scale of the file

	const char* postProcessConfiguration[] = { "Target Realtime Fast", "Target Realtime Quality", "Target Realtime Max Quality", "Custom" };
	static int currentPostProcessConfiguration = meshImportSettings->postProcessConfiguration;
	ImGui::PushItemWidth(100.0f);
	if (ImGui::Combo("Configuration", &currentPostProcessConfiguration, postProcessConfiguration, IM_ARRAYSIZE(postProcessConfiguration)))
		meshImportSettings->postProcessConfiguration = (MeshImportSettings::MeshPostProcessConfiguration)currentPostProcessConfiguration;
	ImGui::PopItemWidth();

	if (currentPostProcessConfiguration != MeshImportSettings::MeshPostProcessConfiguration::CUSTOM)
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);

	ImGui::Checkbox("Calculate Tangent Space", &meshImportSettings->calcTangentSpace);
	ImGui::Checkbox("Generate Normals", &meshImportSettings->genNormals);
	
	if (currentPostProcessConfiguration != MeshImportSettings::MeshPostProcessConfiguration::TARGET_REALTIME_FAST)
		ImGui::Checkbox("Generate Smooth Normals", &meshImportSettings->genSmoothNormals);
	
	ImGui::Checkbox("Join Identical Vertices", &meshImportSettings->joinIdenticalVertices);
	ImGui::Checkbox("Triangulate", &meshImportSettings->triangulate);
	ImGui::Checkbox("Generate UV Coordinates", &meshImportSettings->genUVCoords);
	ImGui::Checkbox("Sort By Primitive Type", &meshImportSettings->sortByPType);
	ImGui::Checkbox("Improve Cache Locality", &meshImportSettings->improveCacheLocality);
	ImGui::Checkbox("Limit Bone Weights", &meshImportSettings->limitBoneWeights);
	ImGui::Checkbox("Remove Redundant Materials", &meshImportSettings->removeRedundantMaterials);
	ImGui::Checkbox("Split Large Meshes", &meshImportSettings->splitLargeMeshes);
	ImGui::Checkbox("Find Degenerates", &meshImportSettings->findDegenerates);
	ImGui::Checkbox("Find Invalid Data", &meshImportSettings->findInvalidData);
	ImGui::Checkbox("Find Instances", &meshImportSettings->findInstances);
	ImGui::Checkbox("Validate Data Structure", &meshImportSettings->validateDataStructure);
	ImGui::Checkbox("Optimize Meshes", &meshImportSettings->optimizeMeshes);

	if (currentPostProcessConfiguration != MeshImportSettings::MeshPostProcessConfiguration::CUSTOM)
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, false);
}

void PanelInspector::ShowTextureImportSettingsInspector()
{
	TextureImportSettings* textureImportSettings = (TextureImportSettings*)App->scene->selectedObject.Get();

	ImGui::Text("Texture Import Settings");
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::Spacing();
	ImGui::Text("Import Settings");
	ImGui::Spacing();

	const char* compression[] = { "DXT1", "DXT2", "DXT3", "DXT4", "DXT5" };
	int currentCompression = textureImportSettings->compression;
	ImGui::PushItemWidth(100.0f);
	if (ImGui::Combo("Compression", &currentCompression, compression, IM_ARRAYSIZE(compression)))
		textureImportSettings->compression = (TextureImportSettings::TextureCompression)currentCompression;

	ImGui::Spacing();
	ImGui::Text("Load Settings");
	ImGui::Spacing();

	ImGui::Text("Wrap Mode");
	const char* wrap[] = { "Repeat", "Mirrored Repeat", "Clamp To Edge", "Clamp To Border" };
	int currentWrapS = textureImportSettings->wrapS;
	int currentWrapT = textureImportSettings->wrapT;
	if (ImGui::Combo("Wrap S", &currentWrapS, wrap, IM_ARRAYSIZE(wrap)))
		textureImportSettings->wrapS = (TextureImportSettings::TextureWrapMode)currentWrapS;
	if (ImGui::Combo("Wrap T", &currentWrapT, wrap, IM_ARRAYSIZE(wrap)))
		textureImportSettings->wrapT = (TextureImportSettings::TextureWrapMode)currentWrapT;

	ImGui::Text("Filter Mode");
	const char* filter[] = { "Nearest", "Linear",
		"Nearest Mipmap Nearest", "Linear Mipmap Nearest", "Nearest Mipmap Linear", "Linear Mipmap Linear" };
	int currentMinFilter = textureImportSettings->minFilter;
	int currentMagFilter = textureImportSettings->magFilter;
	if (ImGui::Combo("Min Filter", &currentMinFilter, filter, IM_ARRAYSIZE(filter)))
		textureImportSettings->minFilter = (TextureImportSettings::TextureFilterMode)currentMinFilter;
	if (ImGui::Combo("Mag Filter", &currentMagFilter, filter, IM_ARRAYSIZE(filter)))
		textureImportSettings->magFilter = (TextureImportSettings::TextureFilterMode)currentMagFilter;
	ImGui::PopItemWidth();

	if (textureImportSettings->UseMipmap())
		ImGui::TextColored(YELLOW, "Mip Maps will be generated");

	if (App->materialImporter->IsAnisotropySupported())
		ImGui::SliderFloat("Anisotropy", &textureImportSettings->anisotropy, 0.0f, App->materialImporter->GetLargestSupportedAnisotropy());
}

#endif // GAME