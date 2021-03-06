#include "PanelAssets.h"

#ifndef GAMEMODE

#include "Application.h"
#include "ModuleFileSystem.h"
#include "ModuleResourceManager.h"
#include "ModuleScene.h"
#include "MaterialImporter.h"
#include "SceneImporter.h"
#include "ShaderImporter.h"
#include "ModuleInternalResHandler.h"
#include "ScriptingModule.h"

#include "imgui\imgui.h"
#include "imgui\imgui_stl.h"

#include "Optick/include/optick.h"

#include "Resource.h"
#include "ResourceMesh.h"
#include "ResourceTexture.h"
#include "ResourceMaterial.h"
#include "ResourceAnimator.h"
#include "ResourceAvatar.h"
#include "ResourceShaderProgram.h"
#include "ResourcePrefab.h"
#include "ResourceFont.h"

#include "Shaders.h"
#include "GameObject.h"

PanelAssets::PanelAssets(const char* name) : Panel(name) {}

PanelAssets::~PanelAssets() {}

bool PanelAssets::Draw()
{
	ImGuiWindowFlags assetsFlags = 0;
	assetsFlags |= ImGuiWindowFlags_NoFocusOnAppearing;

	if (ImGui::Begin(name, &enabled, assetsFlags))
	{
		/*if (ImGui::Button("Re-Import"))
		{
			App->scripting->CreateDomain();

			App->fs->ForceReImport(App->fs->rootDir);

			System_Event event;
			event.type = System_Event_Type::DeleteUnusedFiles;
			App->PushSystemEvent(event);
		}*/

		//ImGui::SameLine();

		if(ImGui::Button("Read Assets changes"))
			App->fs->UpdateAssetsDir();

		ImGui::SameLine();

		if (ImGui::Button("Build"))
		{
			//1. Delete Library
			//2. Export everything in Assets
			//3. GenerateLibraryFiles event
			//4. Copy library, the readme, the dlls, (Settings?) and AudioFolder (temp) to a new Build folder and make a zip with it.

			//Delete library
			//bool ret = App->fs->deleteFiles("Library", "");
			/*if (ret == false)
				return false;*/

			//Export everything in Assets
			//App->fs->ImportMainDir(true);

			System_Event newEvent;

			newEvent.type = System_Event_Type::DeleteUnusedFiles;
			App->PushSystemEvent(newEvent);

			newEvent.type = System_Event_Type::GenerateLibraryFiles;
			App->PushSystemEvent(newEvent);

			newEvent.type = System_Event_Type::Build;
			App->PushSystemEvent(newEvent);

			App->fs->build = true;
		}

		bool treeNodeOpened = ImGui::TreeNodeEx(DIR_ASSETS);
		CreateResourcePopUp(DIR_ASSETS);

		//Create the dummy to receive GameObject drops from the hierarchy;
		/*ImVec2 drawingPos = ImGui::GetCursorScreenPos();
		ImVec2 winSize = ImGui::GetWindowSize();

		ImGui::SetCursorScreenPos(ImGui::GetWindowPos());

		ImGui::Dummy(winSize);
		ImGui::SetCursorScreenPos(drawingPos);*/
		
		if (treeNodeOpened)
		{
			RecursiveDrawAssetsDir(App->fs->rootDir);
			ImGui::TreePop();
		}
	}
	ImGui::End();

	if (showCreateResourceConfirmationPopUp)
	{
		char resource[DEFAULT_BUF_SIZE];
		ResourceTypes resourceType = App->res->GetResourceTypeByExtension(extension.data());
		switch (resourceType)
		{
		case ResourceTypes::ShaderObjectResource:
			strcpy_s(resource, strlen("Create Shader Object") + 1, "Create Shader Object");
			break;
		case ResourceTypes::MaterialResource:
			strcpy_s(resource, strlen("Create Material") + 1, "Create Material");
			break;
		case ResourceTypes::AnimatorResource:
			strcpy_s(resource, strlen("Create Animator") + 1, "Create Animator");
			break;
		case ResourceTypes::AvatarResource:
			strcpy_s(resource, strlen("Create Avatar") + 1, "Create Avatar");
			break;
		}

		ImGui::OpenPopup(resource);
		CreateResourceConfirmationPopUp();
	}
	else if (showDeleteResourceConfirmationPopUp)
	{
		char resource[DEFAULT_BUF_SIZE];
		ResourceTypes resourceType = App->res->GetResourceTypeByExtension(extension.data());
		switch (resourceType)
		{
		case ResourceTypes::ShaderObjectResource:
			strcpy_s(resource, strlen("Delete Shader Object") + 1, "Delete Shader Object");
			break;
		case ResourceTypes::ShaderProgramResource:
			strcpy_s(resource, strlen("Delete Shader Program") + 1, "Delete Shader Program");
			break;
		case ResourceTypes::MaterialResource:
			strcpy_s(resource, strlen("Delete Material") + 1, "Delete Material");
			break;
		case ResourceTypes::AnimatorResource:
			strcpy_s(resource, strlen("Delete Animator") + 1, "Delete Animator");
			break;
		case ResourceTypes::AvatarResource:
			strcpy_s(resource, strlen("Delete Avatar") + 1, "Delete Avatar");
			break;
		}

		ImGui::OpenPopup(resource);
		DeleteResourceConfirmationPopUp();
	}

	return true;
}

void PanelAssets::RecursiveDrawAssetsDir(const Directory& directory)
{
#ifndef GAMEMODE
	OPTICK_CATEGORY("PanelAssets_RecursiveDrawAssetsDir", Optick::Category::Debug);
#endif

	//TODO: ORGANIZE THIS LOGIC INTO THE OWN ONPANELASSETS METHOD:

	//	1* The stantard draw
	//	2* Manage the selection and show the import settings
	//	3* Drag and Drop support

	for (uint i = 0; i < directory.directories.size(); ++i)
	{
		ImGuiTreeNodeFlags flags = 0;
		flags |= ImGuiTreeNodeFlags_OpenOnArrow;

		Directory dir = directory.directories[i];

		bool treeNodeOpened = false;

		char id[DEFAULT_BUF_SIZE];
		sprintf_s(id, DEFAULT_BUF_SIZE, "%s##%s", dir.name.data(), dir.fullPath.data());

		if (ImGui::TreeNodeEx(id, flags))
			treeNodeOpened = true;

		if (ImGui::IsMouseReleased(0) && ImGui::IsItemHovered(ImGuiHoveredFlags_None)
			&& (ImGui::GetMousePos().x - ImGui::GetItemRectMin().x) > ImGui::GetTreeNodeToLabelSpacing())
			SELECT(NULL);

		CreateResourcePopUp(dir.fullPath.data());

		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GAMEOBJECTS_HIERARCHY", 0);
			if (payload)
			{
				if (dir.fullPath.find("Prefabs") != std::string::npos)
				{
					GameObject* gameObject = *(GameObject**)payload->Data;

					if (!gameObject->cmp_canvas && gameObject->GetLayer() == UILAYER)
					{
						CONSOLE_LOG(LogTypes::Error, "You can't make a prefab of child canvas. Only from Canvas.");
					}
					else
					{
						std::vector<Resource*> prefabs = App->res->GetResourcesByType(ResourceTypes::PrefabResource);
						for (Resource* prefab : prefabs)
						{
							if (prefab->GetData().name == gameObject->GetName())
							{
								App->res->DeleteResource(prefab->GetUuid());
							}
						}

						ResourceData data;
						data.file = dir.fullPath + std::string("/") + gameObject->GetName() + EXTENSION_PREFAB;
						data.exportedFile = "";
						data.name = std::string(gameObject->GetName()) + EXTENSION_PREFAB;

						PrefabData prefabData;
						prefabData.root = gameObject;

						App->res->ExportFile(ResourceTypes::PrefabResource, data, &prefabData, std::string());
					}
				}			
			}
			ImGui::EndDragDropTarget();
		}

		if (treeNodeOpened)
		{
			if (!(dir.files.empty() && dir.directories.empty()))
				RecursiveDrawAssetsDir(dir);
			ImGui::TreePop();
		}
	}

	for (uint i = 0; i < directory.files.size(); ++i)
	{
		File file = directory.files[i];

		std::string metaFile = directory.fullPath + "//" + file.name + ".meta";
		if (!App->fs->Exists(metaFile))
			continue;

		char* metaBuffer;
		uint metaSize = App->fs->Load(metaFile, &metaBuffer);
		if (metaSize <= 0)
			continue;

		char* cursor = metaBuffer;

		std::string extension;
		App->fs->GetExtension(file.name.data(), extension);
		ResourceTypes resourceType = App->res->GetResourceTypeByExtension(extension.data());

		switch (resourceType)
		{
			case ResourceTypes::MeshResource:
				{		
				ImGuiTreeNodeFlags flags = 0;
				flags |= ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_OpenOnArrow;

				if (App->scene->selectedObject == CurrentSelection::SelectedType::meshImportSettings)
				{
					ResourceMeshImportSettings* importSettings = (ResourceMeshImportSettings*)App->scene->selectedObject.Get();
					
					if (strstr(importSettings->modelPath, file.name.data()) != nullptr)
						flags |= ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_Selected;

				}

				char id[DEFAULT_BUF_SIZE];
				sprintf(id, "%s##%s", file.name.data(), directory.fullPath.data());

				bool fbxOpened = ImGui::TreeNodeEx(id, flags);

				ImVec2 mouseDelta = ImGui::GetMouseDragDelta(0);
				if (ImGui::IsMouseReleased(0) && ImGui::IsItemHovered(ImGuiHoveredFlags_None)
					&& (ImGui::GetMousePos().x - ImGui::GetItemRectMin().x) > ImGui::GetTreeNodeToLabelSpacing())
				{
					std::vector<uint> uids;
					std::vector<uint> bone_uuids;
					std::vector<uint> anim_uuids;
					ResourceMesh::ReadMeshesUuidsFromBuffer(cursor, uids, bone_uuids, anim_uuids);

					ResourceMesh* tempRes = (ResourceMesh*)App->res->GetResource(uids[0]);
					SELECT(tempRes->GetSpecificData().meshImportSettings);
				}
			
				if (fbxOpened)
				{
					std::vector<uint> uids;
					std::vector<uint> bone_uuids;
					std::vector<uint> anim_uuids;
					ResourceMesh::ReadMeshesUuidsFromBuffer(cursor, uids, bone_uuids, anim_uuids);

					for (int i = 0; i < uids.size(); ++i)
					{
						Resource* res = (Resource*)App->res->GetResource(uids[i]);
						if (res)
							res->OnPanelAssets();
					}

					for (int i = 0; i < anim_uuids.size(); ++i)
					{
						Resource* res = (Resource*)App->res->GetResource(anim_uuids[i]);
						if (res)
							res->OnPanelAssets();
					}

					ImGui::TreePop();
				}	
				break;
			}

			case ResourceTypes::FontResource:
			{
				ImGuiTreeNodeFlags flags = 0;
				flags |= ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_OpenOnArrow;

				if (App->scene->selectedObject == CurrentSelection::SelectedType::fontImportSettings)
				{
					FontImportSettings* importSettings = (FontImportSettings*)App->scene->selectedObject.Get();

					if (strstr(importSettings->fontPath.data(), file.name.data()) != nullptr)
						flags |= ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_Selected;

				}

				char id[DEFAULT_BUF_SIZE];
				sprintf(id, "%s##%s", file.name.data(), directory.fullPath.data());

				bool fontOpened = ImGui::TreeNodeEx(id, flags);

				ImVec2 mouseDelta = ImGui::GetMouseDragDelta(0);
				if (ImGui::IsMouseReleased(0) && ImGui::IsItemHovered(ImGuiHoveredFlags_None)
					&& (ImGui::GetMousePos().x - ImGui::GetItemRectMin().x) > ImGui::GetTreeNodeToLabelSpacing())
				{
					int64_t lastTime;
					std::vector<uint> uuids;
					FontImportSettings importSett;

					ResourceFont::ReadMetaFromBuffer(cursor, lastTime, uuids, importSett);
					
					importSett.fontPath = directory.fullPath + "/" + file.name;
					SELECT(importSett);
					
				}

				if (fontOpened)
				{
					int64_t lastTime;
					std::vector<uint> uuids;
					FontImportSettings importSett;

					ResourceFont::ReadMetaFromBuffer(cursor, lastTime, uuids, importSett);

					for (int i = 0; i < uuids.size(); ++i)
					{
						Resource* res = (Resource*)App->res->GetResource(uuids[i]);
						if (res)
							res->OnPanelAssets();
					}

					ImGui::TreePop();
				}
				break;
			}
			default:
			{
				uint uuid;
				cursor += sizeof(int64_t);
				cursor += sizeof(uint);
				
				memcpy(&uuid, cursor, sizeof(uint));

				Resource* res = (Resource*)App->res->GetResource(uuid);
				if (res)
					res->OnPanelAssets();

				if (resourceType == ResourceTypes::ShaderObjectResource
					|| resourceType == ResourceTypes::ShaderProgramResource
					|| resourceType == ResourceTypes::MaterialResource
					|| resourceType == ResourceTypes::AnimatorResource
					|| resourceType == ResourceTypes::AvatarResource)
					DeleteResourcePopUp(res->GetFile());

				break;
			}
		}

		delete[] metaBuffer;
	}
}

void PanelAssets::CreateResourcePopUp(const char* path)
{
	bool renameFolderClicked = false;

	if (ImGui::BeginPopupContextItem(path))
	{
		if (ImGui::Selectable("Rename Folder"))
		{
			renameFolderClicked = true;		
		}
		else if (ImGui::Selectable("Delete Folder"))
		{
			App->fs->deleteFiles(path, "", true);

			ImGui::CloseCurrentPopup();
		}
		else if (ImGui::Selectable("Create SubFolder"))
		{
			char newDir[DEFAULT_BUF_SIZE];
			sprintf(newDir, "%s/NewFolder", path);

			App->fs->CreateDir(newDir);

			ImGui::CloseCurrentPopup();
		}
		else if (ImGui::Selectable("Create Vertex Shader"))
		{
			extension = EXTENSION_VERTEX_SHADER_OBJECT;
			strcpy_s(resourceName, strlen("New Vertex Shader") + 1, "New Vertex Shader");
			file = path;
			file.append("/");

			showCreateResourceConfirmationPopUp = true;
			ImGui::CloseCurrentPopup();
		}
		else if (ImGui::Selectable("Create Fragment Shader"))
		{
			extension = EXTENSION_FRAGMENT_SHADER_OBJECT;
			strcpy_s(resourceName, strlen("New Fragment Shader") + 1, "New Fragment Shader");
			file = path;
			file.append("/");

			showCreateResourceConfirmationPopUp = true;
			ImGui::CloseCurrentPopup();
		}
		else if (ImGui::Selectable("Create Material"))
		{
			extension = EXTENSION_MATERIAL;
			strcpy_s(resourceName, strlen("New Material") + 1, "New Material");
			file = path;
			file.append("/");

			showCreateResourceConfirmationPopUp = true;
			ImGui::CloseCurrentPopup();
		}
		else if (ImGui::Selectable("Create Animator"))
		{
			extension = EXTENSION_ANIMATOR;
			strcpy_s(resourceName, strlen("New Animator") + 1, "New Animator");
			file = path;
			file.append("/");

			showCreateResourceConfirmationPopUp = true;
			ImGui::CloseCurrentPopup();
		}
		else if (ImGui::Selectable("Create Avatar"))
		{
			extension = EXTENSION_AVATAR;
			strcpy_s(resourceName, strlen("New Avatar") + 1, "New Avatar");
			file = path;
			file.append("/");

			showCreateResourceConfirmationPopUp = true;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	if (renameFolderClicked)
	{
		ImGui::OpenPopup((std::string(path) + "RenameFolder").data());
	}

	if (ImGui::BeginPopup((std::string(path) + "RenameFolder").data()))
	{
		std::string fullPath = path;
		fullPath = fullPath.substr(0, fullPath.find_last_of("/") + 1);

		static std::string newName;
		if (ImGui::InputText((std::string("NewName") + std::string("##") + path).data(), &newName, ImGuiInputTextFlags_::ImGuiInputTextFlags_EnterReturnsTrue))
		{
			fullPath += newName;
			App->fs->RenameDirectory(path, fullPath.data());
			newName = "";
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void PanelAssets::DeleteResourcePopUp(const char* path)
{
	if (ImGui::BeginPopupContextItem(path))
	{
		char resource[DEFAULT_BUF_SIZE];
		App->fs->GetExtension(path, extension);
		ResourceTypes resourceType = App->res->GetResourceTypeByExtension(extension.data());
		switch (resourceType)
		{
		case ResourceTypes::ShaderObjectResource:
			strcpy_s(resource, strlen("Delete Shader Object") + 1, "Delete Shader Object");
			break;
		case ResourceTypes::ShaderProgramResource:
			strcpy_s(resource, strlen("Delete Shader Program") + 1, "Delete Shader Program");
			break;
		case ResourceTypes::MaterialResource:
			strcpy_s(resource, strlen("Delete Material") + 1, "Delete Material");
			break;
		case ResourceTypes::AnimatorResource:
			strcpy_s(resource, strlen("Delete Animator") + 1, "Delete Animator");
			break;
		case ResourceTypes::AvatarResource:
			strcpy_s(resource, strlen("Delete Avatar") + 1, "Delete Avatar");
			break;
		}

		if (ImGui::Selectable(resource))
		{
			file = path;

			showDeleteResourceConfirmationPopUp = true;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void PanelAssets::CreateResourceConfirmationPopUp()
{
	char resource[DEFAULT_BUF_SIZE];
	ResourceTypes resourceType = App->res->GetResourceTypeByExtension(extension.data());
	switch (resourceType)
	{
	case ResourceTypes::ShaderObjectResource:
		strcpy_s(resource, strlen("Create Shader Object") + 1, "Create Shader Object");
		break;
	case ResourceTypes::MaterialResource:
		strcpy_s(resource, strlen("Create Material") + 1, "Create Material");
		break;
	case ResourceTypes::AnimatorResource:
		strcpy_s(resource, strlen("Create Animator") + 1, "Create Animator");
		break;
	case ResourceTypes::AvatarResource:
		strcpy_s(resource, strlen("Create Avatar") + 1, "Create Avatar");
		break;
	}

	if (ImGui::BeginPopupModal(resource, NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("%s", file.data());

		ImGui::PushItemWidth(200.0f);
		ImGui::InputText("##name", resourceName, INPUT_BUF_SIZE);

		ImGui::Text(extension.data());
		
		if (ImGui::Button("Create", ImVec2(120.0f, 0)))
		{
			file.append(resourceName);
			file.append(extension.data());

			ResourceData data;
			data.file = file;

			switch (resourceType)
			{
			case ResourceTypes::ShaderObjectResource:
			{
				// Basic shader object
				ResourceShaderObjectData shaderObjectData;
				ShaderObjectTypes shaderObjectType = ResourceShaderObject::GetShaderObjectTypeByExtension(extension.data());
				shaderObjectData.shaderObjectType = shaderObjectType;
				switch (shaderObjectType)
				{
				case ShaderObjectTypes::VertexType:
					shaderObjectData.SetSource(vShaderTemplate, strlen(vShaderTemplate));
					break;
				case ShaderObjectTypes::FragmentType:
					shaderObjectData.SetSource(fShaderTemplate, strlen(fShaderTemplate));
					break;
				case ShaderObjectTypes::GeometryType:
					shaderObjectData.SetSource(fShaderTemplate, strlen(fShaderTemplate)); // TODO GEOM GEOMETRY TEMPLATE
					break;
				}

				// Export the new file
				std::string outputFile;
				App->res->ExportFile(resourceType, data, &shaderObjectData, outputFile, true); // overwrite true since we already know the path
			}
			break;

			case ResourceTypes::MaterialResource:
			{
				// Basic material info
				ResourceMaterialData materialData;
				materialData.shaderUuid = App->resHandler->defaultShaderProgram;
				((ResourceShaderProgram*)App->res->GetResource(materialData.shaderUuid))->GetUniforms(materialData.uniforms);

				// Export the new file
				std::string outputFile;
				App->res->ExportFile(resourceType, data, &materialData, outputFile, true); // overwrite true since we already know the path
			}
			break;

			case ResourceTypes::AnimatorResource:
			{
				// Basic animator info
				ResourceAnimatorData animatorData;
				animatorData.avatar_uuid = 0u;
				animatorData.name = "Nameless animator uwu edition";

				// Export the new file
				std::string outputFile;
				App->res->ExportFile(resourceType, data, &animatorData, outputFile, true); // overwrite true since we already know the path
			}
			break;

			case ResourceTypes::AvatarResource:
			{
				// Basic avatar info
				ResourceAvatarData avatarData;

				// Export the new file
				std::string outputFile;
				App->res->ExportFile(resourceType, data, &avatarData, outputFile, true); // overwrite true since we already know the path
			}
			break;
			}

			showCreateResourceConfirmationPopUp = false;
			ImGui::CloseCurrentPopup();
		}

		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();

		if (ImGui::Button("Cancel", ImVec2(120.0f, 0)))
		{
			showCreateResourceConfirmationPopUp = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void PanelAssets::DeleteResourceConfirmationPopUp()
{
	char resource[DEFAULT_BUF_SIZE];
	ResourceTypes resourceType = App->res->GetResourceTypeByExtension(extension.data());
	switch (resourceType)
	{
	case ResourceTypes::ShaderObjectResource:
		strcpy_s(resource, strlen("Delete Shader Object") + 1, "Delete Shader Object");
		break;
	case ResourceTypes::ShaderProgramResource:
		strcpy_s(resource, strlen("Delete Shader Program") + 1, "Delete Shader Program");
		break;
	case ResourceTypes::MaterialResource:
		strcpy_s(resource, strlen("Delete Material") + 1, "Delete Material");
		break;
	case ResourceTypes::AnimatorResource:
		strcpy_s(resource, strlen("Delete Animator") + 1, "Delete Animator");
		break;
	case ResourceTypes::AvatarResource:
		strcpy_s(resource, strlen("Delete Avatar") + 1, "Delete Avatar");
		break;
	}

	if (ImGui::BeginPopupModal(resource, NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		switch (resourceType)
		{
		case ResourceTypes::ShaderObjectResource:
			ImGui::Text("Are you sure that you want to delete the following Shader Object?");
			break;
		case ResourceTypes::ShaderProgramResource:
			ImGui::Text("Are you sure that you want to delete the following Shader Program?");
			break;
		case ResourceTypes::MaterialResource:
			ImGui::Text("Are you sure that you want to delete the following Material?");
			break;
		case ResourceTypes::AnimatorResource:
			ImGui::Text("Are you sure that you want to delete the following Animator?");
			break;
		case ResourceTypes::AvatarResource:
			ImGui::Text("Are you sure that you want to delete the following Avatar?");
			break;
		}
		ImGui::TextColored(BLUE, "%s", file.data());

		if (ImGui::Button("Delete", ImVec2(120.0f, 0)))
		{
			App->fs->DeleteFileOrDir(file.data());

			showDeleteResourceConfirmationPopUp = false;
			ImGui::CloseCurrentPopup();
		}

		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();

		if (ImGui::Button("Cancel", ImVec2(120.0f, 0)))
		{
			showDeleteResourceConfirmationPopUp = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

#endif // GAME