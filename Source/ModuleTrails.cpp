#include "ModuleTrails.h"

#include "ModuleTimeManager.h"
#include "ModuleInput.h"
#include "ModuleResourceManager.h"
#include "ModuleRenderer3D.h"
#include "ModuleInternalResHandler.h"

#include "ResourceMaterial.h"
#include "ResourceShaderObject.h"
#include "ResourceShaderProgram.h"
#include "ResourceMesh.h"

#include "Brofiler/Brofiler.h"
#include <algorithm>
#include "MathGeoLib/include/Math/float4x4.h"
#include "Application.h"
#include "DebugDrawer.h"
#include "ComponentTransform.h"

#include "glew/include/GL/glew.h"

ModuleTrails::ModuleTrails(bool start_enabled) : Module(start_enabled)
{}

ModuleTrails::~ModuleTrails()
{
	trails.clear();
}

update_status ModuleTrails::PostUpdate()
{
#ifndef GAMEMODE
	BROFILER_CATEGORY(__FUNCTION__, Profiler::Color::PapayaWhip);
#endif // !GAMEMODE

	for (std::list<ComponentTrail*>::iterator trail = trails.begin(); trail != trails.end(); ++trail)
	{
		(*trail)->Update();
	}

	return UPDATE_CONTINUE;
}

void ModuleTrails::Draw()
{
#ifndef GAMEMODE
	BROFILER_CATEGORY(__FUNCTION__, Profiler::Color::PapayaWhip);
#endif // !GAMEMODE

	for (std::list<ComponentTrail*>::iterator trail = trails.begin(); trail != trails.end(); ++trail)
	{
		if (!(*trail)->trailVertex.empty())
		{
			if ((*trail)->materialRes == 0) continue;

			std::list<TrailNode*>::iterator begin = (*trail)->trailVertex.begin();
			TrailNode* end = (*trail)->trailVertex.back();


			float i = 0.0f;
			float size = (*trail)->trailVertex.size() + 1;

			for (std::list<TrailNode*>::iterator curr = (*trail)->trailVertex.begin(); curr != (*trail)->trailVertex.end(); ++curr)
			{
				i++;
				std::list<TrailNode*>::iterator next = curr;
				++next;
				if (next != (*trail)->trailVertex.end())
				{
					ResourceMaterial* resourceMaterial = (ResourceMaterial*)App->res->GetResource((*trail)->materialRes);
					uint shaderUuid = resourceMaterial->GetShaderUuid();
					ResourceShaderProgram* resourceShaderProgram = (ResourceShaderProgram*)App->res->GetResource(shaderUuid);
					GLuint shaderProgram = resourceShaderProgram->shaderProgram;

					glUseProgram(shaderProgram);

					math::float4x4 model_matrix = math::float4x4::identity;// particle matrix
					model_matrix = model_matrix.Transposed();
					math::float4x4 view_matrix = App->renderer3D->GetCurrentCamera()->GetOpenGLViewMatrix();
					math::float4x4 proj_matrix = App->renderer3D->GetCurrentCamera()->GetOpenGLProjectionMatrix();
					math::float4x4 mvp_matrix = model_matrix * view_matrix * proj_matrix;
					math::float4x4 normal_matrix = model_matrix;
					normal_matrix.Inverse();
					normal_matrix.Transpose();

					uint location = glGetUniformLocation(shaderProgram, "model_matrix");
					glUniformMatrix4fv(location, 1, GL_FALSE, model_matrix.ptr());
					location = glGetUniformLocation(shaderProgram, "mvp_matrix");
					glUniformMatrix4fv(location, 1, GL_FALSE, mvp_matrix.ptr());
					location = glGetUniformLocation(shaderProgram, "normal_matrix");
					glUniformMatrix3fv(location, 1, GL_FALSE, normal_matrix.Float3x3Part().ptr());


					float currUV = (float(i) / size);
					float nextUV = (float(i + 1) / size);
					math::float3 originHigh = (*curr)->originHigh;
					math::float3 originLow = (*curr)->originLow;
					math::float3 destinationHigh = (*next)->originHigh;
					math::float3 destinationLow = (*next)->originLow;

					if ((*trail)->orient)
						RearrangeVertex(trail, curr, next, currUV, nextUV, originHigh, originLow, destinationHigh, destinationLow);


					location = glGetUniformLocation(shaderProgram, "currUV");				// cUV
					glUniform1f(location, currUV);
					location = glGetUniformLocation(shaderProgram, "nextUV");				// cUV
					glUniform1f(location, nextUV);

					location = glGetUniformLocation(shaderProgram, "realColor");			// Color
					glUniform4f(location, (*trail)->color.x, (*trail)->color.y, (*trail)->color.z, (*trail)->color.w);

					location = glGetUniformLocation(shaderProgram, "vertex1");				// Current High
					glUniform3f(location, originHigh.x, originHigh.y, originHigh.z);
					location = glGetUniformLocation(shaderProgram, "vertex2");				// Current Low
					glUniform3f(location, originLow.x, originLow.y, originLow.z);
					location = glGetUniformLocation(shaderProgram, "vertex3");				// Next High
					glUniform3f(location, destinationHigh.x, destinationHigh.y, destinationHigh.z);
					location = glGetUniformLocation(shaderProgram, "vertex4");				// Next Low
					glUniform3f(location, destinationLow.x, destinationLow.y, destinationLow.z);


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

					ResourceMesh* plane = (ResourceMesh*)App->res->GetResource(App->resHandler->plane);
					glBindVertexArray(plane->GetVAO());

					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, plane->GetIBO());
					glDrawElements(GL_TRIANGLES, plane->GetIndicesCount(), GL_UNSIGNED_INT, NULL);

					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, 0);

					glBindBuffer(GL_ARRAY_BUFFER, 0);
					glBindVertexArray(0);
					glUseProgram(0);
				}
			}

			glEnd();
			glPopMatrix();
		}
	}
}

void ModuleTrails::RearrangeVertex(std::list<ComponentTrail *>::iterator &trail, std::list<TrailNode *>::iterator &curr, std::list<TrailNode *>::iterator &next, float &currUV, float &nextUV, math::float3 &originHigh, math::float3 &originLow, math::float3 &destinationHigh, math::float3 &destinationLow)
{
	// Rearrange vertex
	float origin = 0;
	float dest = 0;

	GetOriginAndDest(trail, origin, curr, dest, next);

	if (origin < dest)
	{
		float tmp = currUV;
		currUV = nextUV;
		nextUV = tmp;

		math::float3 tmph = originHigh;
		math::float3 tmpl = originLow;

		originHigh = destinationHigh;
		originLow = destinationLow;

		destinationHigh = tmph;
		destinationLow = tmpl;
	}
}

void ModuleTrails::GetOriginAndDest(std::list<ComponentTrail *>::iterator &trail, float &origin, std::list<TrailNode *>::iterator &curr, float &dest, std::list<TrailNode *>::iterator &next)
{
	switch ((*trail)->vector)
	{
	case X:
		origin = (*curr)->originHigh.x;
		dest = (*next)->originHigh.x;
		break;
	case Y:
		// This is not right
		origin = (*curr)->originHigh.x;
		dest = (*next)->originHigh.x;
		break;
	case Z:
		dest = (*curr)->originHigh.z;
		origin = (*next)->originHigh.z;
		break;
	default:
		break;
	}
}


void ModuleTrails::DebugDraw() const
{
	// Todo
}



void ModuleTrails::OnSystemEvent(System_Event event)
{
	switch (event.type)
	{
		case System_Event_Type::Play:
		case System_Event_Type::LoadFinished:
			// Todo
			break;
		case System_Event_Type::Stop:
			// Todo
			break;
	}
}

void ModuleTrails::RemoveTrail(ComponentTrail* trail)
{
	trails.remove(trail);
}
