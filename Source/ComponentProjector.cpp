#include "ComponentProjector.h"

#include "Application.h"
#include "ModuleWindow.h"
#include "ModuleRenderer3D.h"
#include "ModuleResourceManager.h"
#include "ModuleInternalResHandler.h"
#include "ModuleLayers.h"
#include "ModuleScene.h"

#include "Resource.h"
#include "GameObject.h"
#include "ComponentTransform.h"

#include "imgui\imgui.h"

#include <assert.h>

ComponentProjector::ComponentProjector(GameObject* parent) : Component(parent, ComponentTypes::ProjectorComponent)
{
	SetMaterialRes(App->resHandler->defaultMaterial);

	// Init frustum
	frustum.type = math::FrustumType::PerspectiveFrustum;

	frustum.pos = math::float3::zero;
	frustum.front = math::float3::unitZ;
	frustum.up = math::float3::unitY;

	frustum.nearPlaneDistance = 1.0f;
	frustum.farPlaneDistance = 500.0f;
	frustum.verticalFov = 60.0f * DEGTORAD;
	frustum.horizontalFov = 60.0f * DEGTORAD;

	// -----

	App->renderer3D->AddProjectorComponent(this);
}

ComponentProjector::ComponentProjector(const ComponentProjector& componentProjector, GameObject* parent) : Component(parent, ComponentTypes::ProjectorComponent)
{
	if (App->res->GetResource(componentProjector.materialRes) != nullptr)
		SetMaterialRes(componentProjector.materialRes);
	else
		SetMaterialRes(App->resHandler->defaultMaterial);

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

	App->renderer3D->AddProjectorComponent(this);
}

ComponentProjector::~ComponentProjector()
{
	App->renderer3D->EraseProjectorComponent(this);
	parent->cmp_projector = nullptr;
	SetMaterialRes(0);
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

		// Ignore layers
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
	memcpy(cursor, &filterMask, bytes);
	cursor += bytes;
}

void ComponentProjector::OnInternalLoad(char*& cursor)
{
	size_t bytes = sizeof(math::Frustum);
	memcpy(&frustum, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(uint);
	uint resource = 0;
	memcpy(&resource, cursor, bytes);

	if (App->res->GetResource(resource) != nullptr)
		SetMaterialRes(resource);
	else
		SetMaterialRes(App->resHandler->defaultMaterial);

	cursor += bytes;

	bytes = sizeof(uint);
	memcpy(&filterMask, cursor, bytes);
	cursor += bytes;
}

// ----------------------------------------------------------------------------------------------------

#include "ResourceMaterial.h"

// Draws a decal
void ComponentProjector::Draw() const
{
	ResourceMaterial* resourceMaterial = (ResourceMaterial*)App->res->GetResource(materialRes);
	if (resourceMaterial == nullptr)
		return;

	const ResourceShaderProgram* resourceShaderProgram = (ResourceShaderProgram*)App->res->GetResource(resourceMaterial->GetShaderUuid());
	if (resourceShaderProgram == nullptr) // TODO: or the shader is not a projector...
		return;

	/// Projective texture mapping shader
	uint shaderProgram = resourceShaderProgram->shaderProgram;
	glUseProgram(shaderProgram);

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

	uint location = glGetUniformLocation(shaderProgram, "projector_matrix");
	glUniformMatrix4fv(location, 1, GL_TRUE, projector_matrix.ptr());

	// 3. Unknown projector uniforms
	uint textureUnit = 0;

	std::vector<Uniform> uniforms = resourceMaterial->GetUniforms();
	App->renderer3D->LoadSpecificUniforms(textureUnit, uniforms);

	for (uint i = 0; i < App->renderer3D->GetMaxTextureUnits(); ++i)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	glUseProgram(0);
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

void ComponentProjector::SetNearPlaneDistance(float nearPlane)
{
	frustum.nearPlaneDistance = nearPlane;
}

void ComponentProjector::SetFarPlaneDistance(float farPlane)
{
	frustum.farPlaneDistance = farPlane;
}

void ComponentProjector::SetAspectRatio(float aspectRatio)
{
	frustum.horizontalFov = 2.0f * atanf(tanf(frustum.verticalFov * 0.5f) * aspectRatio);
}

void ComponentProjector::SetMaterialRes(uint materialUuid)
{
	if (materialRes > 0)
		App->res->SetAsUnused(materialRes);

	if (materialUuid > 0)
		App->res->SetAsUsed(materialUuid);

	materialRes = materialUuid;
}

uint ComponentProjector::GetMaterialRes() const
{
	return materialRes;
}

void ComponentProjector::SetFilterMask(uint filterMask)
{
	this->filterMask = filterMask;
}

uint ComponentProjector::GetFilterMask() const
{
	return filterMask;
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