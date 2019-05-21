#include "Globals.h"
#include "Application.h"
#include "ModuleWindow.h"
#include "ModuleRenderer3D.h"
#include "ModuleScene.h"
#include "ModuleCameraEditor.h"
#include "ModuleResourceManager.h"
#include "ModuleTimeManager.h"
#include "ModulePhysics.h"
#include "ModuleParticles.h"
#include "ModuleTrails.h"
#include "ModuleGui.h"
#include "ModuleGOs.h"
#include "ModuleParticles.h"
#include "ModuleUI.h"
#include "ModuleFBOManager.h"
#include "ModuleInput.h"
#include "ScriptingModule.h"
#include "Lights.h"
#include "DebugDrawer.h"
#include "ShaderImporter.h"
#include "MaterialImporter.h"
#include "SceneImporter.h"
#include "Quadtree.h"
#include "GLCache.h"
#include "Raycaster.h"

#include "ComponentBone.h"
#include "ResourceBone.h"
#include "ResourceAvatar.h"

#include "GameObject.h"
#include "ComponentMesh.h"
#include "ComponentTransform.h"
#include "ComponentMaterial.h"
#include "ComponentCamera.h"
#include "ComponentProjector.h"

#include "ResourceMesh.h"
#include "ResourceTexture.h"
#include "ResourceMaterial.h"
#include "ResourceShaderProgram.h"

#include "ModuleNavigation.h"

#include "Optick/include/optick.h"

#pragma comment(lib, "opengl32.lib") /* link Microsoft OpenGL lib   */
#pragma comment(lib, "glu32.lib")    /* link OpenGL Utility lib     */
#pragma comment(lib, "glew\\libx86\\glew32.lib")

#include <stdio.h>
#include <algorithm>

ModuleRenderer3D::ModuleRenderer3D(bool start_enabled) : Module(start_enabled)
{
	name = "Renderer3D";
}

ModuleRenderer3D::~ModuleRenderer3D() {}

bool ModuleRenderer3D::Init(JSON_Object* jObject)
{
	bool ret = true;

	CONSOLE_LOG(LogTypes::Normal, "Creating 3D Renderer context");

	// Create context
	context = SDL_GL_CreateContext(App->window->window);

	if (context == NULL)
	{
		CONSOLE_LOG(LogTypes::Error, "OpenGL context could not be created! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}

	if (ret)
	{
		LoadStatus(jObject);

		// Initialize glew
		GLenum error = glewInit();
		if (error != GL_NO_ERROR)
		{
			CONSOLE_LOG(LogTypes::Error, "Error initializing glew! %s\n", glewGetErrorString(error));
			ret = false;
		}

		// Initialize Projection Matrix
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		// Check for error
		error = glGetError();
		if (error != GL_NO_ERROR)
		{
			CONSOLE_LOG(LogTypes::Error, "Error initializing OpenGL! %s\n", gluErrorString(error));
			ret = false;
		}

		// Initialize Modelview Matrix
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		// Check for error
		error = glGetError();
		if (error != GL_NO_ERROR)
		{
			CONSOLE_LOG(LogTypes::Error, "Error initializing OpenGL! %s\n", gluErrorString(error));
			ret = false;
		}

		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

		// Initialize clear depth
		glClearDepth(1.0f);

		// Initialize clear color
		glClearColor(0.f, 0.f, 0.f, 1.0f);
		//glEnable(GL_BLEND);
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Check for error
		error = glGetError();
		if (error != GL_NO_ERROR)
		{
			CONSOLE_LOG(LogTypes::Error, "Error initializing OpenGL! %s\n", gluErrorString(error));
			ret = false;
		}

		// GL capabilities
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glEnable(GL_LIGHTING);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_COLOR_MATERIAL);

		// Multitexturing
		glGetIntegerv(GL_MAX_TEXTURE_UNITS, (GLint*)&maxTextureUnits);

		for (uint i = 0; i < maxTextureUnits; ++i)
		{
			glActiveTexture(GL_TEXTURE0 + i);
			glEnable(GL_TEXTURE_2D);
		}

		glActiveTexture(GL_TEXTURE0);

		// Material Importer: anisotropic filtering
		if (GLEW_EXT_texture_filter_anisotropic)
		{
			float largestSupportedAnisotropy = 0;
			glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &largestSupportedAnisotropy);

			App->materialImporter->SetIsAnisotropySupported(true);
			App->materialImporter->SetLargestSupportedAnisotropy(largestSupportedAnisotropy);
		}
	}

#ifndef GAMEMODE
	// Editor camera
	currentCamera = App->camera->camera;
	// Projection Matrix
	OnResize(App->window->GetWindowWidth(), App->window->GetWindowHeight());
#endif // GAME

	return ret;
}

// PreUpdate: clear buffer
update_status ModuleRenderer3D::PreUpdate()
{
#ifndef GAMEMODE
	OPTICK_CATEGORY("ModuleRenderer3D_PreUpdate", Optick::Category::Rendering);
#endif // !GAMEMODE
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(currentCamera->GetOpenGLViewMatrix().ptr());

	return UPDATE_CONTINUE;
}

// PostUpdate: present buffer to screen
update_status ModuleRenderer3D::PostUpdate()
{
#ifndef GAMEMODE
	OPTICK_CATEGORY("ModuleRenderer3D_PostUpdate", Optick::Category::Rendering);
#endif
	App->fbo->BindGBuffer();

	if (currentCamera != nullptr)
	{
		for (uint i = 0; i < cameraComponents.size(); ++i)
		{
			if (cameraComponents[i]->IsTreeActive())
				cameraComponents[i]->UpdateTransform();
		}

		for (uint i = 0; i < projectorComponents.size(); ++i)
		{
			if (projectorComponents[i]->IsTreeActive())
				projectorComponents[i]->UpdateTransform();
		}

		viewProj_matrix = currentCamera->GetOpenGLViewMatrix() *
			currentCamera->GetOpenGLProjectionMatrix();
		std::vector<GameObject*> statics;
		std::vector<GameObject*> dynamics;
		if (currentCamera->HasFrustumCulling())
		{
			SetMeshComponentsSeenLastFrame(false);
			FrustumCulling(statics, dynamics);

			Sort(statics);
			Sort(dynamics);

			for each (ComponentMesh* mesh in rendererLast)
			{
				if (mesh->IsTreeActive())
					DrawMesh(mesh, true);
			}

			for (uint i = 0; i < statics.size(); ++i)
			{
				statics[i]->seenLastFrame = true;
				ComponentMesh* toDraw = statics[i]->cmp_mesh;
				if (toDraw->IsTreeActive())
					DrawMesh(toDraw);
			}

			// Draw decals
			bool blend = GetCapabilityState(GL_BLEND);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			for (uint i = 0; i < projectorComponents.size(); ++i)
			{
				if (projectorComponents[i]->GetParent()->IsActive()
					&& projectorComponents[i]->IsTreeActive())
					projectorComponents[i]->Draw();
			}

			if (!blend)
				glDisable(GL_BLEND);

			// Draw dynamic meshes
			for (uint i = 0; i < dynamics.size(); ++i)
			{
				dynamics[i]->seenLastFrame = true;
				ComponentMesh* toDraw = dynamics[i]->cmp_mesh;
				if (toDraw->IsTreeActive())
					DrawMesh(toDraw);
			}
		}
		else // Draw meshes w/out frustum culling
		{

			// Draw static meshes
			for (uint i = 0; i < staticMeshComponents.size(); ++i)
			{
				if (staticMeshComponents[i]->IsTreeActive())
					DrawMesh(staticMeshComponents[i]);
			}
			// Draw decals
			for (uint i = 0; i < projectorComponents.size(); ++i)
			{
				if (projectorComponents[i]->GetParent()->IsActive()
					&& projectorComponents[i]->IsTreeActive())
					projectorComponents[i]->Draw();
			}

			// Draw dynamic meshes
			for (uint i = 0; i < dynamicMeshComponents.size(); ++i)
			{
				if (dynamicMeshComponents[i]->IsTreeActive())
					DrawMesh(dynamicMeshComponents[i]);
			}
		}
	}

	App->fbo->DrawGBufferToScreen();

	App->fbo->MergeDepthBuffer(App->window->GetWindowWidth(), App->window->GetWindowHeight());

	App->glCache->SwitchShader(0);
	bool blend = GetCapabilityState(GL_BLEND);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(GL_FALSE);
	App->particle->Draw();
	glDepthMask(GL_TRUE);
	App->trails->Draw();
	if (!blend)
		glDisable(GL_BLEND);

	App->glCache->SwitchShader(0);

	App->scene->Draw();

#ifndef GAMEMODE
	if (debugDraw)
	{
		App->lights->DebugDrawLights();

		App->navigation->Draw();

		App->debugDrawer->StartDebugDraw();

		if (drawBones)
		{
			std::vector<GameObject*> gameObjects;
			App->GOs->GetGameobjects(gameObjects);
			for (uint i = 0; i < gameObjects.size(); ++i)
			{
				if (gameObjects[i]->cmp_bone != nullptr)
				{
					math::float4x4 globalMatrix = gameObjects[i]->transform->GetGlobalMatrix();
					App->debugDrawer->DebugDrawSphere(1.0f, Yellow, globalMatrix);
				}
			}
		}

		if (drawCurrentGO)
		{
			for (std::list<uint>::iterator iter = App->scene->multipleSelection.begin(); iter != App->scene->multipleSelection.end(); ++iter)
			{
				GameObject* curr = App->GOs->GetGameObjectByUID(*iter);
				if (curr && curr->boundingBox.IsFinite())
					App->debugDrawer->DebugDraw(curr->boundingBox, DeepPink);
			}
		}

		if (drawBoundingBoxes) // boundingBoxesColor = Yellow
		{
			Color boundingBoxesColor = Yellow;

			for (uint i = 0; i < staticMeshComponents.size(); ++i)
				App->debugDrawer->DebugDraw(staticMeshComponents[i]->GetParent()->boundingBox, boundingBoxesColor);

			for (uint i = 0; i < dynamicMeshComponents.size(); ++i)
				App->debugDrawer->DebugDraw(dynamicMeshComponents[i]->GetParent()->boundingBox, boundingBoxesColor);
		}

		if (drawFrustums) // boundingBoxesColor = Grey
		{
			Color frustumsColor = Grey;

			for (uint i = 0; i < cameraComponents.size(); ++i)
				App->debugDrawer->DebugDraw(cameraComponents[i]->frustum, frustumsColor);

			for (uint i = 0; i < projectorComponents.size(); ++i)
			{
				App->debugDrawer->DebugDraw(projectorComponents[i]->GetFrustum(), frustumsColor);
				App->debugDrawer->DebugDraw(projectorComponents[i]->GetFrustum().MinimalEnclosingAABB());
			}
			if (App->raycaster->frustumSelection.MinimalEnclosingAABB().IsFinite())
				App->debugDrawer->DebugDraw(App->raycaster->frustumSelection, frustumsColor);
		}

		if (drawColliders)
			App->physics->DrawColliders();

		if (drawQuadtree) // quadtreeColor = Blue, DarkBlue
			RecursiveDrawQuadtree(App->scene->quadtree.root);

		App->particle->DebugDraw();

		App->scripting->OnDrawGizmos();

		App->scripting->OnDrawGizmosSelected();

		App->debugDrawer->EndDebugDraw();
	}

	App->ui->DrawUI();

	// 3. Editor
	App->gui->Draw();

#else
	App->ui->DrawUI();
#endif // GAME

	App->input->DrawCursor();

	// 4. Swap buffers
	SDL_GL_MakeCurrent(App->window->window, context);
	SDL_GL_SwapWindow(App->window->window);

	return UPDATE_CONTINUE;
}

bool ModuleRenderer3D::CleanUp()
{
	bool ret = true;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	uint x, y;
	App->window->GetScreenSize(x, y);
	glViewport(0, 0, x, y);

	CONSOLE_LOG(LogTypes::Normal, "Destroying 3D Renderer");
	SDL_GL_DeleteContext(context);

	return ret;
}

void ModuleRenderer3D::OnSystemEvent(System_Event event)
{
	switch (event.type)
	{
	case System_Event_Type::Play:
#ifndef GAMEMODE
		CalculateProjectionMatrix();
#endif // !GAMEMODE
		break;
	case System_Event_Type::Stop:
#ifndef GAMEMODE
		currentCamera = App->camera->camera;
		CalculateProjectionMatrix();
#endif // !GAMEMODE
		break;
	case System_Event_Type::LoadFinished:
	{
		// Update all GameObjects transforms
		if (App->scene->root)
			for each (GameObject* child in App->scene->root->children)
			{
				if (child->transform != nullptr)
					child->transform->UpdateGlobal();
			}

		if (App->GetEngineState() == ENGINE_PLAY)
			SetCurrentCamera();

		for each (ComponentCamera* camera in cameraComponents)
		{
			camera->UpdateTransform();
		}

#ifndef GAMEMODE
		CalculateProjectionMatrix();
#endif // !GAMEMODE

		break;
	}
	}
}

void ModuleRenderer3D::SaveStatus(JSON_Object* jObject) const
{
	json_object_set_boolean(jObject, "vSync", vsync);
	json_object_set_boolean(jObject, "debugDraw", debugDraw);
	json_object_set_boolean(jObject, "drawBoundingBoxes", drawBoundingBoxes);
	json_object_set_boolean(jObject, "drawBones", drawBones);
	json_object_set_boolean(jObject, "drawColliders", drawColliders);
	json_object_set_boolean(jObject, "drawCamerasFrustum", drawFrustums);
	json_object_set_boolean(jObject, "drawCurrentGO", drawCurrentGO);
	json_object_set_boolean(jObject, "drawQuadtree", drawQuadtree);
}

void ModuleRenderer3D::LoadStatus(const JSON_Object* jObject)
{
	SetVSync(json_object_get_boolean(jObject, "vSync"));
	debugDraw = json_object_get_boolean(jObject, "debugDraw");
	drawBoundingBoxes = json_object_get_boolean(jObject, "drawBoundingBoxes");
	drawBones = json_object_get_boolean(jObject, "drawBones");
	drawColliders = json_object_get_boolean(jObject, "drawColliders");
	drawFrustums = json_object_get_boolean(jObject, "drawCamerasFrustum");
	drawCurrentGO = json_object_get_boolean(jObject, "drawCurrentGO");
	drawQuadtree = json_object_get_boolean(jObject, "drawQuadtree");
}

void ModuleRenderer3D::OnResize(int width, int height)
{
	glViewport(0, 0, width, height);
	currentCamera->SetAspectRatio((float)width / (float)height);
	CalculateProjectionMatrix();
}

void ModuleRenderer3D::CalculateProjectionMatrix()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glLoadMatrixf(currentCamera->GetOpenGLProjectionMatrix().ptr());

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

uint ModuleRenderer3D::GetMaxTextureUnits() const
{
	return maxTextureUnits;
}

bool ModuleRenderer3D::SetVSync(bool vsync)
{
	bool ret = true;

	this->vsync = vsync;

	if (this->vsync)
	{

		if (SDL_GL_SetSwapInterval(1) == -1)
		{
			ret = false;
			CONSOLE_LOG(LogTypes::Warning, "Unable to set VSync! SDL Error: %s\n", SDL_GetError());
		}
	}
	else {

		if (SDL_GL_SetSwapInterval(0) == -1)
		{
			ret = false;
			CONSOLE_LOG(LogTypes::Warning, "Unable to set immediate updates! SDL Error: %s\n", SDL_GetError());
		}
	}

	return ret;
}

bool ModuleRenderer3D::GetVSync() const
{
	return vsync;
}

void ModuleRenderer3D::SetCapabilityState(GLenum capability, bool enable) const
{
	if (GetCapabilityState(capability))
	{
		if (!enable)
			glDisable(capability);
	}
	else
	{
		if (enable)
			glEnable(capability);
	}
}

bool ModuleRenderer3D::GetCapabilityState(GLenum capability) const
{
	bool ret = false;

	if (glIsEnabled(capability))
		ret = true;

	return ret;
}

void ModuleRenderer3D::SetWireframeMode(bool enable) const
{
	if (enable)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

bool ModuleRenderer3D::IsWireframeMode() const
{
	bool ret = false;

	GLint polygonMode[2];
	glGetIntegerv(GL_POLYGON_MODE, polygonMode);

	if (polygonMode[0] == GL_LINE && polygonMode[1] == GL_LINE)
		ret = true;

	return ret;
}

bool ModuleRenderer3D::AddMeshComponent(ComponentMesh* toAdd)
{
	bool ret = true;
	std::vector<ComponentMesh*>::const_iterator it;
	bool isStatic = toAdd->GetParent()->IsStatic();
	if (isStatic)
	{
		it = std::find(staticMeshComponents.begin(), staticMeshComponents.end(), toAdd);
		ret = it == staticMeshComponents.end();
	}
	else
	{
		it = std::find(dynamicMeshComponents.begin(), dynamicMeshComponents.end(), toAdd);
		ret = it == dynamicMeshComponents.end();
	}

	if (ret)
	{
		if (isStatic)
			staticMeshComponents.push_back(toAdd);
		else
			dynamicMeshComponents.push_back(toAdd);
	}
	return ret;
}

bool ModuleRenderer3D::EraseMeshComponent(ComponentMesh* toErase)
{
	bool ret = false;

	std::vector<ComponentMesh*>::const_iterator it;

	it = std::find(staticMeshComponents.begin(), staticMeshComponents.end(), toErase);
	ret = it != staticMeshComponents.end();

	if (ret)
		staticMeshComponents.erase(it);

	it = std::find(dynamicMeshComponents.begin(), dynamicMeshComponents.end(), toErase);
	ret = it != dynamicMeshComponents.end();

	if (ret)
		dynamicMeshComponents.erase(it);

	return ret;
}

void ModuleRenderer3D::SwapComponents(ComponentMesh* toSwap)
{
	EraseMeshComponent(toSwap);
	AddMeshComponent(toSwap);
}

bool ModuleRenderer3D::AddProjectorComponent(ComponentProjector* toAdd)
{
	bool ret = true;

	std::vector<ComponentProjector*>::const_iterator it = std::find(projectorComponents.begin(), projectorComponents.end(), toAdd);
	ret = it == projectorComponents.end();

	if (ret)
		projectorComponents.push_back(toAdd);

	return ret;
}

bool ModuleRenderer3D::EraseProjectorComponent(ComponentProjector* toErase)
{
	bool ret = false;

	std::vector<ComponentProjector*>::const_iterator it = std::find(projectorComponents.begin(), projectorComponents.end(), toErase);
	ret = it != projectorComponents.end();

	if (ret)
		projectorComponents.erase(it);

	return ret;
}

bool ModuleRenderer3D::AddCameraComponent(ComponentCamera* toAdd)
{
	bool ret = true;

	std::vector<ComponentCamera*>::const_iterator it = std::find(cameraComponents.begin(), cameraComponents.end(), toAdd);
	ret = it == cameraComponents.end();

	if (ret)
		cameraComponents.push_back(toAdd);

	return ret;
}

bool ModuleRenderer3D::EraseCameraComponent(ComponentCamera* toErase)
{
	bool ret = false;

	if (cameraComponents.size() <= 0)
		return false;

	// TODO: Sometimes crash trying to erase editor camera. (its ok to call this method even if editor camera isnt in this vector, but it shouldnt crash)
	std::vector<ComponentCamera*>::const_iterator it = std::find(cameraComponents.begin(), cameraComponents.end(), toErase);
	ret = it != cameraComponents.end();

	if (ret)
		cameraComponents.erase(it);

	return ret;
}

bool ModuleRenderer3D::RecalculateMainCamera()
{
	bool ret = false;

	ComponentCamera* mainCamera = nullptr;

	bool multipleMainCameras = false;

	for (uint i = 0; i < cameraComponents.size(); ++i)
	{
		if (cameraComponents[i]->IsActive() && cameraComponents[i]->IsMainCamera())
		{
			if (mainCamera == nullptr)
				mainCamera = cameraComponents[i];
			else
			{
				multipleMainCameras = true;
				mainCamera = nullptr;
				break;
			}
		}
	}

	if (multipleMainCameras)
	{
		CONSOLE_LOG(LogTypes::Warning, "More than 1 Main Camera is defined");
	}
	else if (mainCamera == nullptr)
	{
		CONSOLE_LOG(LogTypes::Warning, "No Main Camera is defined");
	}

	ret = SetMainCamera(mainCamera);

	return ret;
}

bool ModuleRenderer3D::SetMainCamera(ComponentCamera* mainCamera)
{
	bool ret = mainCamera != nullptr;

	if (ret)
		this->mainCamera = mainCamera;
	else
		CONSOLE_LOG(LogTypes::Error, "Main Camera could not be set");

	return ret;
}

bool ModuleRenderer3D::SetCurrentCamera()
{
	bool ret = RecalculateMainCamera();

	if (ret)
	{
		currentCamera = mainCamera;
		SetMeshComponentsSeenLastFrame(!currentCamera->HasFrustumCulling());
	}
	else
		CONSOLE_LOG(LogTypes::Error, "Current Camera could not be set");

	return ret;
}

ComponentCamera* ModuleRenderer3D::GetMainCamera() const
{
	return mainCamera;
}

ComponentCamera* ModuleRenderer3D::GetCurrentCamera() const
{
	return currentCamera;
}

void ModuleRenderer3D::SetMeshComponentsSeenLastFrame(bool seenLastFrame)
{
	for (uint i = 0; i < staticMeshComponents.size(); ++i)
		staticMeshComponents[i]->GetParent()->seenLastFrame = seenLastFrame;

	for (uint i = 0; i < dynamicMeshComponents.size(); ++i)
		dynamicMeshComponents[i]->GetParent()->seenLastFrame = seenLastFrame;
}

void ModuleRenderer3D::FrustumCulling(std::vector<GameObject*>& statics, std::vector<GameObject*>& dynamics) const
{
	std::vector<GameObject*> gameObjects;
	App->GOs->GetGameobjects(gameObjects);

	// Static objects
	std::vector<GameObject*> staticsGOs;
	App->scene->quadtree.CollectIntersections(staticsGOs, currentCamera->frustum);

	for (uint i = 0; i < staticsGOs.size(); ++i)
	{
		if (staticsGOs[i]->boundingBox.IsFinite())
		{
			if (currentCamera->frustum.Intersects(staticsGOs[i]->boundingBox))
				statics.push_back(staticsGOs[i]);
		}
	}

	// Dynamic objects
	std::vector<GameObject*> dynamicGameObjects;
	App->GOs->GetDynamicGameobjects(dynamicGameObjects);

	for (uint i = 0; i < dynamicGameObjects.size(); ++i)
	{
		if (dynamicGameObjects[i]->boundingBox.IsFinite())
		{
			if (currentCamera->frustum.Intersects(dynamicGameObjects[i]->boundingBox))
				dynamics.push_back(dynamicGameObjects[i]);
		}
	}
}

void ModuleRenderer3D::DrawMesh(ComponentMesh* toDraw, bool drawLast) const
{
	if (toDraw->res == 0 || (!drawLast && toDraw->rendererFlags & ComponentMesh::DRAWLAST))
		return;

	uint textureUnit = 0;

	const ComponentMaterial* materialRenderer = toDraw->GetParent()->cmp_material;
	ResourceMaterial* resourceMaterial = materialRenderer->currentResource;
	GLuint shader = resourceMaterial->materialData.shaderProgram;
	App->glCache->SwitchShader(shader);

	// 1. Generic uniforms
	LoadGenericUniforms(shader);

	// 2. Known mesh uniforms
	math::float4x4 model_matrix = toDraw->GetParent()->transform->GetGlobalMatrix();
	model_matrix = model_matrix.Transposed();
	math::float4x4 mvp_matrix = model_matrix * viewProj_matrix;
	math::float4x4 normal_matrix = model_matrix;
	normal_matrix.Inverse();
	normal_matrix.Transpose();

	int location = glGetUniformLocation(shader, "model_matrix");
	glUniformMatrix4fv(location, 1, GL_FALSE, model_matrix.ptr());
	location = glGetUniformLocation(shader, "mvp_matrix");
	glUniformMatrix4fv(location, 1, GL_FALSE, mvp_matrix.ptr());
	location = glGetUniformLocation(shader, "normal_matrix");
	glUniformMatrix3fv(location, 1, GL_FALSE, normal_matrix.Float3x3Part().ptr());
	location = glGetUniformLocation(shader, "view_matrix");
	if (location != -1)
	{
		math::float4x4 view_matrix = currentCamera->GetOpenGLViewMatrix();
		glUniformMatrix4fv(location, 1, GL_FALSE, view_matrix.ptr());
	}
	location = glGetUniformLocation(shader, "Time");
	glUniform1f(location, App->timeManager->GetRealTime());
	uint screenScale = App->window->GetScreenSize();
	uint screenWidth = App->window->GetWindowWidth();
	uint screenHeight = App->window->GetWindowHeight();
	math::float2 screenSize = math::float2(screenWidth * screenScale, screenHeight * screenScale);

	location = glGetUniformLocation(shader, "screenSize");
	glUniform2fv(location, 1, screenSize.ptr());
	location = glGetUniformLocation(shader, "dot");
	glUniform1i(location, drawLast || !toDraw->GetParent()->IsStatic() ? 1 : 0);

	// Animations
	char boneName[DEFAULT_BUF_SIZE];
	ResourceAvatar* avatarResource = (ResourceAvatar*)App->res->GetResource(toDraw->avatarResource);
	bool animate = avatarResource != nullptr /*&& avatarResource->GetIsAnimated()*/;

	location = glGetUniformLocation(shader, "animate");
	glUniform1i(location, animate);

	if (animate)
	{
		std::vector<uint> bonesUuids = avatarResource->GetBonesUuids();
		for (uint i = 0; i < bonesUuids.size(); ++i)
		{
			/// Bone game object
			GameObject* boneGameObject = App->GOs->GetGameObjectByUID(bonesUuids[i]);
			if (boneGameObject == nullptr)
				continue;

			/// Bone component
			ComponentBone* boneComponent = boneGameObject->cmp_bone;
			if (boneComponent == nullptr)
				continue;

			/// Bone resource
			ResourceBone* boneResource = (ResourceBone*)App->res->GetResource(boneComponent->res);
			if (boneResource == nullptr)
				continue;

			math::float4x4 boneGlobalMatrix = boneComponent->GetParent()->transform->GetGlobalMatrix();
			math::float4x4 meshMatrix = toDraw->GetParent()->transform->GetGlobalMatrix().Inverted();
			math::float4x4 boneTransform = meshMatrix * boneGlobalMatrix * boneResource->boneData.offsetMatrix;

			sprintf_s(boneName, "bones[%u]", i);
			location = glGetUniformLocation(shader, boneName);
			glUniformMatrix4fv(location, 1, GL_TRUE, boneTransform.ptr());
		}
	}

	location = glGetUniformLocation(shader, "color");
	if (location != -1)
	{
		math::float4 color = materialRenderer->GetColor();
		glUniform4fv(location, 1, &color[0]);
	}
	location = glGetUniformLocation(shader, "pct");
	if (location != -1)
		glUniform1f(location, materialRenderer->GetPct());

	// Fog
	location = glGetUniformLocation(shader, "view_matrix");
	if (location != -1)
	{
		math::float4x4 view_matrix = currentCamera->GetOpenGLViewMatrix();
		glUniformMatrix4fv(location, 1, GL_FALSE, view_matrix.ptr());
	}

	location = glGetUniformLocation(shader, "fog.color");
	if (location != -1)
		glUniform3fv(location, 1, &App->lights->fog.color[0]);

	location = glGetUniformLocation(shader, "fog.density");
	if (location != -1)
		glUniform1f(location, App->lights->fog.density);

	// 3. Unknown mesh uniforms
	std::vector<Uniform> uniforms = resourceMaterial->GetUniforms();
	std::vector<const char*> ignore;
	ignore.push_back("animate");
	ignore.push_back("color");
	ignore.push_back("pct");
	ignore.push_back("view_matrix");
	ignore.push_back("fog.color");
	ignore.push_back("fog.density");

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, App->fbo->gInfo);
	location = glGetUniformLocation(shader, "gInfoTexture");
	glUniform1i(location, 0);
	textureUnit += 1;

	LoadSpecificUniforms(textureUnit, uniforms, ignore);

	// Mesh
	const ResourceMesh* mesh = toDraw->currentResource;

	glBindVertexArray(mesh->GetVAO());
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->GetIBO());

	bool adjacency = mesh->UseAdjacency();
	glDrawElements(adjacency ? GL_TRIANGLES_ADJACENCY : GL_TRIANGLES, adjacency ? mesh->GetIndicesCount() * 2 : mesh->GetIndicesCount(), GL_UNSIGNED_INT, NULL);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	for (uint i = 0; i < maxTextureUnits; ++i)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	if (animate)
	{
		std::vector<uint> bonesUuids = avatarResource->GetBonesUuids();
		for (uint i = 0; i < bonesUuids.size(); ++i)
		{
			math::float4x4 boneTransform = math::float4x4::identity;
			sprintf_s(boneName, "bones[%u]", i);
			location = glGetUniformLocation(shader, boneName);
			glUniformMatrix4fv(location, 1, GL_TRUE, boneTransform.ptr());
		}
	}
}

void ModuleRenderer3D::RecursiveDrawQuadtree(QuadtreeNode* node) const
{
	App->debugDrawer->DebugDraw(node->boundingBox, Blue);

	for (std::list<GameObject*>::const_iterator it = node->objects.begin(); it != node->objects.end(); ++it)
		App->debugDrawer->DebugDraw((*it)->boundingBox, DarkBlue);

	if (!node->IsLeaf())
	{
		for (uint i = 0; i < 4; ++i)
			RecursiveDrawQuadtree(node->children[i]);
	}
}

void ModuleRenderer3D::LoadSpecificUniforms(uint& textureUnit, const std::vector<Uniform>& uniforms, const std::vector<const char*>& ignore) const
{
	for (uint i = 0; i < uniforms.size(); ++i)
	{
		Uniform uniform = uniforms[i];

		for (uint j = 0; j < ignore.size(); ++j)
		{
			if (strcmp(uniform.common.name, ignore[j]) == 0)
				goto hereWeGo;
		}

		switch (uniform.common.type)
		{
		case Uniforms_Values::FloatU_value:
			glUniform1f(uniform.common.location, uniform.floatU.value);
			break;
		case Uniforms_Values::IntU_value:
			glUniform1i(uniform.common.location, uniform.intU.value);
			break;
		case Uniforms_Values::Vec2FU_value:
			glUniform2f(uniform.common.location, uniform.vec2FU.value.x, uniform.vec2FU.value.y);
			break;
		case Uniforms_Values::Vec3FU_value:
			glUniform3f(uniform.common.location, uniform.vec3FU.value.x, uniform.vec3FU.value.y, uniform.vec3FU.value.z);
			break;
		case Uniforms_Values::Vec4FU_value:
			glUniform4f(uniform.common.location, uniform.vec4FU.value.x, uniform.vec4FU.value.y, uniform.vec4FU.value.z, uniform.vec4FU.value.w);
			break;
		case Uniforms_Values::Vec2IU_value:
			glUniform2i(uniform.common.location, uniform.vec2IU.value.x, uniform.vec2IU.value.y);
			break;
		case Uniforms_Values::Vec3IU_value:
			glUniform3i(uniform.common.location, uniform.vec3IU.value.x, uniform.vec3IU.value.y, uniform.vec3IU.value.z);
			break;
		case Uniforms_Values::Vec4IU_value:
			glUniform4i(uniform.common.location, uniform.vec4IU.value.x, uniform.vec4IU.value.y, uniform.vec4IU.value.z, uniform.vec4IU.value.w);
			break;
		case Uniforms_Values::Sampler2U_value:
			if (textureUnit < maxTextureUnits && uniform.sampler2DU.value.id > 0)
			{
				glActiveTexture(GL_TEXTURE0 + textureUnit);
				glBindTexture(GL_TEXTURE_2D, uniform.sampler2DU.value.id);
				glUniform1i(uniform.common.location, textureUnit);
				++textureUnit;
			}
			break;
		}

	hereWeGo:;
	}
}

void ModuleRenderer3D::LoadGenericUniforms(uint shaderProgram) const
{
	int location = glGetUniformLocation(shaderProgram, "viewPos");
	if (location != -1)
		glUniform3fv(location, 1, currentCamera->frustum.pos.ptr());

	/*
	switch (App->GetEngineState())
	{
	// Game
	case ENGINE_PLAY:
	case ENGINE_PAUSE:
	case ENGINE_STEP:
	glUniform1f(location, App->timeManager->GetTime());
	break;
	// Editor
	case ENGINE_EDITOR:
	glUniform1f(location, App->timeManager->GetRealTime());
	break;
	}
	*/
}

bool renderSortDeferred(const GameObject* a, const GameObject* b)
{
	return a->cmp_material->currentResource->materialData.shaderProgram < b->cmp_material->currentResource->materialData.shaderProgram;
}

void ModuleRenderer3D::Sort(std::vector<GameObject*> toSort) const
{
	std::sort(toSort.begin(), toSort.end(), renderSortDeferred);
}