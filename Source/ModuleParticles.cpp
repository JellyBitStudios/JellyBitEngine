#include "ModuleParticles.h"
#include "ModuleTimeManager.h"
#include "ModuleInput.h"

#include "Optick/include/optick.h"
#include <algorithm>
#include "MathGeoLib/include/Math/float4x4.h"
#include "Application.h"
#include "DebugDrawer.h"
#include "ComponentTransform.h"
#include "ModuleRenderer3D.h"

ModuleParticle::ModuleParticle(bool start_enabled) : Module(start_enabled)
{}

ModuleParticle::~ModuleParticle()
{}

update_status ModuleParticle::Update()
{
#ifndef GAMEMODE
	OPTICK_CATEGORY("ModuleParticle_Update", Optick::Category::VFX);
#endif // !GAMEMODE

	for (std::list<ComponentEmitter*>::iterator emitter = emitters.begin(); emitter != emitters.end(); ++emitter)
	{
		(*emitter)->Update();
	}

	int count = 0;

	float dt;
	if(App->IsEditor())
		dt = App->timeManager->GetRealDt();
	else
		dt = App->timeManager->GetDt();

	partVec.resize(activeParticles);
	
	int j = 0;
	for (int i = 0; i < MAX_PARTICLES; ++i)
	{
		if (allParticles[i].active)
		{
			allParticles[i].Update(dt); //Particles can be created here, they sould not be updated yet
			++count;
			allParticles[i].SetCamDistance();
			partVec[j++] = &allParticles[i];
		}
		else
		{
			allParticles[i].camDistance = -1;
		}
	}
	return UPDATE_CONTINUE;
}

void ModuleParticle::Draw()
{
	OPTICK_CATEGORY("ModuleParticle_Draw", Optick::Category::VFX);

	SortParticles();
	DrawParticles();
}

void ModuleParticle::DrawParticles()
{
	OPTICK_CATEGORY("ModuleParticle_DrawParticles", Optick::Category::VFX);
	for (int i = 0; i < partVec.size(); ++i)
	{
		if (partVec[i]->owner && partVec[i]->owner->isInFrustum)
			partVec[i]->Draw();
	}
}

void ModuleParticle::DebugDraw() const
{
	for (std::list<ComponentEmitter*>::iterator emitter = App->particle->emitters.begin(); emitter != App->particle->emitters.end(); ++emitter)
	{
		if ((*emitter)->drawShape)
		{
			math::float4x4 globalMat = (*emitter)->GetParent()->transform->GetGlobalMatrix();;
			switch ((*emitter)->normalShapeType)
			{
			case ShapeType_BOX:
				App->debugDrawer->DebugDraw((*emitter)->boxCreation, White, globalMat);
				break;
			case ShapeType_SPHERE:
			case ShapeType_SPHERE_BORDER:
			case ShapeType_SPHERE_CENTER:
				App->debugDrawer->DebugDrawSphere((*emitter)->sphereCreation.r, White, globalMat);
				break;
			case ShapeType_CONE:
				App->debugDrawer->DebugDrawCone((*emitter)->circleCreation.r, (*emitter)->coneHeight, White, globalMat);
				break;
			case ShapeType_MESH:
				App->debugDrawer->DebugDrawMesh((*emitter)->shapeMesh.meshVertex, (*emitter)->shapeMesh.indices, (*emitter)->shapeMesh.indicesSize, globalMat);
				break;
			default:
				break;
			}
		}
		if ((*emitter)->burst)
		{
			math::float4x4 globalMat = (*emitter)->GetParent()->transform->GetGlobalMatrix();;
			switch ((*emitter)->burstType)
			{
			case ShapeType_MESH:
				App->debugDrawer->DebugDrawMesh((*emitter)->burstMesh.meshVertex, (*emitter)->burstMesh.indices, (*emitter)->burstMesh.indicesSize, globalMat);
				break;
			default:
				break;
			}
		}
		if ((*emitter)->drawAABB)
			App->debugDrawer->DebugDraw((*emitter)->boundingBox, White);
	}
}

void ModuleParticle::SortParticles()
{
	OPTICK_CATEGORY("ModuleParticle_SoftParticles", Optick::Category::VFX);

	std::sort(partVec.begin(), partVec.end(), particleCompare());
}

bool ModuleParticle::GetParticle(int& id)
{
	for (int i = lastUsedParticle; i < MAX_PARTICLES; ++i) 
	{
		if (!allParticles[i].active) 
		{
			lastUsedParticle = i;
			id = i;
			return true;
		}
	}

	for (int i = 0; i < lastUsedParticle; ++i)
	{
		if (!allParticles[i].active)
		{
			lastUsedParticle = i;
			id = i;
			return true;
		}
	}
	return false;
}

void ModuleParticle::ClearEmitters()
{
	for (std::list<ComponentEmitter*>::iterator emitter = emitters.begin(); emitter != emitters.end(); ++emitter)
	{
		(*emitter)->SoftClearEmitter();
	}
	emitters.clear();

	for (int i = 0; i < MAX_PARTICLES; ++i)
	{
		allParticles[i].active = false;
		allParticles[i].owner = nullptr;		
	}

	activeParticles = 0;
	lastUsedParticle = 0;
}

void ModuleParticle::OnSystemEvent(System_Event event)
{
	switch (event.type)
	{
		case System_Event_Type::Play:
		case System_Event_Type::LoadFinished:
			for (std::list<ComponentEmitter*>::iterator emitter = emitters.begin(); emitter != emitters.end(); ++emitter)
			{
				if((*emitter)->startOnPlay)
					(*emitter)->StartEmitter();
			}
			break;
		case System_Event_Type::Stop:
			for (std::list<ComponentEmitter*>::iterator emitter = emitters.begin(); emitter != emitters.end(); ++emitter)
			{
				(*emitter)->ClearEmitter();
			}
			break;
		case System_Event_Type::LoadGMScene:
			for (std::list<ComponentEmitter*>::iterator emitter = emitters.begin(); emitter != emitters.end(); ++emitter)
			{
				(*emitter)->ConnectSubEmitter();
			}
			break;
	}
}

void ModuleParticle::RemoveEmitter(ComponentEmitter * emitter)
{
	emitters.remove(emitter);

	if (emitter->isSubEmitter)
	{
		for (std::list<ComponentEmitter*>::iterator iterator = emitters.begin(); iterator != emitters.end(); ++iterator)
		{
			if ((*iterator)->subEmitter == emitter->GetParent())
			{
				(*iterator)->startValues.subEmitterActive = false;
				(*iterator)->subEmitter = nullptr;
			}
		}
	}
}
