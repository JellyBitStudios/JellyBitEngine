#include "ModuleFBOManager.h"
#include "glew\include\GL\glew.h"
#include "Application.h"
#include "ModuleWindow.h"
#include "ModuleResourceManager.h"
#include "ModuleRenderer3D.h"
#include "ResourceMesh.h"
#include "ResourceShaderProgram.h"
#include "ModuleInternalResHandler.h"
#include "Lights.h"

ModuleFBOManager::ModuleFBOManager() {}

ModuleFBOManager::~ModuleFBOManager() {}

bool ModuleFBOManager::Start()
{
	LoadGBuffer(App->window->GetWindowWidth(), App->window->GetWindowHeight());
	return true;
}

bool ModuleFBOManager::CleanUp()
{
	UnloadGBuffer();
	return true;
}

void ModuleFBOManager::OnSystemEvent(System_Event event)
{}

void ModuleFBOManager::LoadGBuffer(uint width, uint height)
{
	// Get default depth buffer bits in order to generate a proper depth fbo buffer
	GLint depthBits;
	glGetIntegerv(GL_DEPTH_BITS, &depthBits);

	// DEFERRED SHADING G BUFFER
	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

	// - position color buffer
	glGenTextures(1, &gPosition);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	// Component Alpha is not required for storing Normals or Positions, but we need a floating point frame buffer
	// and RGB16F is not compatible with most computers (at least mine not)
	// no F= buffer clamped from 0 to 1. with f components are not clamped
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

	// - normal color buffer
	glGenTextures(1, &gNormal);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	// Component Alpha is not required for storing Normals or Positions, but we need a floating point frame buffer
	// and RGB16F is not compatible with most computers (at least mine not)
	// no F= buffer clamped from 0 to 1. with f components are not clamped
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

	// - color + specular color buffer
	glGenTextures(1, &gAlbedoSpec);
	glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);

	// - additional info color buffer
	glGenTextures(1, &gInfo);
	glBindTexture(GL_TEXTURE_2D, gInfo);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, width, height, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gInfo, 0);

	// - tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
	uint attachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
	glDrawBuffers(4, attachments);

	// - depth
	glGenTextures(1, &gDepth);
	glBindTexture(GL_TEXTURE_2D, gDepth);
	if (depthBits == 16)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
	else if (depthBits == 24)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
	else if (depthBits == 32)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, gDepth, 0);

	// then also add render buffer object as depth buffer and check for completeness
	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	if (depthBits == 16)
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
	else if (depthBits == 24)
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
	else if (depthBits == 32)
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

	// finally check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		CONSOLE_LOG(LogTypes::Error, "Framebuffer not complete!");
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// BIND UNIFORMS
	ResourceShaderProgram* resProgram = (ResourceShaderProgram*)App->res->GetResource(App->resHandler->deferredShaderProgram);
	glUseProgram(resProgram->shaderProgram);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	int location = glGetUniformLocation(resProgram->shaderProgram, "gPosition");
	glUniform1i(location, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	location = glGetUniformLocation(resProgram->shaderProgram, "gNormal");
	glUniform1i(location, 1);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
	location = glGetUniformLocation(resProgram->shaderProgram, "gAlbedoSpec");
	glUniform1i(location, 2);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, gInfo);
	location = glGetUniformLocation(resProgram->shaderProgram, "gInfo");
	glUniform1i(location, 3);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, gDepth);
	location = glGetUniformLocation(resProgram->shaderProgram, "gDepth");
	glUniform1i(location, 4);

	glUseProgram(0);
}

void ModuleFBOManager::UnloadGBuffer()
{
	glDeleteFramebuffers(1, &gBuffer);
	glDeleteRenderbuffers(1, &rboDepth);
	glDeleteTextures(1, &gPosition);
	glDeleteTextures(1, &gNormal);
	glDeleteTextures(1, &gAlbedoSpec);
	glDeleteTextures(1, &gInfo);
	glDeleteTextures(1, &gDepth);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ModuleFBOManager::ResizeGBuffer(uint width, uint height)
{
	// First set the new width and height and then call this method. GBuffer would be loaded with the new 
	UnloadGBuffer();
	LoadGBuffer(width, height);
}

void ModuleFBOManager::BindGBuffer()
{
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void ModuleFBOManager::DrawGBufferToScreen() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	ResourceShaderProgram* resProgram = (ResourceShaderProgram*)App->res->GetResource(App->resHandler->deferredShaderProgram);
	glUseProgram(resProgram->shaderProgram);

	// Fog
	int location = glGetUniformLocation(resProgram->shaderProgram, "view_matrix");
	if (location != -1)
	{
		math::float4x4 view_matrix = App->renderer3D->GetCurrentCamera()->GetOpenGLViewMatrix();
		glUniformMatrix4fv(location, 1, GL_FALSE, view_matrix.ptr());
	}

	location = glGetUniformLocation(resProgram->shaderProgram, "fog.color");
	if (location != -1)
		glUniform3fv(location, 1, &App->lights->fog.color[0]);
	/*
	location = glGetUniformLocation(resProgram->shaderProgram, "fog.minDist");
	if (location != -1)
		glUniform1f(location, App->lights->fog.minDist);
	location = glGetUniformLocation(resProgram->shaderProgram, "fog.maxDist");
	if (location != -1)
		glUniform1f(location, App->lights->fog.maxDist);
	*/
	location = glGetUniformLocation(resProgram->shaderProgram, "fog.density");
	if (location != -1)
		glUniform1f(location, App->lights->fog.density);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, gInfo);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, gDepth);
	App->lights->UseLights(resProgram->shaderProgram);

	const ResourceMesh* mesh = (const ResourceMesh*)App->res->GetResource(App->resHandler->plane);

	glBindVertexArray(mesh->GetVAO());
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->GetIBO());
	glDrawElements(GL_TRIANGLES, mesh->GetIndicesCount(), GL_UNSIGNED_INT, NULL);
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);
}

void ModuleFBOManager::MergeDepthBuffer(uint width, uint height)
{
	// Here we write current depth buffer from gbuffer to default
	// buffer so we can draw in forward rendering as we used to
	glBindFramebuffer(GL_READ_FRAMEBUFFER, rboDepth);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // write to default framebuffer
	glBlitFramebuffer(
		0, 0, width, height, 0, 0, width, height, GL_DEPTH_BUFFER_BIT, GL_NEAREST
	);
}