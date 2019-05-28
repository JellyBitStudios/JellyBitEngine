#include "ModuleEvents.h"
#include "EventSystem.h"

#include "ComponentTypes.h"
#include "Component.h"

#include "ComponentTransform.h"
#include "ComponentMesh.h"
#include "ComponentMaterial.h"
#include "ComponentCamera.h"
#include "ComponentNavAgent.h"
#include "ComponentEmitter.h"
#include "ComponentScript.h"
#include "ComponentBone.h"
#include "ComponentAnimation.h"
#include "ComponentAnimator.h"
#include "ComponentLight.h"
#include "ComponentProjector.h"

// UI
#include "ComponentCanvas.h"
#include "ComponentCanvasRenderer.h"
#include "ComponentRectTransform.h"
#include "ComponentImage.h"
#include "ComponentButton.h"
#include "ComponentLabel.h"

// Physics
#include "ComponentCollider.h"
#include "ComponentBoxCollider.h"
#include "ComponentCapsuleCollider.h"
#include "ComponentSphereCollider.h"
#include "ComponentPlaneCollider.h"
#include "ComponentRigidActor.h"
#include "ComponentRigidStatic.h"
#include "ComponentRigidDynamic.h"

// Audio
#include "ComponentAudioListener.h"
#include "ComponentAudioSource.h"

// Trail
#include "ComponentTrail.h"

#include "ResourceScene.h"
#include "ResourceScript.h"

#include "Application.h"
#include "ModuleGOs.h"
#include "ModuleResourceManager.h"
#include "ModuleScene.h"
#include "ModuleFileSystem.h"
#include "ScriptingModule.h"

void ModuleEvents::OnSystemEvent(System_Event event)
{
	switch (event.type)
	{
	case System_Event_Type::ComponentDestroyed:
	{
		delete event.compEvent.component;
		break;
	}
	case System_Event_Type::GameObjectDestroyed:
	{
		delete event.goEvent.gameObject;
		break;
	}
	case System_Event_Type::ResourceDestroyed:
	{
		App->res->EraseResource(event.resEvent.resource);
		delete event.resEvent.resource;
		break;
	}

	case System_Event_Type::LoadFinished:
	{
#ifdef GAMEMODE
		if (App->GetEngineState() != engine_states::ENGINE_PLAY)
		{
			System_Event newEvent;
			newEvent.type = System_Event_Type::Play;
			App->PushSystemEvent(newEvent);
		}
#endif
		// Mesh updated: recalculate bounding boxes
		System_Event updateBB;
		updateBB.goEvent.gameObject = App->scene->root;
		updateBB.type = System_Event_Type::RecalculateBBoxes;
		App->PushSystemEvent(updateBB);

		// Bounding box changed: recreate quadtree
		System_Event newEvent;
		newEvent.type = System_Event_Type::RecreateQuadtree;
		App->PushSystemEvent(newEvent);

		break;
	}
	case System_Event_Type::Play:
		assert(App->GOs->sceneStateBuffer == 0);
		App->GOs->SerializeFromNode(App->scene->root, App->GOs->sceneStateBuffer, App->GOs->sceneStateSize);
		App->SetEngineState(engine_states::ENGINE_PLAY);

		App->scripting->Play();

		break;
	case System_Event_Type::SaveScene:
	{
		ResourceScene::ExportFile(event.sceneEvent.nameScene);
		break;
	}

	case System_Event_Type::ScriptingDomainReloaded:
	{
		App->scripting->CreateDomain();
		App->scripting->UpdateScriptingReferences();

		std::vector<Resource*> scriptResources = App->res->GetResourcesByType(ResourceTypes::ScriptResource);
		for (int i = 0; i < scriptResources.size(); ++i)
		{
			ResourceScript* scriptRes = (ResourceScript*)scriptResources[i];
			scriptRes->referenceMethods();
		}

		App->scripting->ReInstance();

		App->scripting->TemporalLoad();

		break;
	}

	case System_Event_Type::Stop:
		assert(App->GOs->sceneStateBuffer != 0);
#ifndef GAMEMODE
		App->scene->selectedObject = 0;
#endif
		App->GOs->ClearScene();
		App->GOs->LoadScene(App->GOs->sceneStateBuffer, App->GOs->sceneStateSize);
		delete[] App->GOs->sceneStateBuffer;
		App->GOs->sceneStateBuffer = 0;
		System_Event newEvent;
		newEvent.type = System_Event_Type::RecreateQuadtree;
		App->PushSystemEvent(newEvent);
		break;
	case System_Event_Type::LoadScene:
	{
		char metafile[DEFAULT_BUF_SIZE];
#ifndef GAMEMODE
		sprintf_s(metafile, "%s/%s%s%s", DIR_ASSETS_SCENES, event.sceneEvent.nameScene, EXTENSION_SCENE, EXTENSION_META);
#else
		sprintf_s(metafile, "%s/%s%s%s", DIR_LIBRARY_SCENES, event.sceneEvent.nameScene, EXTENSION_SCENE, EXTENSION_META);
#endif
		if (App->fs->Exists(metafile))
		{
			char* metaBuffer;
			size_t size = App->fs->Load(metafile, &metaBuffer);
			if (size != 0)
			{
				uint UID;
				char* cursor = metaBuffer;
				cursor += sizeof(int64_t) + sizeof(uint);
				memcpy(&UID, cursor, sizeof(uint));

				delete[] metaBuffer;

				ResourceScene* scene = (ResourceScene*)App->res->GetResource(UID);
				if (scene)
				{
					App->scene->currentScene = scene->GetName();

#ifndef GAMEMODE
					App->scene->selectedObject = 0;
#endif
					char* sceneBuffer;
					uint sceneSize = scene->GetSceneBuffer(sceneBuffer);

					App->GOs->ClearScene();
					App->GOs->LoadScene(sceneBuffer, sceneSize, true);
					delete[] sceneBuffer;
				}
				else
					CONSOLE_LOG(LogTypes::Error, "Unable to find the Scene...");
			}
			else
				CONSOLE_LOG(LogTypes::Error, "Unable to find the Scene...");

			break;
		}
	}
	}
}
