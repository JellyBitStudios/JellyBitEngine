#include "ComponentProjector.h"

#include "Application.h"
#include "ModuleWindow.h"
#include "ModuleRenderer3D.h"
#include "ModuleResourceManager.h"
#include "ModuleInternalResHandler.h"
#include "ModuleLayers.h"
#include "ModuleScene.h"
#include "ModuleFBOManager.h"
#include "GLCache.h"

#include "ResourceMaterial.h"
#include "ResourceMesh.h"
#include "GameObject.h"
#include "ComponentTransform.h"

#include "MathGeoLib\include\Geometry\Sphere.h"

#include "imgui\imgui.h"

#include <assert.h>

ComponentProjector::ComponentProjector(GameObject* parent, bool include) : Component(parent, ComponentTypes::ProjectorComponent)
{
	SetMaterialRes(App->resHandler->defaultMaterial);
	SetMeshRes(App->resHandler->cube);

	// Init frustum
	frustum.type = math::FrustumType::PerspectiveFrustum;

	frustum.pos = math::float3::zero;
	frustum.front = math::float3::unitZ;
	frustum.up = math::float3::unitY;

	frustum.nearPlaneDistance = 1.0f;
	frustum.farPlaneDistance = 5.0f;
	frustum.verticalFov = 60.0f * DEGTORAD;
	frustum.horizontalFov = 60.0f * DEGTORAD;

	// -----

	if (include)
		App->renderer3D->AddProjectorComponent(this);
}

ComponentProjector::ComponentProjector(const ComponentProjector& componentProjector, GameObject* parent, bool include) : Component(parent, ComponentTypes::ProjectorComponent)
{
	if (App->res->GetResource(componentProjector.materialRes) != nullptr)
		SetMaterialRes(componentProjector.materialRes);
	else
		SetMaterialRes(App->resHandler->defaultMaterial);

	if (App->res->GetResource(componentProjector.meshRes) != nullptr)
		SetMeshRes(componentProjector.meshRes);
	else
		SetMeshRes(App->resHandler->cube);

	// Init frustum
	frustum.type = componentProjector.frustum.type;

	frustum.pos = componentProjector.frustum.pos;
	frustum.front = componentProjector.frustum.front;
	frustum.up = componentProjector.frustum.up;

	frustum.nearPlaneDistance = componentProjector.frustum.nearPlaneDistance;
	frustum.farPlaneDistance = componentProjector.frustum.farPlaneDistance;
	frustum.verticalFov = componentProjector.frustum.verticalFov;
	frustum.horizontalFov = componentProjector.frustum.horizontalFov;

	// -----

	if (include)
		App->renderer3D->AddProjectorComponent(this);
}

ComponentProjector::~ComponentProjector()
{
	App->renderer3D->EraseProjectorComponent(this);
	parent->cmp_projector = nullptr;

	SetMaterialRes(0);
	SetMeshRes(0);
}

void ComponentProjector::UpdateTransform()
{
	math::float4x4 matrix = parent->transform->GetGlobalMatrix();
	frustum.pos = matrix.TranslatePart();
	frustum.front = matrix.WorldZ();
	frustum.up = matrix.WorldY();
}

void ComponentProjector::OnUniqueEditor()
{
#ifndef GAMEMODE
	if (ImGui::CollapsingHeader("Projector", ImGuiTreeNodeFlags_DefaultOpen))
	{
		// Frustum
		ImGui::Text("FRUSTUM");

		ImGui::Text("Field of view"); ImGui::SameLine(); ImGui::AlignTextToFramePadding();
		float fov = frustum.verticalFov * RADTODEG;
		if (ImGui::SliderFloat("##fov", &fov, 0.0f, 180.0f))
			SetFOV(fov);

		ImGui::Text("Near clip plane"); ImGui::PushItemWidth(50.0f);
		ImGui::DragFloat("##nearClipPlane", &frustum.nearPlaneDistance, 0.01f, 0.0f, FLT_MAX, "%.2f", 1.0f);
		ImGui::PopItemWidth();

		ImGui::Text("Far clip plane"); ImGui::PushItemWidth(50.0f);
		ImGui::DragFloat("##farClipPlane", &frustum.farPlaneDistance, 0.01f, 0.0f, FLT_MAX, "%.2f", 1.0f);
		ImGui::PopItemWidth();

		// Material
		ImGui::Spacing();
		ImGui::Text("MATERIAL");

		const Resource* resource = App->res->GetResource(materialRes);
		std::string materialName = resource->GetName();

		ImGui::PushID("material");
		ImGui::Button(materialName.data(), ImVec2(150.0f, 0.0f));
		ImGui::PopID();

		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::Text("%u", materialRes);
			ImGui::EndTooltip();
		}

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MATERIAL_INSPECTOR_SELECTOR"))
			{
				uint payload_n = *(uint*)payload->Data;
				SetMaterialRes(payload_n);
			}
			ImGui::EndDragDropTarget();
		}

		ImGui::SameLine();

		if (materialRes != App->resHandler->defaultMaterial)
		{
			if (ImGui::Button("EDIT"))
				SELECT(App->res->GetResource(materialRes));
		}

		if (ImGui::SmallButton("Use default material"))
			SetMaterialRes(App->resHandler->defaultMaterial);

		ImGui::Text("Alpha multiplier"); ImGui::PushItemWidth(50.0f);
		ImGui::DragFloat("##alphaMultiplier", &alphaMultiplier, 0.01f, 0.0f, 1.0f, "%.2f", 1.0f);
		ImGui::PopItemWidth();

		// Ignore layers
		ImGui::Spacing();

		std::string title;
		std::vector<Layer*> activeLayers;
		uint enabledLayers = 0;

		for (uint i = 0; i < MAX_NUM_LAYERS; ++i)
		{
			Layer* layer = App->layers->GetLayer(i);
			const char* layerName = layer->name.data();
			if (strcmp(layerName, "") == 0)
				continue;

			if (filterMask & layer->GetFilterGroup())
			{
				title.append(layer->name);
				title.append(", ");

				++enabledLayers;
			}

			activeLayers.push_back(layer);
		}

		if (enabledLayers == 0)
			title = "Nothing";
		else if (enabledLayers == activeLayers.size())
			title = "Everything";
		else
		{
			uint found = title.find_last_of(",");
			if (found != std::string::npos)
				title = title.substr(0, found);
		}

		ImGui::PushItemWidth(150.0f);
		if (ImGui::BeginCombo("Ignore layers", title.data()))
		{
			if (ImGui::Selectable("Nothing", enabledLayers == 0 ? true : false))
				filterMask = 0;

			if (ImGui::Selectable("Everything", enabledLayers == activeLayers.size() ? true : false))
			{
				for (uint i = 0; i < activeLayers.size(); ++i)
					filterMask |= activeLayers[i]->GetFilterGroup();
			}

			for (uint i = 0; i < activeLayers.size(); ++i)
			{
				Layer* layer = activeLayers[i];
				if (ImGui::Selectable(layer->name.data(), filterMask & layer->GetFilterGroup() ? true : false))
					filterMask ^= layer->GetFilterGroup();
			}
			ImGui::EndCombo();
		}
	}
#endif
}

uint ComponentProjector::GetInternalSerializationBytes()
{
	return sizeof(math::Frustum) +
		sizeof(uint) +
		sizeof(uint) +
		sizeof(uint);
}

void ComponentProjector::OnInternalSave(char*& cursor)
{
	size_t bytes = sizeof(math::Frustum);
	memcpy(cursor, &frustum, bytes);
	cursor += bytes;

	bytes = sizeof(uint);
	memcpy(cursor, &materialRes, bytes);
	cursor += bytes;

	bytes = sizeof(uint);
	memcpy(cursor, &meshRes, bytes);
	cursor += bytes;

	bytes = sizeof(uint);
	memcpy(cursor, &filterMask, bytes);
	cursor += bytes;

	/*
	bytes = sizeof(float);
	memcpy(cursor, &alphaMultiplier, bytes);
	cursor += bytes;
	*/
}

void ComponentProjector::OnInternalLoad(char*& cursor)
{
	size_t bytes = sizeof(math::Frustum);
	memcpy(&frustum, cursor, bytes);
	cursor += bytes;

	uint resource;

	bytes = sizeof(uint);
	memcpy(&resource, cursor, bytes);
	cursor += bytes;

	if (App->res->GetResource(resource) != nullptr)
		SetMaterialRes(resource);
	else
		SetMaterialRes(App->resHandler->defaultMaterial);

	bytes = sizeof(uint);
	memcpy(&resource, cursor, bytes);
	cursor += bytes;

	if (App->res->GetResource(resource) != nullptr)
		SetMeshRes(resource);
	else
		SetMeshRes(App->resHandler->cube);

	bytes = sizeof(uint);
	memcpy(&filterMask, cursor, bytes);
	cursor += bytes;

	/*
	bytes = sizeof(float);
	memcpy(&alphaMultiplier, cursor, bytes);
	cursor += bytes;
	*/
}

// ----------------------------------------------------------------------------------------------------

// Draws a decal
void ComponentProjector::Draw() const
{
	ResourceMaterial* resourceMaterial = (ResourceMaterial*)App->res->GetResource(materialRes);
	if (resourceMaterial == nullptr)
		return;

	const ResourceShaderProgram* resourceShaderProgram = (ResourceShaderProgram*)App->res->GetResource(resourceMaterial->GetShaderUuid());
	if (resourceShaderProgram == nullptr)
		return;

	/// Projective texture mapping shader
	uint shaderProgram = resourceShaderProgram->shaderProgram;
	App->glCache->SwitchShader(shaderProgram);

	// 1. Generic uniforms
	App->renderer3D->LoadGenericUniforms(shaderProgram);

	// 2. Known projector uniforms
	math::float4x4 bias_matrix = math::float4x4(
		0.5f, 0.0f, 0.0f, 0.5f,
		0.0f, 0.5f, 0.0f, 0.5f,
		0.0f, 0.0f, 0.5f, 0.5f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	math::float4x4 projector_view_matrix = GetOpenGLViewMatrix().Transposed();
	math::float4x4 projector_proj_matrix = GetOpenGLProjectionMatrix().Transposed();
	math::float4x4 projector_matrix = bias_matrix * projector_proj_matrix * projector_view_matrix;

	int location = glGetUniformLocation(shaderProgram, "projectorMatrix");
	if (location != -1)
		glUniformMatrix4fv(location, 1, GL_TRUE, projector_matrix.ptr());

	math::AABB aabb = frustum.MinimalEnclosingAABB();
	math::float3 aabbPosition = aabb.CenterPoint();
	math::float3 aabbScaling = aabb.Size();
	math::float4x4 aabbMatrix = math::float4x4::FromTRS(aabbPosition, math::Quat::identity, aabbScaling);

	math::float4x4 model_matrix = aabbMatrix;
	model_matrix = model_matrix.Transposed();
	ComponentCamera* camera = App->renderer3D->GetCurrentCamera();
	math::float4x4 mvp_matrix = model_matrix * App->renderer3D->viewProj_matrix;
	math::float4x4 normal_matrix = model_matrix;
	normal_matrix.Inverse();
	normal_matrix.Transpose();

	location = glGetUniformLocation(shaderProgram, "model_matrix");
	if (location != -1)
		glUniformMatrix4fv(location, 1, GL_FALSE, model_matrix.ptr());
	location = glGetUniformLocation(shaderProgram, "mvp_matrix");
	if (location != -1)
		glUniformMatrix4fv(location, 1, GL_FALSE, mvp_matrix.ptr());
	location = glGetUniformLocation(shaderProgram, "normal_matrix");
	if (location != -1)
		glUniformMatrix3fv(location, 1, GL_FALSE, normal_matrix.Float3x3Part().ptr());

	uint textureUnit = 0;

	glActiveTexture(GL_TEXTURE0 + textureUnit);
	glBindTexture(GL_TEXTURE_2D, App->fbo->gPosition);
	location = glGetUniformLocation(shaderProgram, "gBufferPosition");
	if (location != -1)
	{
		glUniform1i(location, textureUnit);
		++textureUnit;
	}

	glActiveTexture(GL_TEXTURE0 + textureUnit);
	glBindTexture(GL_TEXTURE_2D, App->fbo->gNormal);
	location = glGetUniformLocation(shaderProgram, "gBufferNormal");
	if (location != -1)
	{
		glUniform1i(location, textureUnit);
		++textureUnit;
	}

	glActiveTexture(GL_TEXTURE0 + textureUnit);
	glBindTexture(GL_TEXTURE_2D, App->fbo->gInfo);
	location = glGetUniformLocation(shaderProgram, "gBufferInfo");
	if (location != -1)
	{
		glUniform1i(location, textureUnit);
		++textureUnit;
	}

	uint screenScale = App->window->GetScreenSize();
	uint screenWidth = App->window->GetWindowWidth();
	uint screenHeight = App->window->GetWindowHeight();
	math::float2 screenSize = math::float2(screenWidth * screenScale, screenHeight * screenScale);

	location = glGetUniformLocation(shaderProgram, "screenSize");
	if (location != -1)
		glUniform2fv(location, 1, screenSize.ptr());

	location = glGetUniformLocation(shaderProgram, "alphaMultiplier");
	if (location != -1)
		glUniform1f(location, alphaMultiplier);

	location = glGetUniformLocation(shaderProgram, "filterMask");
	if (location != -1)
		glUniform1i(location, filterMask);

	// 3. Unknown uniforms
	std::vector<Uniform> uniforms = resourceMaterial->GetUniforms();
	std::vector<const char*> ignore;
	ignore.push_back("gBufferPosition");
	ignore.push_back("gBufferNormal");
	ignore.push_back("gBufferInfo");
	ignore.push_back("screenSize");
	ignore.push_back("filterMask");
	ignore.push_back("alphaMultiplier");
	App->renderer3D->LoadSpecificUniforms(textureUnit, uniforms, ignore);

	glDepthMask(false);

	/// Camera-box intersection test
	math::Sphere sphere = math::Sphere(camera->frustum.pos, camera->frustum.nearPlaneDistance);
	if (sphere.Intersects(aabb))
	{
		glFrontFace(GL_CW); // cull mode: clockwise
		glDepthFunc(GL_GREATER);
	}
	else
	{
		glFrontFace(GL_CCW); // cull mode: counterclockwise (default)
		glDepthFunc(GL_LESS);
	}

	// Mesh
	const ResourceMesh* mesh = (const ResourceMesh*)App->res->GetResource(meshRes);

	glBindVertexArray(mesh->GetVAO());
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->GetIBO());

	glDrawElements(GL_TRIANGLES, mesh->GetIndicesCount(), GL_UNSIGNED_INT, NULL);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	for (uint i = 0; i < App->renderer3D->GetMaxTextureUnits(); ++i)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	glFrontFace(GL_CCW); // cull mode: counterclockwise (default)
	glDepthFunc(GL_LESS);

	glDepthMask(true); // :)
}

// ----------------------------------------------------------------------------------------------------

void ComponentProjector::SetFOV(float fov)
{
	frustum.verticalFov = fov * DEGTORAD;
	frustum.horizontalFov = fov * DEGTORAD;
}

float ComponentProjector::GetFOV() const
{
	return frustum.verticalFov * RADTODEG;
}

float ComponentProjector::GetNearPlaneDistance()
{
	return frustum.nearPlaneDistance;
}

void ComponentProjector::SetNearPlaneDistance(float nearPlane)
{
	frustum.nearPlaneDistance = nearPlane;
}

float ComponentProjector::GetFarPlaneDistance()
{
	return frustum.farPlaneDistance;
}

void ComponentProjector::SetFarPlaneDistance(float farPlane)
{
	frustum.farPlaneDistance = farPlane;
}

math::Frustum ComponentProjector::GetFrustum() const
{
	return frustum;
}

math::float4x4 ComponentProjector::GetOpenGLViewMatrix() const
{
	math::float4x4 matrix = frustum.ViewMatrix();
	return matrix.Transposed();
}

math::float4x4 ComponentProjector::GetOpenGLProjectionMatrix() const
{
	math::float4x4 matrix = frustum.ProjectionMatrix();
	return matrix.Transposed();
}

void ComponentProjector::SetMaterialRes(uint materialUuid)
{
	if (materialRes > 0)
		App->res->SetAsUnused(materialRes);

	if (materialUuid > 0)
		App->res->SetAsUsed(materialUuid);

	materialRes = materialUuid;
}

void ComponentProjector::SetMaterialRes(std::string materialName)
{
	std::vector<Resource*> materials = App->res->GetResourcesByType(ResourceTypes::MaterialResource);
	for (Resource* material : materials)
	{
		if (material->GetData().name == materialName)
		{
			if (materialRes > 0)
				App->res->SetAsUnused(materialRes);
			
			materialRes = material->GetUuid();
			App->res->SetAsUsed(materialRes);
		}
	}
}

uint ComponentProjector::GetMaterialRes() const
{
	return materialRes;
}

std::string ComponentProjector::GetMaterialResName() const
{
	Resource* res = App->res->GetResource(materialRes);
	if (res != nullptr)
		return res->GetName();
	return "";
}

void ComponentProjector::SetAlphaMultiplier(float alphaMultiplier)
{
	this->alphaMultiplier = alphaMultiplier;

	if (this->alphaMultiplier < 0.0f)
		this->alphaMultiplier = 0.0f;
	else if (this->alphaMultiplier > 1.0f)
		this->alphaMultiplier = 1.0f;
}

float ComponentProjector::GetAlphaMultiplier() const
{
	return alphaMultiplier;
}

void ComponentProjector::SetMeshRes(uint meshUuid)
{
	if (meshRes > 0)
		App->res->SetAsUnused(meshRes);

	if (meshUuid > 0)
		App->res->SetAsUsed(meshUuid);

	meshRes = meshUuid;
}

uint ComponentProjector::GetMeshRes() const
{
	return meshRes;
}

void ComponentProjector::SetFilterMask(uint filterMask)
{
	this->filterMask = filterMask;
}

uint ComponentProjector::GetFilterMask() const
{
	return filterMask;
}