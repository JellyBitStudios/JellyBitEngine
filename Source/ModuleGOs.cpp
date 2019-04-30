#include "ModuleGOs.h"

#include "Application.h"
#include "ModuleScene.h"
#include "ModuleNavigation.h"
#include "ModuleResourceManager.h"
#include "ModuleInternalResHandler.h"
#include "ModuleRenderer3D.h"
#include "ModuleUI.h"
#include "Lights.h"
#include "ModuleFBOManager.h"

#include "GameObject.h"
#include "ComponentMaterial.h"
#include "ComponentProjector.h"
#include "ComponentAnimator.h"
#include "ComponentEmitter.h"
#include "ComponentMesh.h"
#include "ComponentImage.h"
#include "ComponentRectTransform.h"
#include "ComponentSlider.h"
#include "ResourceShaderProgram.h"
#include "ResourceAnimation.h"
#include "ComponentTrail.h"

#include <assert.h>

ModuleGOs::ModuleGOs(bool start_enabled) : Module(start_enabled)
{
	strcpy_s(nameScene, DEFAULT_BUF_SIZE, "Main Scene");
}

ModuleGOs::~ModuleGOs() {}

update_status ModuleGOs::Update() // TODO_G : for VS2 its ok but this have to be better
{
	for (std::vector<GameObject*>::const_iterator it = gameobjects.begin(); it != gameobjects.end(); ++it)
	{
		if (ComponentAnimator* anim_co = (ComponentAnimator*)(*it)->GetComponent(ComponentTypes::AnimatorComponent)) {
			anim_co->Update(); // hehehehhehehe
		}
	}
	return update_status::UPDATE_CONTINUE;
}

bool ModuleGOs::CleanUp()
{
	ClearScene();

	return true;
}

void ModuleGOs::OnSystemEvent(System_Event event)
{
	switch (event.type)
	{
	case System_Event_Type::CalculateBBoxes:
	case System_Event_Type::RecalculateBBoxes:

		for (auto it = gameobjects.begin(); it != gameobjects.end(); ++it)
		{
			if (event.goEvent.gameObject == *it)
				(*it)->OnSystemEvent(event);
		}

		break;

	case System_Event_Type::LayerNameReset:

		// Reset layer to default (all game objects)
		for (std::vector<GameObject*>::const_iterator it = gameobjects.begin(); it != gameobjects.end(); ++it)
		{
			if ((*it)->GetLayer() == event.layerEvent.layer)
				(*it)->SetLayer(0);
		}
		break;

	case System_Event_Type::GameObjectDestroyed:
		event.goEvent.gameObject->GetParent()->EraseChild(event.goEvent.gameObject);
		gameobjects.erase(std::remove(gameobjects.begin(), gameobjects.end(), event.goEvent.gameObject), gameobjects.end());
		staticGos.erase(std::remove(staticGos.begin(), staticGos.end(), event.goEvent.gameObject), staticGos.end());
		dynamicGos.erase(std::remove(dynamicGos.begin(), dynamicGos.end(), event.goEvent.gameObject), dynamicGos.end());
		break;

	case System_Event_Type::ComponentDestroyed:
	{
		GameObject* go = event.compEvent.component->GetParent();
		go->EraseComponent(event.compEvent.component);

		switch (event.compEvent.component->GetType())
		{
		case ComponentTypes::TransformComponent:
			go->transform = nullptr;
			break;
		case ComponentTypes::MeshComponent:
			go->cmp_mesh = nullptr;
			break;
		case ComponentTypes::MaterialComponent:
			go->cmp_material = nullptr;
			break;
		case ComponentTypes::CameraComponent:
			go->cmp_camera = nullptr;
			break;
		case ComponentTypes::NavAgentComponent:
			go->cmp_navAgent = nullptr;
			break;
		case ComponentTypes::EmitterComponent:
			go->cmp_emitter = nullptr;
			break;
		case ComponentTypes::BoneComponent:
			go->cmp_bone = nullptr;
			break;
		case ComponentTypes::LightComponent:
			go->cmp_light = nullptr;
			break;
		case ComponentTypes::ProjectorComponent:
			go->cmp_projector = nullptr;
			break;
		case ComponentTypes::AnimatorComponent:
			go->cmp_animator = nullptr;
			break;
		case ComponentTypes::RigidStaticComponent:
		case ComponentTypes::RigidDynamicComponent:
			go->cmp_rigidActor = nullptr;
			break;
		case ComponentTypes::BoxColliderComponent:
		case ComponentTypes::SphereColliderComponent:
		case ComponentTypes::CapsuleColliderComponent:
		case ComponentTypes::PlaneColliderComponent:
			go->cmp_collider = nullptr;
			break;
		case ComponentTypes::CanvasComponent:
			go->cmp_canvas = nullptr;
			break;
		case ComponentTypes::RectTransformComponent:
			go->cmp_rectTransform = nullptr; // Uh
			break;
		case ComponentTypes::ImageComponent:
			go->cmp_image = nullptr;
			break;
		case ComponentTypes::ButtonComponent:
			go->cmp_button = nullptr;
			break;
		case ComponentTypes::LabelComponent:
			go->cmp_label = nullptr;
			break;
		case ComponentTypes::SliderComponent:
			go->cmp_slider = nullptr;
			break;
		case ComponentTypes::CanvasRendererComponent:
			go->cmp_canvasRenderer = nullptr;
		case ComponentTypes::AudioListenerComponent:
			go->cmp_audioListener = nullptr;
			break;
		case ComponentTypes::AudioSourceComponent:
			go->cmp_audioSource = nullptr;
			break;
		}
		break;
	}
	case System_Event_Type::ResourceDestroyed:
		InvalidateResource(event.resEvent.resource);
		break;
	case System_Event_Type::ScriptingDomainReloaded:
	case System_Event_Type::Stop:
	case System_Event_Type::LoadFinished:
	{
		for (auto it = gameobjects.begin(); it != gameobjects.end(); ++it)
		{
			(*it)->OnSystemEvent(event);
		}
		break;
	}
	}
}

GameObject* ModuleGOs::CreateGameObject(const char* goName, GameObject* parent, bool disableTransform)
{
	GameObject* newGameObject = new GameObject(goName, parent, disableTransform);
	gameobjects.push_back(newGameObject);
	dynamicGos.push_back(newGameObject);

	return newGameObject;
}

GameObject* ModuleGOs::Instanciate(GameObject* copy, GameObject* newRoot)
{
	GameObject* newGameObject = new GameObject(*copy);

	if (!newRoot)
	{
		newGameObject->SetParent(copy->GetParent());
		copy->GetParent()->AddChild(newGameObject);
	}
	else
	{
		newGameObject->SetParent(newRoot);
		newRoot->AddChild(newGameObject);
	}

	std::vector<GameObject*> childs; newGameObject->GetChildrenAndThisVectorFromLeaf(childs);
	for (int i = 0; i < childs.size(); ++i)
	{
		gameobjects.push_back(childs[i]);
		App->GOs->RecalculateVector(childs[i], false);
	}


	System_Event newEvent;
	newEvent.type = System_Event_Type::RecreateQuadtree;
	App->PushSystemEvent(newEvent);

	// Calculate the global
	if (newGameObject && newGameObject->transform)
		newGameObject->transform->UpdateGlobal();

	return newGameObject;
}

void ModuleGOs::DeleteGameObject(GameObject* toDelete)
{
	toDelete->Destroy();
}

void ModuleGOs::Kill(GameObject* go)
{
	std::vector<GameObject*> toDestroy;
	go->GetChildrenAndThisVectorFromLeaf(toDestroy);
	for (int i = 0; i < toDestroy.size(); ++i)
	{
		for (int j = 0; j < toDestroy[i]->components.size(); ++j)
			RELEASE(toDestroy[i]->components[j]);
		RELEASE(toDestroy[i]);
	}
}

void ModuleGOs::GetGameobjects(std::vector<GameObject*>& gos) const
{
	gos = gameobjects;
}

void ModuleGOs::GetStaticGameobjects(std::vector<GameObject*>& gos) const
{
	gos = staticGos;
}

void ModuleGOs::GetDynamicGameobjects(std::vector<GameObject*>& gos) const
{
	gos = dynamicGos;
}

GameObject* ModuleGOs::GetGameObjectByUID(uint UID) const
{
	for (int i = 0; i < gameobjects.size(); ++i)
	{
		if (gameobjects[i]->GetUUID() == UID)
		{
			return gameobjects[i];
		}
	}

	return nullptr;
}

void ModuleGOs::ClearScene()
{
	for (int i = 0; i < gameobjects.size(); ++i)
		RELEASE(gameobjects[i]);

	gameobjects.clear();
	staticGos.clear();
	dynamicGos.clear();

	App->scene->FreeRoot();
}

void ModuleGOs::RecalculateVector(GameObject* go, bool sendEvent)
{
	dynamicGos.erase(std::remove(dynamicGos.begin(), dynamicGos.end(), go), dynamicGos.end());
	staticGos.erase(std::remove(staticGos.begin(), staticGos.end(), go), staticGos.end());

	if (go->IsStatic())
		staticGos.push_back(go);
	else
		dynamicGos.push_back(go);

	if (go->cmp_mesh)
		App->renderer3D->SwapComponents(go->cmp_mesh);

	if (sendEvent)
	{
		System_Event newEvent;
		newEvent.type = System_Event_Type::RecreateQuadtree;
		App->PushSystemEvent(newEvent);
	}
}

bool ModuleGOs::SerializeFromNode(GameObject* node, char*& outStateBuffer, size_t& sizeBuffer, bool navmesh)
{
	std::vector<GameObject*> go;
	node->GetChildrenVector(go, true);
	sizeBuffer = sizeof(uint);
	for (int i = 0; i < go.size(); ++i)
		sizeBuffer += go[i]->GetSerializationBytes();

	// Get size navmesh tiles data
	if (navmesh)
		sizeBuffer += App->navigation->GetNavMeshSerialitzationBytes();

	// ambient light value
	sizeBuffer += sizeof(float);

	outStateBuffer = new char[sizeBuffer];
	char* cursor = outStateBuffer;

	uint totalGO = go.size();
	memcpy(cursor, &totalGO, sizeof(uint));
	cursor += sizeof(uint);

	for (int i = 0; i < go.size(); ++i)
		go[i]->OnSave(cursor);

	// Discuss if this should be a resource
	if (navmesh)
		App->navigation->SaveNavmesh(cursor);

	memcpy(cursor, &App->lights->ambientValue, sizeof(float));
	cursor += sizeof(float);

	// Fog
	sizeBuffer += sizeof(math::float3) + /*sizeof(float) + sizeof(float)*/ + sizeof(float);

	memcpy(cursor, &App->lights->fog.color[0], sizeof(math::float3));
	cursor += sizeof(math::float3);
	/*
	memcpy(cursor, &App->lights->fog.minDist, sizeof(float));
	cursor += sizeof(float);
	memcpy(cursor, &App->lights->fog.maxDist, sizeof(float));
	cursor += sizeof(float);
	*/
	memcpy(cursor, &App->lights->fog.density, sizeof(float));
	cursor += sizeof(float);

	sizeBuffer += sizeof(math::float3);

	memcpy(cursor, &App->fbo->dotColor[0], sizeof(math::float3));
	cursor += sizeof(math::float3);

	return true;
}

GameObject* ModuleGOs::DeSerializeToNode(char*& buffer, size_t sizeBuffer, bool navmesh)
{
	char* cursor = buffer;
	size_t bytes = sizeof(uint);
	uint totalGO;
	memcpy(&totalGO, cursor, bytes);
	cursor += bytes;

	std::vector<GameObject*> gos;
	gos.reserve(totalGO);

	for (int i = 0; i < totalGO; ++i)
	{
		GameObject* go = new GameObject("", nullptr, true);
		go->OnLoad(cursor, false);

		for (int i = gos.size() - 1; i >= 0; --i)
		{
			if (gos[i]->GetUUID() == go->GetParentUUID())
			{
				go->SetParent(gos[i]);
				gos[i]->AddChild(go);
			}
		}

		if (go->GetParent() == 0)
			go->SetParent(0);

		gos.push_back(go);
	}

	return gos[0]; //the root node
}

bool ModuleGOs::LoadScene(char*& buffer, size_t sizeBuffer, bool navmesh)
{
	char* cursor = buffer;
	size_t bytes = sizeof(uint);
	uint totalGO;
	memcpy(&totalGO, cursor, bytes);
	cursor += bytes;

	std::vector<GameObject*> gos;
	gameobjects.reserve(totalGO - 1);
	gos.reserve(totalGO);

	for (int i = 0; i < totalGO; ++i)
	{
		GameObject* go = new GameObject("", nullptr, true);
		go->OnLoad(cursor);

		for (int i = gos.size() - 1; i >= 0; --i)
		{
			if (gos[i]->GetUUID() == go->GetParentUUID())
			{
				go->SetParent(gos[i]);
				gos[i]->AddChild(go);
			}
		}
		if (go->GetParent() == 0)
		{
			//assert(App->scene->root == 0);
			App->scene->root = go;
		}
		else
		{
			gameobjects.push_back(go);
			go->IsStatic() ? staticGos.push_back(go) : dynamicGos.push_back(go);
		}
		gos.push_back(go);
	}

	// Discuss if this should be a resource
	if (navmesh)
		App->navigation->LoadNavmesh(cursor);
	
	memcpy(&App->lights->ambientValue, cursor, sizeof(float));
	cursor += sizeof(float);

	// Fog
	memcpy(&App->lights->fog.color[0], cursor, sizeof(math::float3));
	cursor += sizeof(math::float3);
	/*
	memcpy(&App->lights->fog.minDist, cursor, sizeof(float));
	cursor += sizeof(float);
	memcpy(&App->lights->fog.maxDist, cursor, sizeof(float));
	cursor += sizeof(float);
	*/
	memcpy(&App->lights->fog.density, cursor, sizeof(float));
	cursor += sizeof(float);

	memcpy(&App->fbo->dotColor[0], cursor, sizeof(math::float3));
	cursor += sizeof(math::float3);

	//App->animation->SetUpAnimations();

	//StartAttachingBones(); SetUpAnimations();

	System_Event event;
	event.type = System_Event_Type::LoadFinished;
	App->PushSystemEvent(event);

	return true;
}

bool ModuleGOs::InvalidateResource(Resource* resource)
{
	assert(resource != nullptr);

	for (uint i = 0; i < gameobjects.size(); ++i)
	{
		switch (resource->GetType())
		{
		case ResourceTypes::MeshResource:

			// Mesh component uses Mesh resource
			if (gameobjects[i]->cmp_mesh != nullptr && gameobjects[i]->cmp_mesh->res == resource->GetUuid())
				gameobjects[i]->cmp_mesh->SetResource(0);

			break;

		case ResourceTypes::TextureResource:

			// UI component uses Texture resource
			if (gameobjects[i]->cmp_image != nullptr && gameobjects[i]->cmp_image->GetResImageUuid() == resource->GetUuid())
				gameobjects[i]->cmp_image->SetResImageUuid(App->resHandler->defaultTexture);
			if (gameobjects[i]->cmp_slider != nullptr && 
				(gameobjects[i]->cmp_slider->GetBackTexture() == resource->GetUuid() || gameobjects[i]->cmp_slider->GetFrontTexture() == resource->GetUuid()))
				gameobjects[i]->cmp_slider->SetFromInvadilateResource(App->resHandler->defaultTexture);

			break;

		case ResourceTypes::MaterialResource:

			// Material component uses Material resource
			if (gameobjects[i]->cmp_material != nullptr && gameobjects[i]->cmp_material->res == resource->GetUuid())
				gameobjects[i]->cmp_material->SetResource(App->resHandler->defaultMaterial);

			// Projector component uses Material resource
			if (gameobjects[i]->cmp_projector != nullptr && gameobjects[i]->cmp_projector->GetMaterialRes() == resource->GetUuid())
				gameobjects[i]->cmp_projector->SetMaterialRes(App->resHandler->defaultMaterial);

			// Emitter component uses Material resource
			if (gameobjects[i]->cmp_emitter != nullptr && gameobjects[i]->cmp_emitter->GetMaterialRes() == resource->GetUuid())
				gameobjects[i]->cmp_emitter->SetUuidRes(App->resHandler->defaultMaterial, gameobjects[i]->cmp_emitter->materialRes);

			break;
		}
	}

	return true;
}
