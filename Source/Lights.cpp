#include "Lights.h"
#include "ComponentLight.h"

#include "glew\include\GL\glew.h"

#include "Application.h"
#include "ModuleInternalResHandler.h"
#include "ModuleResourceManager.h"
#include "GameObject.h"
#include "ComponentTransform.h"
#include "ComponentCamera.h"
#include "ModuleRenderer3D.h"
#include "ResourceMesh.h"
#include "ResourceTexture.h"
#include "ResourceShaderProgram.h"
#include "GLCache.h"

#include <algorithm>

Lights::Lights()
{
}

Lights::~Lights()
{
}

void Lights::AddLight(ComponentLight* light)
{
	lights.push_back(light);
}

bool Lights::EraseLight(ComponentLight* light)
{
	bool ret = false;

	if (lights.size() <= 0)
		return false;

	std::vector<ComponentLight*>::const_iterator it = std::find(lights.begin(), lights.end(), light);
	ret = it != lights.end();

	if (ret)
		lights.erase(it);

	return ret;
}

void Lights::UseLights(const unsigned int shaderID)
{
	glUniform1f(glGetUniformLocation(shaderID, "ambient"), ambientValue);
	for (int i = 0; i < 32; ++i)
	{
		if (i < lights.size())
		{
			lights[i]->Update();
			char str[DEFAULT_BUF_SIZE];
			sprintf(str, "lights[%i].type", i);
			if (!lights[i]->enabled)
			{
				glUniform1i(glGetUniformLocation(shaderID, str), -1);
				continue;
			}
			glUniform1i(glGetUniformLocation(shaderID, str), lights[i]->lightType);
			if (lights[i]->lightType == LightTypes::DirectionalLight)
			{
				sprintf(str, "lights[%i].dir", i);
				math::float3 dir = lights[i]->GetParent()->transform->GetGlobalMatrix().WorldZ();
				glUniform3fv(glGetUniformLocation(shaderID, str), 1, dir.ptr());
			}
			else
			{
				sprintf(str, "lights[%i].position", i);
				math::float3 pos = lights[i]->GetParent()->transform->GetGlobalMatrix().TranslatePart();
				glUniform3fv(glGetUniformLocation(shaderID, str), 1, pos.ptr());
			}
			sprintf(str, "lights[%i].color", i);
			glUniform3fv(glGetUniformLocation(shaderID, str), 1, lights[i]->color);
			sprintf(str, "lights[%i].linear", i);
			glUniform1f(glGetUniformLocation(shaderID, str), lights[i]->linear);
			sprintf(str, "lights[%i].quadratic", i);
			glUniform1f(glGetUniformLocation(shaderID, str), lights[i]->quadratic);
		}
		else
		{
			char str[20];
			sprintf(str, "lights[%i].type", i);
			glUniform1i(glGetUniformLocation(shaderID, str), 0);
		}
	}
}

void Lights::DebugDrawLights() const
{
	glDisable(GL_DEPTH_TEST);
	ResourceShaderProgram* resProgram = (ResourceShaderProgram*)App->res->GetResource(App->resHandler->billboardShaderProgram);
	uint shader = resProgram->shaderProgram;
	App->glCache->SwitchShader(shader);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ((ResourceTexture*)App->res->GetResource(App->resHandler->lightIcon))->GetId());
	glUniform1i(glGetUniformLocation(shader, "diffuse"), 0);
	const ResourceMesh* mesh = (const ResourceMesh*)App->res->GetResource(App->resHandler->plane);
	glBindVertexArray(mesh->GetVAO());
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->GetIBO());
	for (int i = 0; i < lights.size(); ++i)
	{
		math::float4x4 model_matrix = lights[i]->GetParent()->transform->GetGlobalMatrix();
		math::float3 zAxis = -App->renderer3D->GetCurrentCamera()->frustum.front;
		math::float3 yAxis = App->renderer3D->GetCurrentCamera()->frustum.up;
		math::float3 xAxis = yAxis.Cross(zAxis).Normalized();
		model_matrix.SetRotatePart(math::float3x3(xAxis, yAxis, zAxis));
		model_matrix = model_matrix.Transposed();
		ComponentCamera* camera = App->renderer3D->GetCurrentCamera();
		math::float4x4 view_matrix = camera->GetOpenGLViewMatrix();
		math::float4x4 proj_matrix = camera->GetOpenGLProjectionMatrix();
		math::float4x4 mvp_matrix = model_matrix * view_matrix * proj_matrix;
		glUniformMatrix4fv(glGetUniformLocation(shader, "mvp_matrix"), 1, GL_FALSE, mvp_matrix.ptr());
		glDrawElements(GL_TRIANGLES, mesh->GetIndicesCount(), GL_UNSIGNED_INT, NULL);

	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glEnable(GL_DEPTH_TEST);
	App->glCache->SwitchShader(0);
}