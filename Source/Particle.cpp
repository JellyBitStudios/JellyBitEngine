#include "Application.h"
#include "Particle.h"
#include "Primitive.h"
#include "ComponentEmitter.h"
#include "ModuleParticles.h"
#include "ModuleResourceManager.h"
#include "ModuleInternalResHandler.h"
#include "ModuleRenderer3D.h"
#include "ShaderImporter.h"
#include "SceneImporter.h"
#include "ResourceMesh.h"
#include "MaterialImporter.h"
#include "ComponentMaterial.h"
#include "ResourceShaderProgram.h"
#include "ResourceMaterial.h"
#include "Uniforms.h"
#include "GLCache.h"

#include "MathGeoLib/include/Math/Quat.h"
#include "MathGeoLib/include/Math/float3.h"

#include <vector>

Particle::Particle(math::float3 pos, StartValues data)
{
}

Particle::Particle()
{
}

Particle::~Particle()
{
}

void Particle::SetActive(math::float3 pos, StartValues data, ParticleAnimation partAnim)
{
	color.clear();

	lifeTime = CreateRandomNum(data.life);

	life = 0.0f;

	speed = CreateRandomNum(data.speed);
	gravity = data.gravity;
	acceleration = CreateRandomNum(data.acceleration);
	direction = data.particleDirection;

	angle = CreateRandomNum(data.rotation) * DEGTORAD;
	angularVelocity = CreateRandomNum(data.angularVelocity) * DEGTORAD;
	angularAcceleration = CreateRandomNum(data.angularAcceleration) * DEGTORAD;

	sizeOverTime = CreateRandomNum(data.sizeOverTime);

	_movement = pos;
	transform.rotation = math::Quat::FromEulerXYZ(0, 0, 0); //Start rotation
	transform.scale = math::float3::one * CreateRandomNum(data.size);

	for (std::list<ColorTime>::iterator iter = data.color.begin(); iter != data.color.end(); ++iter)
		color.push_back(*iter);

	currentColor = (*color.begin()).color;

	multicolor = data.timeColor;

	animationTime = 0.0f;

	isParticleAnimated = partAnim.isParticleAnimated;
	textureRows = partAnim.textureRows;
	textureColumns = partAnim.textureColumns;
	textureRowsNorm = partAnim.textureRowsNorm;
	textureColumnsNorm = partAnim.textureColumnsNorm;
	animationSpeed = CreateRandomNum(partAnim.animationSpeed);

	if (partAnim.randAnim)
		currentFrame = rand() % (textureColumns * textureRows);
	else
		currentFrame = 0u;

	contFrame = 0u;
	currMinUVCoord.x = (currentFrame % textureColumns) * textureColumnsNorm;
	currMinUVCoord.y = ((textureRows - 1) - (currentFrame / textureColumns)) * textureRowsNorm;


	active = true;
	subEmitterActive = data.subEmitterActive;
	index = 0;

	App->particle->activeParticles++;
}

bool Particle::Update(float dt)
{
	if (owner->simulatedGame == SimulatedGame_PAUSE || App->IsPause())
		dt = 0;
	life += dt;
	if (life < lifeTime)
	{
		gravity += gravity * dt;
		speed += acceleration * dt;
		math::float3 movement = direction * (speed * dt);
		_movement += movement + gravity * dt;

		transform.position = _movement;

		if (owner->isPlane)
			LookAtCamera();

		if (color.size() == 1 || !multicolor)
			currentColor = color.front().color;

		else if (index + 1 < color.size())
		{
			float lifeNormalized = life / lifeTime;
			if (color[index + 1].position > lifeNormalized)
			{
				float timeNormalized = (lifeNormalized - color[index].position) / (color[index + 1].position - color[index].position);
				if (color[index + 1].position == 0)
					timeNormalized = 0;
				//LOG("%i", index);
				currentColor = color[index].color.Lerp(color[index + 1].color, timeNormalized);
				//LERP Color
			}
			else
				index++;
		}
		else
			currentColor = color[index].color;

		transform.scale.x += sizeOverTime * dt;
		transform.scale.y += sizeOverTime * dt;
		transform.scale.z += sizeOverTime * dt;
		
		owner->UpdateParticleTrans(transform);

		angularVelocity += angularAcceleration * dt;
		angle += angularVelocity * dt;
		if (owner->isPlane)
			transform.rotation = transform.rotation.Mul(math::Quat::RotateZ(angle));
		else
			transform.rotation = transform.rotation.Mul(math::Quat::RotateAxisAngle(direction, angularVelocity * dt));


		if (isParticleAnimated && (textureRows > 1 || textureColumns > 1))
		{
			animationTime += dt;
			if (animationTime > animationSpeed)
			{
				if ((textureColumns * textureRows) > contFrame + 1)
				{
					if ((textureColumns * textureRows) > currentFrame + 1)
					{
						currentFrame++;
						contFrame++;
					}
					else
						currentFrame = 0;

					currMinUVCoord.x = (currentFrame % textureColumns) * textureColumnsNorm;
					currMinUVCoord.y = ((textureRows - 1) - (currentFrame / textureColumns)) * textureRowsNorm;
					animationTime = 0.0f;
				}
				else if (!owner->dieOnAnimation)
					contFrame = 0u;
				
				else
					EndParticle();
			}
		}
	}
	else
	{
		EndParticle();
	}

	return true;
}

void Particle::EndParticle()
{
	active = false;
	owner->particles.remove(this);
	App->particle->activeParticles--;
	if (owner->subEmitter)
	{
		ComponentEmitter* emitter = (ComponentEmitter*)owner->subEmitter->GetComponent(EmitterComponent);
		if (subEmitterActive && emitter)
		{
			math::float3 globalPos = owner->subEmitter->transform->GetPosition();
			emitter->newPositions.push_back(transform.position - globalPos);
		}
	}
}

void Particle::LookAtCamera()
{
	math::float3 zAxis = -App->renderer3D->GetCurrentCamera()->frustum.front;
	math::float3 yAxis = App->renderer3D->GetCurrentCamera()->frustum.up;
	math::float3 xAxis = yAxis.Cross(zAxis).Normalized();

	transform.rotation.Set(math::float3x3(xAxis, yAxis, zAxis));
}

float Particle::GetCamDistance() const
{
	return App->renderer3D->GetCurrentCamera()->frustum.pos.DistanceSq(transform.position);
}

void Particle::SetCamDistance()
{
	 camDistance = App->renderer3D->GetCurrentCamera()->frustum.pos.DistanceSq(transform.position);
}

void Particle::Draw()
{
	if (active)
	{
		ResourceMaterial* resourceMaterial = (ResourceMaterial*)App->res->GetResource(owner->materialRes);
		uint shaderUuid = resourceMaterial->GetShaderUuid();
		ResourceShaderProgram* resourceShaderProgram = (ResourceShaderProgram*)App->res->GetResource(shaderUuid);
		GLuint shaderProgram = resourceShaderProgram->shaderProgram;

		App->glCache->SwitchShader(shaderProgram);
		
		math::float4x4 model_matrix = transform.GetMatrix();// particle matrix
		model_matrix = model_matrix.Transposed();
		math::float4x4 mvp_matrix = model_matrix * App->renderer3D->viewProj_matrix;
		math::float4x4 normal_matrix = model_matrix;
		normal_matrix.Inverse();
		normal_matrix.Transpose();

		uint location = glGetUniformLocation(shaderProgram, "model_matrix");
		glUniformMatrix4fv(location, 1, GL_FALSE, model_matrix.ptr());
		location = glGetUniformLocation(shaderProgram, "mvp_matrix");
		glUniformMatrix4fv(location, 1, GL_FALSE, mvp_matrix.ptr());
		location = glGetUniformLocation(shaderProgram, "normal_matrix");
		glUniformMatrix3fv(location, 1, GL_FALSE, normal_matrix.Float3x3Part().ptr());
		location = glGetUniformLocation(shaderProgram, "currColor");
		glUniform4f(location,currentColor.x, currentColor.y, currentColor.z, currentColor.w);

		location = glGetUniformLocation(shaderProgram, "rowUVNorm");
		glUniform1f(location, textureRowsNorm);
		location = glGetUniformLocation(shaderProgram, "columUVNorm");
		glUniform1f(location, textureColumnsNorm);
		location = glGetUniformLocation(shaderProgram, "currMinCoord");
		glUniform2f(location, currMinUVCoord.x, currMinUVCoord.y);
		location = glGetUniformLocation(shaderProgram, "isAnimated");
		glUniform1i(location, isParticleAnimated);
		location = glGetUniformLocation(shaderProgram, "averageColor");
		glUniform1f(location, owner->colorAverage);

		// Unknown uniforms
		uint textureUnit = 0;
		std::vector<Uniform> uniforms = resourceMaterial->GetUniforms();
		for (uint i = 0; i < uniforms.size(); ++i)
		{
			Uniform uniform = uniforms[i];

			if (strcmp(uniform.common.name, "material.albedo") == 0 || strcmp(uniform.common.name, "material.specular") == 0)
			{

				if (textureUnit < App->renderer3D->GetMaxTextureUnits())
				{
					glActiveTexture(GL_TEXTURE0 + textureUnit);
					glBindTexture(GL_TEXTURE_2D, uniform.sampler2DU.value.id);
					glUniform1i(uniform.common.location, textureUnit);
					++textureUnit;
				}
			}
		}

		ResourceMesh* plane = (ResourceMesh*)App->res->GetResource(owner->uuidMeshPart);
		glBindVertexArray(plane->GetVAO());

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, plane->GetIBO());
		glDrawElements(GL_TRIANGLES, plane->GetIndicesCount(), GL_UNSIGNED_INT, NULL);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
}

float Particle::CreateRandomNum(math::float2 edges)//.x = minPoint & .y = maxPoint
{
	float num = edges.x;
	if (edges.x < edges.y)
	{
		float random = App->GenerateRandomNumber();	
		num = ((edges.y - edges.x) * random / (float)MAXUINT) + edges.x;
	}
	return num;
}

void Particle::ChangeAnim(ParticleAnimation partAnim)
{
	isParticleAnimated = partAnim.isParticleAnimated;
	textureRows = partAnim.textureRows;
	textureColumns = partAnim.textureColumns;
	textureRowsNorm = partAnim.textureRowsNorm;
	textureColumnsNorm = partAnim.textureColumnsNorm;
	animationSpeed = CreateRandomNum(partAnim.animationSpeed);
	if (!partAnim.randAnim)
		currentFrame = 0u;
	else
		currentFrame = rand() % (textureColumns * textureRows - 1);
}

//Particle transform
math::float4x4 ParticleTrans::GetMatrix() const
{
	return  math::float4x4::FromTRS(position, rotation, scale);
}

//Particle transform
math::float4x4 ParticleTrans::GetSpaceMatrix() const
{
	return  math::float4x4::FromTRS(position, math::Quat::identity, scale);
}
