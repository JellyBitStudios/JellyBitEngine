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

#include <algorithm>

Lights::Lights()
{
}

Lights::~Lights()
{
}

void Lights::AddLight(const ComponentLight* light)
{
	lights.push_back(light);
}

bool Lights::EraseLight(const ComponentLight* light)
{
	bool ret = false;

	if (lights.size() <= 0)
		return false;

	std::vector<const ComponentLight*>::const_iterator it = std::find(lights.begin(), lights.end(), light);
	ret = it != lights.end();

	if (ret)
		lights.erase(it);

	return ret;
}

void Lights::UseLights(const unsigned int shaderID) const
{
	for (int i = 0; i < lights.size(); ++i)
	{
		char str[20];
		sprintf(str, "lights[%i].Dir", i);
		math::float3 dir = lights[i]->GetParent()->transform->GetGlobalMatrix().WorldZ();
		glUniform3fv(glGetUniformLocation(shaderID, str), 1, dir.ptr());
		sprintf(str, "lights[%i].Color", i);
		glUniform3fv(glGetUniformLocation(shaderID, str), 1, lights[i]->color);
	}
}

void Lights::DebugDrawLights() const
{
	for (int i = 0; i < lights.size(); ++i)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		ResourceShaderProgram* resProgram = (ResourceShaderProgram*)App->res->GetResource(App->resHandler->billboardShaderProgram);
		uint shader = resProgram->shaderProgram;
		glUseProgram(shader);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, ((ResourceTexture*)App->res->GetResource(App->resHandler->lightIcon))->GetId());
		glUniform1i(glGetUniformLocation(shader, "diffuse"), 0);

		math::float4x4 model_matrix = lights[i]->GetParent()->transform->GetGlobalMatrix();
		model_matrix = model_matrix.Transposed();
		ComponentCamera* camera = App->renderer3D->GetCurrentCamera();
		math::float4x4 view_matrix = camera->GetOpenGLViewMatrix();
		math::float4x4 proj_matrix = camera->GetOpenGLProjectionMatrix();
		math::float4x4 mvp_matrix = model_matrix * view_matrix * proj_matrix;
		glUniformMatrix4fv(glGetUniformLocation(shader, "mvp_matrix"), 1, GL_FALSE, mvp_matrix.ptr());

		const ResourceMesh* mesh = (const ResourceMesh*)App->res->GetResource(App->resHandler->plane);
		glBindVertexArray(mesh->GetVAO());
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->GetIBO());
		glDrawElements(GL_TRIANGLES, mesh->GetIndicesCount(), GL_UNSIGNED_INT, NULL);
		glBindVertexArray(0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glActiveTexture(GL_TEXTURE0);
	}
}
