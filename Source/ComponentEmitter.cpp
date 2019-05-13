#include "Application.h"

#include "ModuleGOs.h"
#include "ComponentEmitter.h"
#include "Application.h"
#include "ModuleTimeManager.h"
#include "ModuleScene.h"
#include "ResourceTexture.h"
#include "ResourceMesh.h"
#include "ModuleResourceManager.h"
#include "ModuleInternalResHandler.h"
#include "ModuleRenderer3D.h"

#include "ComponentMaterial.h"
#include "ComponentTransform.h"

#include <vector>

#include "ModuleParticles.h"
#include "imgui\imgui.h"

ComponentEmitter::ComponentEmitter(GameObject* gameObject, bool include) : Component(gameObject, EmitterComponent)
{
	if (include)
	{
		boundingBox = math::AABB();
		boundingBox.SetFromCenterAndSize(gameObject->transform->GetGlobalMatrix().TranslatePart(), math::float3::one);
		App->particle->emitters.push_back(this);

		SetUuidRes(App->resHandler->defaultMaterial, materialRes);
	}
	SetUuidRes(App->resHandler->plane, uuidMeshPart);
}

ComponentEmitter::ComponentEmitter(const ComponentEmitter& componentEmitter, GameObject* parent, bool include) : Component(parent, EmitterComponent)
{
	duration = componentEmitter.duration;

	loop = componentEmitter.loop;

	burst = componentEmitter.burst;
	minPart = componentEmitter.minPart;
	maxPart = componentEmitter.maxPart;
	repeatTime = componentEmitter.repeatTime;

	// personal boundingBox
	boundingBox = componentEmitter.boundingBox;

	// boxCreation
	boxCreation = componentEmitter.boxCreation;
	// SphereCreation
	sphereCreation.r = componentEmitter.sphereCreation.r;

	circleCreation.r = componentEmitter.circleCreation.r;

	coneHeight = componentEmitter.coneHeight;

	normalShapeType = componentEmitter.normalShapeType;
	burstType = componentEmitter.burstType;
	SetBurstText();

	startValues = componentEmitter.startValues;

	checkLife = componentEmitter.checkLife;
	checkSpeed = componentEmitter.checkSpeed;
	checkAcceleration = componentEmitter.checkAcceleration;
	checkSize = componentEmitter.checkSize;
	checkRotation = componentEmitter.checkRotation;
	checkAngularAcceleration = componentEmitter.checkAngularAcceleration;
	checkSizeOverTime = componentEmitter.checkSizeOverTime;
	checkAngularVelocity = componentEmitter.checkAngularVelocity;

	localSpace = componentEmitter.localSpace;

	particleAnim.isParticleAnimated = componentEmitter.particleAnim.isParticleAnimated;
	if (particleAnim.isParticleAnimated)
	{
		checkAnimationSpeed = componentEmitter.checkAnimationSpeed;
		particleAnim.animationSpeed = componentEmitter.particleAnim.animationSpeed;
		particleAnim.textureRows = componentEmitter.particleAnim.textureRows;
		particleAnim.textureColumns = componentEmitter.particleAnim.textureColumns;
		particleAnim.textureRowsNorm = componentEmitter.particleAnim.textureRowsNorm;
		particleAnim.textureColumnsNorm = componentEmitter.particleAnim.textureColumnsNorm;
		particleAnim.randAnim = componentEmitter.particleAnim.randAnim;
	}

	dieOnAnimation = componentEmitter.dieOnAnimation;

	drawAABB = componentEmitter.drawAABB;

	isSubEmitter = componentEmitter.isSubEmitter;
	subEmitter = componentEmitter.subEmitter;

	rateOverTime = componentEmitter.rateOverTime;

	if (include)
	{
		App->particle->emitters.push_back(this);

		if (App->res->GetResource(componentEmitter.materialRes) != nullptr)
			SetUuidRes(componentEmitter.materialRes, materialRes);
		else
			SetUuidRes(App->resHandler->defaultMaterial, materialRes);

		SetMeshParticleRes(componentEmitter.shapeMesh.uuid);
		SetBurstMeshParticleRes(componentEmitter.burstMesh.uuid);

		if (componentEmitter.uuidMeshPart > 0)
			SetUuidRes(componentEmitter.uuidMeshPart, uuidMeshPart);
		else
			SetUuidRes(App->resHandler->plane, uuidMeshPart);
	}

	startOnPlay = componentEmitter.startOnPlay;

	if (startOnPlay && App->IsPlay())
		StartEmitter();
}

ComponentEmitter::~ComponentEmitter()
{
	SetUuidRes(0, materialRes);
	SetMeshParticleRes(0);
	SetBurstMeshParticleRes(0);

	App->timeManager->RemoveGameTimer(&timer);
	App->timeManager->RemoveGameTimer(&burstTime);
	App->timeManager->RemoveGameTimer(&loopTimer);
	App->timeManager->RemoveGameTimer(&timeSimulating);

	App->particle->RemoveEmitter(this);
	ClearEmitter();

	SetUuidRes(0, uuidMeshPart);
}

void ComponentEmitter::StartEmitter()
{
	if (!isSubEmitter)
	{
		timer.Start();
		burstTime.Start();
		loopTimer.Start();

		timeToParticle = 0.0f;
		isPlaying = true;
		burstLoop = true;
		if (subEmitter)
			((ComponentEmitter*)subEmitter)->isPlaying = true;
	}
}

void ComponentEmitter::ChangeGameState(SimulatedGame state)
{
	simulatedGame = state;
	if (state == SimulatedGame_PLAY)
		state = SimulatedGame_STOP;
	else if (state == SimulatedGame_STOP)
		ClearEmitter();

	if (subEmitter)
	{
		ComponentEmitter* compEmitter = (ComponentEmitter*)(subEmitter->GetComponent(EmitterComponent));
		if (compEmitter)
			compEmitter->ChangeGameState(state);
	}
}

void ComponentEmitter::Update()
{
	if (isPlaying)
	{
		if (rateOverTime > 0)
		{
			float time = timer.ReadSec();
			if (time > timeToParticle && (loop || loopTimer.ReadSec() < duration))
			{
				if (App->IsPlay() || simulatedGame == SimulatedGame_PLAY || App->IsStep())
				{
					int particlesToCreate = (time / (1.0f / rateOverTime));
					CreateParticles(particlesToCreate, normalShapeType, math::float3::zero);
					timeToParticle = (1.0f / rateOverTime);

					timer.Start();
				}

			}
		}
		if (burst && burstTime.ReadSec() > repeatTime && (loop || burstLoop))
		{
			if (App->IsPlay() || simulatedGame == SimulatedGame_PLAY || App->IsStep())
			{
				int particlesToCreate = minPart;
				if (minPart != maxPart)
					particlesToCreate = (rand() % (maxPart - minPart)) + minPart;
				CreateParticles(particlesToCreate, burstType, math::float3::zero, true);
			}
			burstTime.Start();
			if (!loop)
				burstLoop = false;
		}

		//Used for SubEmitter. Create particles from ParticleEmiter death (On Emiter update because need to resize before Particle update)
		if (!newPositions.empty())
		{
			for (std::list<math::float3>::const_iterator iterator = newPositions.begin(); iterator != newPositions.end(); ++iterator)
			{
				CreateParticles(rateOverTime, normalShapeType, *iterator);
			}

			newPositions.clear();
		}
		if (App->renderer3D->GetCurrentCamera()->frustum.Intersects(boundingBox))
			isInFrustum = true;
		else
			isInFrustum = false;
	}
}

void ComponentEmitter::ClearEmitter()
{
	for (std::list<Particle*>::iterator iterator = particles.begin(); iterator != particles.end(); ++iterator)
	{
		(*iterator)->active = false;
		(*iterator)->owner = nullptr;
	}

	App->particle->activeParticles -= particles.size();

	particles.clear();
	isPlaying = false;
}

void ComponentEmitter::SetLifeTime(float life)
{
	startValues.life.x = startValues.life.y = life;
}

void ComponentEmitter::ConnectSubEmitter()
{
	if (uuidSubEmitter > 0)
		subEmitter = App->GOs->GetGameObjectByUID(uuidSubEmitter);
}

void ComponentEmitter::SoftClearEmitter()
{
	App->particle->activeParticles -= particles.size();

	particles.clear();
}


void ComponentEmitter::CreateParticles(int particlesToCreate, ShapeType shapeType, const math::float3& pos, bool isBurst)
{
	if (particlesToCreate == 0)
		++particlesToCreate;

	for (int i = 0; i < particlesToCreate; ++i)
	{
		int particleId = 0;
		if (App->particle->GetParticle(particleId))
		{
			math::float3 spawnPos = pos;

			spawnPos += RandPos(shapeType, isBurst);

			App->particle->allParticles[particleId].SetActive(spawnPos, startValues, particleAnim);

			App->particle->allParticles[particleId].owner = this;
			particles.push_back(&App->particle->allParticles[particleId]);
		}
		else
			break;
	}
}

math::float3 ComponentEmitter::RandPos(ShapeType shapeType, bool isBurst)
{
	math::float3 spawn = math::float3::zero;

	switch (shapeType)
	{
	case ShapeType_BOX:
		spawn = boxCreation.RandomPointInside(App->randomMathLCG);
		if (localSpace)
			startValues.particleDirection = math::float3::unitY;
		else
			startValues.particleDirection = (math::float3::unitY * parent->transform->GetGlobalMatrix().RotatePart().Transposed()).Normalized();
		break;

	case ShapeType_SPHERE:
		spawn = sphereCreation.RandomPointInside(App->randomMathLCG);
		startValues.particleDirection = spawn.Normalized();
		break;

	case ShapeType_SPHERE_CENTER:
		startValues.particleDirection = sphereCreation.RandomPointInside(App->randomMathLCG).Normalized();
		break;

	case ShapeType_SPHERE_BORDER:
		spawn = sphereCreation.RandomPointOnSurface(App->randomMathLCG);
		startValues.particleDirection = spawn.Normalized();
		break;

	case ShapeType_CONE:
	{
		float angle = 0.0f;
		float centerDist = 0.0f;

		angle = (2 * PI) * (float)App->GenerateRandomNumber() / MAXUINT;
		centerDist = (float)App->GenerateRandomNumber() / MAXUINT;

		circleCreation.pos = math::float3(0, coneHeight, 0);
		if (!localSpace)
			circleCreation.pos = circleCreation.pos * parent->transform->GetGlobalMatrix().RotatePart().Transposed();

		startValues.particleDirection = (circleCreation.GetPoint(angle, centerDist)).Normalized();
		break;
	}
	case ShapeType_MESH:
	{
		if (isBurst)
		{
			if (!burstMesh.uniqueVertex.empty())
			{
				spawn.x = burstMesh.uniqueVertex[burstMesh.meshVertexCont].x;
				spawn.y = burstMesh.uniqueVertex[burstMesh.meshVertexCont].y;
				spawn.z = burstMesh.uniqueVertex[burstMesh.meshVertexCont].z;
				burstMesh.meshVertexCont++;
				if (burstMesh.meshVertexCont >= burstMesh.uniqueVertex.size())
					burstMesh.meshVertexCont = 0u;
			}
		}
		else if (!shapeMesh.uniqueVertex.empty())
		{
			uint pos = rand() % shapeMesh.uniqueVertex.size();
			spawn.x = shapeMesh.uniqueVertex[pos].x;
			spawn.y = shapeMesh.uniqueVertex[pos].y;
			spawn.z = shapeMesh.uniqueVertex[pos].z;
		}
		startValues.particleDirection = (math::float3::unitY * parent->transform->GetGlobalMatrix().RotatePart().Transposed()).Normalized();

		if (!localSpace)
			spawn = spawn * parent->transform->GetGlobalMatrix().RotatePart().Transposed();
		break;
	}
	default:
		break;
	}

	math::float3 global = math::float3::zero;
	if (!localSpace && parent)
	{
		math::float4x4 trans = parent->transform->GetGlobalMatrix();
		math::Quat identity = math::Quat::identity;
		math::float3 zero = math::float3::zero;

		trans.Decompose(global, identity, zero);

		spawn = trans.Mul(math::float4(spawn, 0)).xyz();
	}
	return spawn;
}

void ComponentEmitter::OnUniqueEditor()
{
#ifndef GAMEMODE
	ImGui::Text("Particle System");
	ImGui::Spacing();

	ParticleValues();

	ParticleShape();

	ParticleColor();

	ParticleBurst();

	ParticleAABB();

	ParticleTexture();

	ParticleSubEmitter();

	ParticleSpace();


#endif
}

void ComponentEmitter::ParticleValues()
{
#ifndef GAMEMODE
	if (ImGui::CollapsingHeader("Particle Values", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::ShowHelpMarker("Active checkBox if you want a random number");

		if (ImGui::Checkbox("##Speed", &checkSpeed))
			EqualsMinMaxValues(startValues.speed);
		ShowFloatValue(startValues.speed, checkSpeed, "Speed", 0.25f, 0.25f, 20.0f);

		if (ImGui::Checkbox("##Rotation", &checkRotation))
			EqualsMinMaxValues(startValues.rotation);
		ShowFloatValue(startValues.rotation, checkRotation, "Initial Rotation", 0.25f, -360.0f, 360.0f);

		if (ImGui::Checkbox("##AngularVelocity", &checkAngularVelocity))
			EqualsMinMaxValues(startValues.angularVelocity);
		ShowFloatValue(startValues.angularVelocity, checkAngularVelocity, "Angular Velocity", 0.25f, -45.0f, 45.0f);

		if (ImGui::Checkbox("##AngularAcceleration", &checkAngularAcceleration))
			EqualsMinMaxValues(startValues.angularAcceleration);
		ShowFloatValue(startValues.angularAcceleration, checkAngularAcceleration, "Angular Acceleration", 0.25f, -45.0f, 45.0f);

		if (ImGui::Checkbox("##Lifetime", &checkLife))
			EqualsMinMaxValues(startValues.life);
		ShowFloatValue(startValues.life, checkLife, "Lifetime", 0.5f, 1.0f, 20.0f);

		if (ImGui::Checkbox("##Size", &checkSize))
			EqualsMinMaxValues(startValues.size);
		ShowFloatValue(startValues.size, checkSize, "Size", 0.1f, 0.1f, 5.0f);

		if (ImGui::Checkbox("##SizeOverTime", &checkSizeOverTime))
			EqualsMinMaxValues(startValues.sizeOverTime);
		ShowFloatValue(startValues.sizeOverTime, checkSizeOverTime, "SizeOverTime", 0.25f, -1.0f, 1.0f);

		if (ImGui::Checkbox("##Acceleration", &checkAcceleration))
			EqualsMinMaxValues(startValues.acceleration);
		ShowFloatValue(startValues.acceleration, checkAcceleration, "Acceleration", 0.25f, -1.0f, 1.0f);

		ImGui::PushItemWidth(127.0f);
		ImGui::DragFloat3("Gravity", &startValues.gravity.x, 0.1f, -5.0f, 5.0f, "%.2f");
		ImGui::PopItemWidth();

		ImGui::PushItemWidth(100.0f);
		ImGui::DragInt("Emition", &rateOverTime, 1.0f, 0.0f, 300.0f, "%.2f");

		ImGui::Separator();
		ImGui::Text("Particle Mesh");
		ImGui::PushID("particleMesh");
		ImGui::Button(std::to_string(uuidMeshPart).data(), ImVec2(150.0f, 0.0f));
		ImGui::PopID();

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MESH_INSPECTOR_SELECTOR"))
			{
				uint payload_n = *(uint*)payload->Data;
				SetUuidRes(payload_n, uuidMeshPart);
				isPlane = false;
			}
			ImGui::EndDragDropTarget();
		}
		ImGui::SameLine();
		if (ImGui::Button("Set plane mesh", ImVec2(125.0f, 25.0f)))
		{
			SetUuidRes(App->resHandler->plane, uuidMeshPart);
			isPlane = true;
		}

		ImGui::Separator();
		if (ImGui::Checkbox("Loop", &loop))
			loopTimer.Start();
		ImGui::DragFloat("Duration", &duration, 0.5f, 0.5f, 20.0f, "%.2f");
		ImGui::Separator();

		ImGui::Checkbox("Start on play", &startOnPlay);
	}
#endif
}

void ComponentEmitter::ParticleShape()
{
#ifndef GAMEMODE
	if (ImGui::CollapsingHeader("Particle Shape"))
	{
		ImGui::Separator();
		if (ImGui::BeginMenu("Change Shape"))
		{
			if (ImGui::MenuItem("Box"))
				normalShapeType = ShapeType_BOX;
			else if (ImGui::MenuItem("Sphere"))
				normalShapeType = ShapeType_SPHERE;
			else if (ImGui::MenuItem("Cone"))
				normalShapeType = ShapeType_CONE;
			else if (ImGui::MenuItem("Mesh"))
				normalShapeType = ShapeType_MESH;

			ImGui::End();
		}

		math::float3 pos;
		switch (normalShapeType)
		{
		case ShapeType_BOX:
			ImGui::Text("Box");
			pos = boxCreation.Size();
			ImGui::DragFloat3("Box Size", &pos.x, 0.1f, 0.1f, 20.0f, "%.2f");

			boxCreation.SetFromCenterAndSize(boxCreation.CenterPoint(), pos);

			break;
		case ShapeType_SPHERE:
		case ShapeType_SPHERE_BORDER:
		case ShapeType_SPHERE_CENTER:
			ImGui::Text("Sphere");

			ImGui::Text("Particle emision from:");

			if (ImGui::RadioButton("Random", normalShapeType == ShapeType_SPHERE))
				normalShapeType = ShapeType_SPHERE;
			ImGui::SameLine();
			if (ImGui::RadioButton("Center", normalShapeType == ShapeType_SPHERE_CENTER))
				normalShapeType = ShapeType_SPHERE_CENTER;
			ImGui::SameLine();
			if (ImGui::RadioButton("Border", normalShapeType == ShapeType_SPHERE_BORDER))
				normalShapeType = ShapeType_SPHERE_BORDER;

			ImGui::DragFloat("Sphere Size", &sphereCreation.r, 0.25f, 1.0f, 20.0f, "%.2f");

			break;
		case ShapeType_CONE:
			ImGui::Text("Cone");
			ImGui::DragFloat("Sphere Size", &circleCreation.r, 0.25f, 0.25f, 20.0f, "%.2f");
			ImGui::DragFloat("Height Cone", &coneHeight, 0.0f, 0.25f, 20.0f, "%.2f");

			break;
		case ShapeType_MESH:
			ImGui::PushID("shapeMesh");
			ImGui::Button(std::to_string(shapeMesh.uuid).data(), ImVec2(150.0f, 0.0f));
			ImGui::PopID();

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MESH_INSPECTOR_SELECTOR"))
				{
					uint payload_n = *(uint*)payload->Data;
					SetMeshParticleRes(payload_n);
				}
				ImGui::EndDragDropTarget();
			}
			break;
		default:
			break;
		}
		ImGui::Checkbox("Debug Draw", &drawShape);
	}
#endif
}

void ComponentEmitter::ParticleColor()
{
#ifndef GAMEMODE
	if (ImGui::CollapsingHeader("Particle Color"))

	{
		ImGui::Text("Particle Color");
		ImGui::SameLine();
		ImGui::ShowHelpMarker("Click color square for change it");
		std::vector<ColorTime> deleteColor;
		std::list<ColorTime>::iterator iter = startValues.color.begin();
		uint posList = 0u;
		while (iter != startValues.color.end())
		{
			//TODO: they must be able to change position
			if ((iter) == startValues.color.begin())
			{//Cant delete 1st color
				ImGui::PushItemWidth(150.0f);
				if (!EditColor(*iter))
					break;
				iter++;
			}
			else
			{
				if (!EditColor(*iter, posList))
					startValues.color.erase(iter++);
				else
					iter++;
			}
			++posList;
		}
		ImGui::Separator();
		ImGui::Checkbox("Color time", &startValues.timeColor);
		if (startValues.timeColor)
		{

			ImGui::DragInt("Position", &nextPos, 1.0f, 1, 100);
			ImGui::ColorPicker4("", &nextColor.x, ImGuiColorEditFlags_AlphaBar);
			if (ImGui::Button("Add Color", ImVec2(125, 25)))
			{
				ColorTime colorTime;
				colorTime.color = nextColor;
				colorTime.position = (float)nextPos / 100;
				colorTime.name = std::to_string((int)nextPos) + "%";
				startValues.color.push_back(colorTime);
				startValues.color.sort();
			}
		}
	}
#endif
}

void ComponentEmitter::ParticleBurst()
{
#ifndef GAMEMODE
	if (ImGui::CollapsingHeader("Particle Burst"))
	{
		ImGui::Checkbox("Burst", &burst);
		if (ImGui::BeginMenu(burstTypeName.data()))
		{
			if (ImGui::MenuItem("Box"))
				burstType = ShapeType_BOX;
			else if (ImGui::MenuItem("Sphere"))
				burstType = ShapeType_SPHERE_CENTER;
			else if (ImGui::MenuItem("Cone"))
				burstType = ShapeType_CONE;
			else if (ImGui::MenuItem("Mesh"))
				burstType = ShapeType_MESH;

			ImGui::End();
			SetBurstText();
		}

		if (burstType == ShapeType_SPHERE_CENTER || burstType == ShapeType_SPHERE || burstType == ShapeType_SPHERE_BORDER)
		{
			if (ImGui::RadioButton("Random", burstType == ShapeType_SPHERE))
				burstType = ShapeType_SPHERE;
			ImGui::SameLine();
			if (ImGui::RadioButton("Center", burstType == ShapeType_SPHERE_CENTER))
				burstType = ShapeType_SPHERE_CENTER;
			ImGui::SameLine();
			if (ImGui::RadioButton("Border", burstType == ShapeType_SPHERE_BORDER))
				burstType = ShapeType_SPHERE_BORDER;
		}

		ImGui::PushItemWidth(100.0f);
		ImGui::DragInt("Min particles", &minPart, 1.0f, 0, 100);
		if (minPart > maxPart)
			maxPart = minPart;
		ImGui::DragInt("Max Particles", &maxPart, 1.0f, 0, 100);
		if (maxPart < minPart)
			minPart = maxPart;
		ImGui::DragFloat("Repeat Time", &repeatTime, 0.5f, 0.0f, 50.0f, "%.1f");

		if (burstType == ShapeType_MESH)
		{
			ImGui::PushID("BurstMesh");
			ImGui::Button(std::to_string(burstMesh.uuid).data(), ImVec2(150.0f, 0.0f));
			ImGui::PopID();

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MESH_INSPECTOR_SELECTOR"))
				{
					uint payload_n = *(uint*)payload->Data;
					SetBurstMeshParticleRes(payload_n);
				}
				ImGui::EndDragDropTarget();
			}
		}
		ImGui::Separator();
	}
#endif
}

void ComponentEmitter::SetBurstText()
{
	switch (burstType)
	{
	case ShapeType_BOX:
		burstTypeName = "Box Burst";
		break;
	case ShapeType_SPHERE:
	case ShapeType_SPHERE_CENTER:
	case ShapeType_SPHERE_BORDER:
		burstTypeName = "Sphere Burst";
		break;
	case ShapeType_CONE:
		burstTypeName = "Cone Burst";
		break;
	case ShapeType_MESH:
		burstTypeName = "Mesh Burst";
		break;
	default:
		break;
	}
}

void ComponentEmitter::ParticleAABB()
{
#ifndef GAMEMODE

	if (ImGui::CollapsingHeader("Particle BoundingBox"))
	{
		ImGui::Checkbox("Bounding Box", &drawAABB);
		if (drawAABB)
		{
			math::float3 size = boundingBox.Size();
			math::float3 pos = boundingBox.CenterPoint();
			if (ImGui::DragFloat3("Dimensions", &size.x, 1.0f, 0.0f, 0.0f, "%.0f"))
				boundingBox.SetFromCenterAndSize(pos, size);
			if (ImGui::DragFloat3("Pos", &pos.x, 1.0f, 0.0f, 0.0f, "%.0f"))
				boundingBox.SetFromCenterAndSize(pos, size);
		}
	}
#endif
}

void ComponentEmitter::UpdateTransform()
{
	math::float3 size = boundingBox.Size();
	math::float3 pos = parent->transform->GetGlobalMatrix().TranslatePart();
	boundingBox.SetFromCenterAndSize(pos, size);
}

void ComponentEmitter::ParticleTexture()
{
#ifndef GAMEMODE
	if (ImGui::CollapsingHeader("Particle Texture", ImGuiTreeNodeFlags_FramePadding))
	{
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
				SetUuidRes(payload_n, materialRes);
			}
			ImGui::EndDragDropTarget();
		}

		if (ImGui::SmallButton("Use default material"))
			SetUuidRes(App->resHandler->defaultMaterial, materialRes);
		ImGui::Separator();

		if (ImGui::Checkbox("Animated sprite", &particleAnim.isParticleAnimated))
		{
			if (!particleAnim.isParticleAnimated)
			{
				particleAnim.textureRows = 1;
				particleAnim.textureColumns = 1;
				dieOnAnimation = false;
				SetNewAnimation();
			}
			else
				SetNewAnimation();
		}
		if (particleAnim.isParticleAnimated)
		{
			if (ImGui::Checkbox("##AnimationSpeed", &checkAnimationSpeed))
				EqualsMinMaxValues(startValues.acceleration);
			ShowFloatValue(particleAnim.animationSpeed, checkAnimationSpeed, "Animation Speed", 0.001, 0.0f, 5.0f);

			if (ImGui::DragInt("Rows", &particleAnim.textureRows, 1, 1, 10))
				particleAnim.textureRowsNorm = 1.0f / particleAnim.textureRows;
			if (ImGui::DragInt("Columns", &particleAnim.textureColumns, 1, 1, 10))
				particleAnim.textureColumnsNorm = 1.0f / particleAnim.textureColumns;

			ImGui::Checkbox("Kill particle with animation", &dieOnAnimation);
			ImGui::Checkbox("Random Starting Frame", &particleAnim.randAnim);
			if (dieOnAnimation)
			{
				checkLife = false;
				startValues.life.x = particleAnim.animationSpeed.x * (particleAnim.textureColumns * particleAnim.textureRows - 1);
			}
			if (ImGui::Button("Instant Animation", ImVec2(150.0f, 25.0f)))
			{
				SetNewAnimation();
			}
		}
		ImGui::Separator();
	}
#endif
}

void ComponentEmitter::SetNewAnimation()
{
	particleAnim.textureColumnsNorm = 1.0f / particleAnim.textureColumns;
	particleAnim.textureRowsNorm = 1.0f / particleAnim.textureRows;
	for (std::list<Particle*>::iterator iterator = particles.begin(); iterator != particles.end(); ++iterator)
	{
		(*iterator)->ChangeAnim(particleAnim);
	}
}

void ComponentEmitter::ParticleSubEmitter()
{
#ifndef GAMEMODE
	if (ImGui::Checkbox("SubEmitter", &startValues.subEmitterActive))
	{
		if (startValues.subEmitterActive)
		{
			if (subEmitter)
				subEmitter->ToggleIsActive();
			else
			{
				subEmitter = App->GOs->CreateGameObject("SubEmition", parent);
				subEmitter->AddComponent(EmitterComponent);
				((ComponentEmitter*)subEmitter->GetComponent(EmitterComponent))->isSubEmitter = true;
			}
		}
		else
			subEmitter->ToggleIsActive();
	}
	ImGui::Separator();
#endif
}

void ComponentEmitter::ParticleSpace()
{
#ifndef GAMEMODE

	ImGui::Checkbox("Local space", &localSpace);

	ImGui::Separator();
#endif
}

void ComponentEmitter::ShowFloatValue(math::float2& value, bool checkBox, const char* name, float v_speed, float v_min, float v_max)
{
#ifndef GAMEMODE
	ImGui::SameLine();
	if (checkBox)
	{
		ImGui::PushItemWidth(42.0f);
		std::string str = "##";
		str.append(name);
		str.append("min");
		if (ImGui::DragFloat(str.data(), &value.x, v_speed, v_min, v_max, "%.2f"))
			CheckMinMax(value);
		ImGui::SameLine();
		if (ImGui::DragFloat(name, &value.y, v_speed, v_min, v_max, "%.2f"))
			CheckMinMax(value);
	}
	else
	{
		ImGui::PushItemWidth(100.0f);
		if (ImGui::DragFloat(name, &value.x, v_speed, v_min, v_max, "%.2f"))
			value.y = value.x;
	}
	ImGui::PopItemWidth();
#endif
}

void ComponentEmitter::EqualsMinMaxValues(math::float2 & value)
{
#ifndef GAMEMODE
	if (value[1] != value[0])
		value[1] = value[0];
#endif
}

void ComponentEmitter::CheckMinMax(math::float2& value)
{
	if (value.x > value.y)
		value.y = value.x;
}

bool ComponentEmitter::EditColor(ColorTime &colorTime, uint pos)
{
	bool ret = true;

#ifndef GAMEMODE

	ImVec4 color = EqualsFloat4(colorTime.color);
	if (ImGui::ColorButton(colorTime.name.data(), color, ImGuiColorEditFlags_None, ImVec2(100, 20)))
		colorTime.changingColor = !colorTime.changingColor;

	if (!colorTime.changingColor)
	{
		ImGui::SameLine();
		ImGui::TextUnformatted(colorTime.name.data());
		if (pos > 0)
		{
			std::string colorStr = "Remove Color ";
			colorStr.append(std::to_string(pos));
			ImGui::SameLine();
			if (ImGui::Button(colorStr.data(), ImVec2(125, 25)))
				ret = false;
		}
		else if (!startValues.timeColor)
			ret = false;
	}
	else
		ImGui::ColorEdit4(colorTime.name.data(), &colorTime.color.x, ImGuiColorEditFlags_AlphaBar);

#endif
	return ret;
}

void ComponentEmitter::SetUuidRes(uint newUuid, uint &oldUuid)
{
	if (oldUuid > 0)
		App->res->SetAsUnused(oldUuid);

	if (newUuid > 0)
		App->res->SetAsUsed(newUuid);

	oldUuid = newUuid;
}

void ComponentEmitter::SetMeshParticleRes(uint res_uuid)
{
	if (shapeMesh.uuid > 0)
		App->res->SetAsUnused(shapeMesh.uuid);

	if (res_uuid > 0)
	{
		App->res->SetAsUsed(res_uuid);
		Resource* resource = App->res->GetResource(res_uuid);

		if (resource)
			SetMeshInfo((ResourceMesh*)resource, shapeMesh);
	}
	else
		shapeMesh.uuid = 0u;
}

void ComponentEmitter::SetBurstMeshParticleRes(uint res_uuid)
{
	if (burstMesh.uuid > 0)
		App->res->SetAsUnused(burstMesh.uuid);

	if (res_uuid > 0)
	{
		App->res->SetAsUsed(res_uuid);
		Resource* resource = App->res->GetResource(res_uuid);

		if (resource)
			SetMeshInfo((ResourceMesh*)resource, burstMesh);
	}
	else
		burstMesh.uuid = 0u;
}
void ComponentEmitter::SetMeshInfo(ResourceMesh* resource, MeshShape &shape)
{
	resource->GetVerticesReference(shape.meshVertex);
	shape.uniqueVertex.clear();
	resource->GetUniqueVertexPositions(shape.uniqueVertex);

	shape.uuid = resource->GetUuid();
	shape.meshVertexCont = 0u;
	resource->GetIndicesReference(shape.indices);
	shape.indicesSize = resource->GetIndicesCount() / 3;
}
uint ComponentEmitter::GetMaterialRes() const
{
	return materialRes;
}

#ifndef GAMEMODE
ImVec4 ComponentEmitter::EqualsFloat4(const math::float4 float4D)
{
	ImVec4 vec;
	vec.x = float4D.x;
	vec.y = float4D.y;
	vec.z = float4D.z;
	vec.w = float4D.w;
	return vec;
}
#endif

int ComponentEmitter::GetEmition() const
{
	return rateOverTime;
}

void ComponentEmitter::UpdateParticleTrans(ParticleTrans& trans)
{
	if (localSpace)
	{
		math::Quat tempRot;
		math::float3 tempScale;

		parent->transform->GetGlobalMatrix().Mul(trans.GetSpaceMatrix()).Decompose(trans.position, tempRot, tempScale);
	}
}

uint ComponentEmitter::GetInternalSerializationBytes()
{
	uint sizeOfList = 0u;
	for (std::list<ColorTime>::iterator it = startValues.color.begin(); it != startValues.color.end(); ++it)
	{
		sizeOfList += (*it).GetColorListSerializationBytes();
	}

	return sizeof(bool) * 18 + sizeof(int) * 3 + sizeof(float) * 5 + sizeof(uint) * 6
		+ sizeof(ShapeType) * 2 + sizeof(math::AABB) * 2 + sizeof(math::float2) * 8 + sizeof(math::float3) * 2
		+ particleAnim.GetPartAnimationSerializationBytes() + sizeOfList;//Bytes of all Start Values Struct
}


void ComponentEmitter::OnInternalSave(char *& cursor)
{
	startValues.OnInternalSave(cursor);

	size_t bytes = sizeof(bool);
	memcpy(cursor, &checkLife, bytes);
	cursor += bytes;

	memcpy(cursor, &checkSpeed, bytes);
	cursor += bytes;

	memcpy(cursor, &checkAcceleration, bytes);
	cursor += bytes;

	memcpy(cursor, &checkSizeOverTime, bytes);
	cursor += bytes;

	memcpy(cursor, &checkSize, bytes);
	cursor += bytes;

	memcpy(cursor, &checkRotation, bytes);
	cursor += bytes;

	memcpy(cursor, &checkAngularAcceleration, bytes);
	cursor += bytes;

	memcpy(cursor, &checkAngularVelocity, bytes);
	cursor += bytes;

	memcpy(cursor, &checkAnimationSpeed, bytes);
	cursor += bytes;

	memcpy(cursor, &drawAABB, bytes);
	cursor += bytes;

	memcpy(cursor, &isSubEmitter, bytes);
	cursor += bytes;

	memcpy(cursor, &dieOnAnimation, bytes);
	cursor += bytes;

	memcpy(cursor, &loop, bytes);
	cursor += bytes;

	memcpy(cursor, &burst, bytes);
	cursor += bytes;

	memcpy(cursor, &startOnPlay, bytes);
	cursor += bytes;

	memcpy(cursor, &localSpace, bytes);
	cursor += bytes;

	bytes = sizeof(int);
	memcpy(cursor, &rateOverTime, bytes);
	cursor += bytes;

	memcpy(cursor, &minPart, bytes);
	cursor += bytes;

	memcpy(cursor, &maxPart, bytes);
	cursor += bytes;

	bytes = sizeof(float);
	memcpy(cursor, &duration, bytes);
	cursor += bytes;

	memcpy(cursor, &repeatTime, bytes);
	cursor += bytes;

	memcpy(cursor, &circleCreation.r, bytes);
	cursor += bytes;

	memcpy(cursor, &sphereCreation.r, bytes);
	cursor += bytes;

	memcpy(cursor, &coneHeight, bytes);
	cursor += bytes;

	uint uuid = 0u;
	if (subEmitter)
		uuid = subEmitter->GetUUID();

	bytes = sizeof(uint);
	memcpy(cursor, &uuid, bytes);
	cursor += bytes;

	memcpy(cursor, &materialRes, bytes);
	cursor += bytes;

	memcpy(cursor, &burstMesh.uuid, bytes);
	cursor += bytes;

	memcpy(cursor, &shapeMesh.uuid, bytes);
	cursor += bytes;

	memcpy(cursor, &uuidMeshPart, bytes);
	cursor += bytes;

	particleAnim.OnInternalSave(cursor);

	bytes = sizeof(ShapeType);
	memcpy(cursor, &normalShapeType, bytes);
	cursor += bytes;

	memcpy(cursor, &burstType, bytes);
	cursor += bytes;

	bytes = sizeof(math::AABB);
	memcpy(cursor, &boxCreation, bytes);
	cursor += bytes;

	memcpy(cursor, &boundingBox, bytes);
	cursor += bytes;
}

void ComponentEmitter::OnInternalLoad(char *& cursor)
{
	startValues.OnInternalLoad(cursor);

	size_t bytes = sizeof(bool);
	memcpy(&checkLife, cursor, bytes);
	cursor += bytes;

	memcpy(&checkSpeed, cursor, bytes);
	cursor += bytes;

	memcpy(&checkAcceleration, cursor, bytes);
	cursor += bytes;

	memcpy(&checkSizeOverTime, cursor, bytes);
	cursor += bytes;

	memcpy(&checkSize, cursor, bytes);
	cursor += bytes;

	memcpy(&checkRotation, cursor, bytes);
	cursor += bytes;

	memcpy(&checkAngularAcceleration, cursor, bytes);
	cursor += bytes;

	memcpy(&checkAngularVelocity, cursor, bytes);
	cursor += bytes;

	memcpy(&checkAnimationSpeed, cursor, bytes);
	cursor += bytes;

	memcpy(&drawAABB, cursor, bytes);
	cursor += bytes;

	memcpy(&isSubEmitter, cursor, bytes);
	cursor += bytes;

	memcpy(&dieOnAnimation, cursor, bytes);
	cursor += bytes;

	memcpy(&loop, cursor, bytes);
	cursor += bytes;

	memcpy(&burst, cursor, bytes);
	cursor += bytes;

	memcpy(&startOnPlay, cursor, bytes);
	cursor += bytes;

	memcpy(&localSpace, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(int);
	memcpy(&rateOverTime, cursor, bytes);
	cursor += bytes;

	memcpy(&minPart, cursor, bytes);
	cursor += bytes;

	memcpy(&maxPart, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(float);
	memcpy(&duration, cursor, bytes);
	cursor += bytes;

	memcpy(&repeatTime, cursor, bytes);
	cursor += bytes;

	memcpy(&circleCreation.r, cursor, bytes);
	cursor += bytes;

	memcpy(&sphereCreation.r, cursor, bytes);
	cursor += bytes;

	memcpy(&coneHeight, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(uint);
	memcpy(&uuidSubEmitter, cursor, bytes);
	cursor += bytes;

	uint uuidMaterial;
	memcpy(&uuidMaterial, cursor, bytes);

	App->res->GetResource(uuidMaterial) ? SetUuidRes(uuidMaterial, materialRes) : SetUuidRes(App->resHandler->defaultMaterial, materialRes);
	cursor += bytes;

	memcpy(&burstMesh.uuid, cursor, bytes);
	Resource* res = App->res->GetResource(burstMesh.uuid);
	if (res)
		SetBurstMeshParticleRes(burstMesh.uuid);
	cursor += bytes;

	memcpy(&shapeMesh.uuid, cursor, bytes);
	res = App->res->GetResource(shapeMesh.uuid);
	if (res)
		SetMeshParticleRes(shapeMesh.uuid);
	cursor += bytes;

	uint uuidRes;
	memcpy(&uuidRes, cursor, bytes);
	uuidRes > 0 ? SetUuidRes(uuidRes, uuidMeshPart) : SetUuidRes(App->resHandler->plane, uuidMeshPart);
	if (uuidRes != App->resHandler->plane)
		isPlane = false;

	cursor += bytes;

	particleAnim.OnInternalLoad(cursor);

	bytes = sizeof(ShapeType);
	memcpy(&normalShapeType, cursor, bytes);
	cursor += bytes;

	memcpy(&burstType, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(math::AABB);
	memcpy(&boxCreation, cursor, bytes);
	cursor += bytes;

	memcpy(&boundingBox, cursor, bytes);
	cursor += bytes;

	SetBurstText();
}
//--------------------------------------------------------------------------------------------------------------------------------------
//Start Values Save&Load
void StartValues::OnInternalSave(char *& cursor)
{
	size_t bytes = sizeof(math::float2);
	memcpy(cursor, &life, bytes);
	cursor += bytes;

	memcpy(cursor, &speed, bytes);
	cursor += bytes;

	memcpy(cursor, &size, bytes);
	cursor += bytes;

	memcpy(cursor, &sizeOverTime, bytes);
	cursor += bytes;

	memcpy(cursor, &rotation, bytes);
	cursor += bytes;

	memcpy(cursor, &angularAcceleration, bytes);
	cursor += bytes;

	memcpy(cursor, &angularVelocity, bytes);
	cursor += bytes;

	memcpy(cursor, &acceleration, bytes);
	cursor += bytes;

	bytes = sizeof(math::float3);
	memcpy(cursor, &gravity, bytes);
	cursor += bytes;

	memcpy(cursor, &particleDirection, bytes);
	cursor += bytes;

	bytes = sizeof(bool);
	memcpy(cursor, &timeColor, bytes);
	cursor += bytes;

	memcpy(cursor, &subEmitterActive, bytes);
	cursor += bytes;

	bytes = sizeof(uint);
	uint listSize = color.size();
	memcpy(cursor, &listSize, bytes);
	cursor += bytes;

	for (std::list<ColorTime>::iterator it = color.begin(); it != color.end(); ++it)
	{
		(*it).OnInternalSave(cursor);
	}

}

void StartValues::OnInternalLoad(char *& cursor)
{
	size_t bytes = sizeof(math::float2);
	memcpy(&life, cursor, bytes);
	cursor += bytes;

	memcpy(&speed, cursor, bytes);
	cursor += bytes;

	memcpy(&size, cursor, bytes);
	cursor += bytes;

	memcpy(&sizeOverTime, cursor, bytes);
	cursor += bytes;

	memcpy(&rotation, cursor, bytes);
	cursor += bytes;

	memcpy(&angularAcceleration, cursor, bytes);
	cursor += bytes;

	memcpy(&angularVelocity, cursor, bytes);
	cursor += bytes;

	memcpy(&acceleration, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(math::float3);
	memcpy(&gravity, cursor, bytes);
	cursor += bytes;

	memcpy(&particleDirection, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(bool);
	memcpy(&timeColor, cursor, bytes);
	cursor += bytes;

	memcpy(&subEmitterActive, cursor, bytes);
	cursor += bytes;

	uint listSize;
	bytes = sizeof(uint);
	memcpy(&listSize, cursor, bytes);
	cursor += bytes;

	color.pop_back();//Pop started value
	for (int i = 0; i < listSize; ++i)
	{
		ColorTime colorTime;
		colorTime.OnInternalLoad(cursor);
		color.push_back(colorTime);
	}
}

void StartValues::operator=(StartValues startValue)
{
	life = startValue.life;
	speed = startValue.speed;
	gravity = startValue.gravity;
	acceleration = startValue.acceleration;
	sizeOverTime = startValue.sizeOverTime;
	size = startValue.size;
	rotation = startValue.rotation;
	angularAcceleration = startValue.angularAcceleration;
	angularVelocity = startValue.angularVelocity;

	timeColor = startValue.timeColor;
	subEmitterActive = startValue.subEmitterActive;

	particleDirection = startValue.particleDirection;

	color.clear();
	for (std::list<ColorTime>::iterator it = startValue.color.begin(); it != startValue.color.end(); ++it)
	{
		color.push_back(*it);
	}
}

//--------------------------------------------------------------------------------------------------------------------------------------
//COLOR TIME Save&Load

uint ColorTime::GetColorListSerializationBytes()
{
	return sizeof(bool) + sizeof(float) + sizeof(math::float4) + sizeof(char) * name.size() + sizeof(uint);//Size of name
}

void ColorTime::OnInternalSave(char* &cursor)
{
	size_t bytes = sizeof(uint);
	uint nameLenght = name.size();
	memcpy(cursor, &nameLenght, bytes);
	cursor += bytes;

	bytes = nameLenght;
	memcpy(cursor, name.c_str(), bytes);
	cursor += bytes;

	bytes = sizeof(bool);
	memcpy(cursor, &changingColor, bytes);
	cursor += bytes;

	bytes = sizeof(float);
	memcpy(cursor, &position, bytes);
	cursor += bytes;

	bytes = sizeof(math::float4);
	memcpy(cursor, &color, bytes);
	cursor += bytes;

}

void ColorTime::OnInternalLoad(char *& cursor)
{
	//Load lenght + string
	size_t bytes = sizeof(uint);
	uint nameLenght;
	memcpy(&nameLenght, cursor, bytes);
	cursor += bytes;

	bytes = nameLenght;
	name.resize(nameLenght);
	memcpy((void*)name.c_str(), cursor, bytes);
	name.resize(nameLenght);
	cursor += bytes;

	bytes = sizeof(bool);
	memcpy(&changingColor, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(float);
	memcpy(&position, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(math::float4);
	memcpy(&color, cursor, bytes);
	cursor += bytes;

}

//--------------------------------------------------------------------------------------------------------------------------------------
//ParticleAnimation Save&Load
void ParticleAnimation::OnInternalSave(char *& cursor)
{
	size_t bytes = sizeof(bool);
	memcpy(cursor, &isParticleAnimated, bytes);
	cursor += bytes;

	memcpy(cursor, &randAnim, bytes);
	cursor += bytes;

	bytes = sizeof(int);
	memcpy(cursor, &textureRows, bytes);
	cursor += bytes;

	memcpy(cursor, &textureColumns, bytes);
	cursor += bytes;

	bytes = sizeof(float);
	memcpy(cursor, &textureRowsNorm, bytes);
	cursor += bytes;

	memcpy(cursor, &textureColumnsNorm, bytes);
	cursor += bytes;

	bytes = sizeof(math::float2);
	memcpy(cursor, &animationSpeed, bytes);
	cursor += bytes;
}

void ParticleAnimation::OnInternalLoad(char *& cursor)
{
	size_t bytes = sizeof(bool);
	memcpy(&isParticleAnimated, cursor, bytes);
	cursor += bytes;

	memcpy(&randAnim, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(int);
	memcpy(&textureRows, cursor, bytes);
	cursor += bytes;

	memcpy(&textureColumns, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(float);
	memcpy(&textureRowsNorm, cursor, bytes);
	cursor += bytes;

	memcpy(&textureColumnsNorm, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(math::float2);
	memcpy(&animationSpeed, cursor, bytes);
	cursor += bytes;
}
