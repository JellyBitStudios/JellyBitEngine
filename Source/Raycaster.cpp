#include "Raycaster.h"

#ifndef GAMEMODE

#include "Application.h"
#include "ModuleCameraEditor.h"
#include "ComponentCamera.h"
#include "ModuleWindow.h"
#include "GameObject.h"
#include "ComponentMesh.h"
#include "ComponentTransform.h"
#include "ModuleResourceManager.h"
#include "ResourceMesh.h"
#include "ModuleScene.h"
#include "ModuleGOs.h"

#include "MathGeoLib\include\Geometry\Triangle.h"
#include "MathGeoLib\include\Geometry\LineSegment.h"

Raycaster::Raycaster()
{
}

Raycaster::~Raycaster()
{
}

void Raycaster::ScreenPointToRay(int posX, int posY, float& shortestDistance, math::float3& shortestHitPoint, GameObject** hitGo) const
{
	*hitGo = nullptr;
	shortestHitPoint = math::float3::inf;
	shortestDistance = FLOAT_INF;

	int winWidth = App->window->GetWindowWidth();
	int winHeight = App->window->GetWindowHeight();
	float normalized_x = -(1.0f - (float(posX) * 2.0f) / winWidth);
	float normalized_y = 1.0f - (float(posY) * 2.0f) / winHeight;

	math::LineSegment raycast = App->camera->camera->frustum.UnProjectLineSegment(normalized_x, normalized_y);
	
	// Static objects
	std::vector<GameObject*> hits;
	App->scene->quadtree.CollectIntersections(hits, raycast);

	// Dynamic objects
	std::vector<GameObject*> dynamicGameObjects;
	App->GOs->GetDynamicGameobjects(dynamicGameObjects);

	for (uint i = 0; i < dynamicGameObjects.size(); ++i)
	{
		// TODO CHECK THIS
		if (dynamicGameObjects[i]->GetLayer() != UILAYER && raycast.Intersects(dynamicGameObjects[i]->boundingBox))
			hits.push_back(dynamicGameObjects[i]);
	}

	for (uint i = 0; i < hits.size(); ++i)
	{
		math::Triangle tri;
		math::LineSegment localSpaceSegment(raycast);
		localSpaceSegment.Transform(hits[i]->transform->GetGlobalMatrix().Inverted());

		if (hits[i]->cmp_mesh == nullptr)
			continue;

		const ResourceMesh* resMesh = (const ResourceMesh*)App->res->GetResource(hits[i]->cmp_mesh->res);

		if (resMesh == nullptr)
			continue;

		Vertex* vertices = nullptr;
		resMesh->GetVerticesReference(vertices);
		uint* indices = nullptr;
		resMesh->GetIndicesReference(indices);

		for (uint j = 0; j < resMesh->GetIndicesCount();)
		{
			tri.a = math::float3(vertices[indices[j]].position[0], vertices[indices[j]].position[1], vertices[indices[j]].position[2]); j++;
			tri.b = math::float3(vertices[indices[j]].position[0], vertices[indices[j]].position[1], vertices[indices[j]].position[2]); j++;
			tri.c = math::float3(vertices[indices[j]].position[0], vertices[indices[j]].position[1], vertices[indices[j]].position[2]); j++;

			float distance;
			math::float3 hitPoint;
			if (localSpaceSegment.Intersects(tri, &distance, &hitPoint))
			{
				if (shortestDistance > distance)
				{
					shortestDistance = distance;
					shortestHitPoint = hitPoint;
					*hitGo = (GameObject*)hits[i];
				}
			}
		}
	}
}

bool Raycaster::ScreenQuadToFrustum(int posX, int posY, int posW, int posH) const
{
	bool ret = false;

	int winWidth = App->window->GetWindowWidth();
	int winHeight = App->window->GetWindowHeight();	
	math::float2 width_height = math::Abs(math::float2(posW - posX, posH - posY));

	math::float2 centerPos;
	centerPos.x = -(1.0f - float(posX + posW) / winWidth);
	centerPos.y = 1.0f - float(posY + posH) / winHeight;

	math::Frustum selectionFrustum = CreateFrustum(App->camera->camera->frustum, width_height, centerPos);

	// Static objects
	std::vector<GameObject*> hits;
	App->scene->quadtree.CollectContains(hits, selectionFrustum);

	// Dynamic objects
	std::vector<GameObject*> dynamicGameObjects;
	App->GOs->GetDynamicGameobjects(dynamicGameObjects);

	for (uint i = 0; i < dynamicGameObjects.size(); ++i)
	{
		if (dynamicGameObjects[i]->GetLayer() != UILAYER && selectionFrustum.Contains(dynamicGameObjects[i]->boundingBox))
			hits.push_back(dynamicGameObjects[i]);
	}

	if (!hits.empty())
	{
		for (uint i = 0; i < hits.size(); ++i)
		{
			App->scene->selectedObject += hits[i];
		}
		ret = true;
	}

	return ret;
}

math::Frustum Raycaster::CreateFrustum(math::Frustum cameraFrustum, math::float2 width_heith, math::float2 centerPos) const
{
	math::Frustum frustumSelection;

	frustumSelection.type = cameraFrustum.type;

	frustumSelection.pos = cameraFrustum.NearPlanePos(centerPos);
	frustumSelection.front = cameraFrustum.front;
	frustumSelection.up = cameraFrustum.up;

	frustumSelection.nearPlaneDistance = cameraFrustum.nearPlaneDistance;
	frustumSelection.farPlaneDistance = cameraFrustum.farPlaneDistance;

	frustumSelection.verticalFov = cameraFrustum.verticalFov;
	frustumSelection.horizontalFov = 2.0f * atanf(tanf(frustumSelection.verticalFov / 2.0f) * width_heith.x / width_heith.y);

	return frustumSelection;
}

#endif
