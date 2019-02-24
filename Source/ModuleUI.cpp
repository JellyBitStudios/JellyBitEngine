#include "ModuleUI.h"

#include "Application.h"
#include "ModuleCameraEditor.h"
#include "ComponentCamera.h"
#include "ModuleRenderer3D.h"
#include "ModuleResourceManager.h"
#include "ResourceShaderProgram.h"
#include "ResourceShaderObject.h"
#include "ShaderImporter.h"
#include "MaterialImporter.h"
#include "ModuleFileSystem.h"
#include "ComponentTransform.h"
#include "ComponentRectTransform.h"
#include "ComponentCanvasRenderer.h"
#include "ResourceTexture.h"
#include "ModuleWindow.h"
#include "ModuleGOs.h"
#include "ModuleInternalResHandler.h"
#include "GameObject.h"

#include "MathGeoLib/include/Geometry/Frustum.h"

ModuleUI::ModuleUI(bool start_enabled) : Module(start_enabled)
{
	//math::Frustum::ViewportToScreenSpace();
}

ModuleUI::~ModuleUI()
{
}

void ModuleUI::DrawCanvas()
{
	if (App->GOs->ExistCanvas())
	{
		GameObject* canvas = App->GOs->GetCanvas();

		std::vector<GameObject*> gos;
		canvas->GetChildrenAndThisVectorFromLeaf(gos);

		for (GameObject* go_rend : gos)
		{
			ComponentCanvasRenderer* ui_rend = (ComponentCanvasRenderer*)go_rend->GetComponent(ComponentTypes::CanvasRendererComponent);
			if (ui_rend)
			{
				ui_rend->Update();
				ComponentCanvasRenderer::ToUIRend* rend = ui_rend->GetFistQueue();
				while (rend != nullptr)
				{
					switch (rend->GetType())
					{
					case ComponentCanvasRenderer::RenderTypes::COLOR_VECTOR:
						DrawUIColor((ComponentRectTransform*)go_rend->GetComponent(ComponentTypes::RectTransformComponent), rend->GetColor());
						break;
					case ComponentCanvasRenderer::RenderTypes::TEXTURE:
						DrawUITexture((ComponentRectTransform*)go_rend->GetComponent(ComponentTypes::RectTransformComponent), rend->GetTexture());
						break;
					}

					ui_rend->Drawed();
					rend = ui_rend->GetFistQueue();
				}
			}
		}
	}
}

bool ModuleUI::Init(JSON_Object * jObject)
{
	return true;
}

bool ModuleUI::Start()
{
	initRenderData();

	//Shader
	App->resHandler->CreateUIShaderProgram();
	ui_shader = App->resHandler->UIShaderProgram;

	ui_size_draw[UI_XRECT] = 0;
	ui_size_draw[UI_YRECT] = 0;
	ui_size_draw[UI_WIDTHRECT] = App->window->GetWindowWidth();
	ui_size_draw[UI_HEIGHTRECT] = App->window->GetWindowHeight();

	return true;
}

update_status ModuleUI::PreUpdate()
{
	//orthonormalMatrix = math::float4x4(App->camera->camera->frustum.NearPlane().OrthoProjection());

	return update_status::UPDATE_CONTINUE;
}

update_status ModuleUI::Update()
{

	return update_status::UPDATE_CONTINUE;
}

update_status ModuleUI::FixedUpdate()
{
	return update_status::UPDATE_CONTINUE;
}

update_status ModuleUI::PostUpdate()
{
	return update_status::UPDATE_CONTINUE;
}

bool ModuleUI::CleanUp()
{
	return true;
}

void ModuleUI::OnSystemEvent(System_Event event)
{
}

void ModuleUI::initRenderData()
{
	// Configure VAO/VBO
	GLuint VBO;
	GLfloat vertices[] = {
		// Pos			//Tex
		1.0f,  1.0f,	1.0f, 1.0f,
		1.0f, -1.0f,	1.0f, 0.0f,
		-1.0f,  1.0f,	0.0f, 1.0f,

		 1.0f, -1.0f,	1.0f, 0.0f,
		-1.0f, -1.0f,	0.0f, 0.0f,
		-1.0f,  1.0f,	0.0f, 1.0f
	};

	glGenVertexArrays(1, &reference_vertex);
	glGenBuffers(1, &VBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);

	glBindVertexArray(reference_vertex);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)2);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void ModuleUI::DrawUIColor(ComponentRectTransform* rect, math::float4& color, float rotation)
{
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	//glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	use(ui_shader);
	SetRectToShader(rect);
	setFloat(ui_shader, "spriteColor", color.x, color.y, color.z, color.w);

	glBindVertexArray(reference_vertex);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindVertexArray(0);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	//glEnable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);

	use(0);
}

void ModuleUI::DrawUITexture(ComponentRectTransform * rect, uint texture, float rotation)
{
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	//glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	use(ui_shader);
	SetRectToShader(rect);
	setFloat(ui_shader, "image", 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, App->materialImporter->GetCheckers());

	glBindVertexArray(reference_vertex);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	//glEnable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);

	use(0);
}

void ModuleUI::SetRectToShader(ComponentRectTransform * rect)
{
	const uint* rect_points = rect->GetRect();

	float w_width = ui_size_draw[UI_WIDTHRECT];
	float w_height = ui_size_draw[UI_HEIGHTRECT];

	math::float2 pos;
	pos = math::Frustum::ScreenToViewportSpace({ (float)rect_points[X_RECT], (float)rect_points[Y_RECT] }, w_width, w_height);
	setFloat(ui_shader, "topLeft", pos.x, pos.y);
	pos = math::Frustum::ScreenToViewportSpace({ (float)rect_points[X_RECT] + (float)rect_points[XDIST_RECT], (float)rect_points[Y_RECT] }, w_width, w_height);
	setFloat(ui_shader, "topRight", pos.x, pos.y);
	pos = math::Frustum::ScreenToViewportSpace({ (float)rect_points[X_RECT], (float)rect_points[Y_RECT] + (float)rect_points[YDIST_RECT] }, w_width, w_height);
	setFloat(ui_shader, "bottomLeft", pos.x, pos.y);
	pos = math::Frustum::ScreenToViewportSpace({ (float)rect_points[X_RECT] + (float)rect_points[XDIST_RECT], (float)rect_points[Y_RECT] + (float)rect_points[YDIST_RECT] }, w_width, w_height);
	setFloat(ui_shader, "bottomRight", pos.x, pos.y);
}


bool ModuleUI::GetUIMode() const
{
	return uiMode;
}

void ModuleUI::SetUIMode(bool stat)
{
	uiMode = stat;
}

void ModuleUI::OnWindowResize(uint width, uint height)
{
	ui_size_draw[UI_WIDTHRECT] = width;
	ui_size_draw[UI_HEIGHTRECT] = height;
}

const uint * ModuleUI::GetRectUI() const
{
	return ui_size_draw;
}

void ModuleUI::LinkAllRectsTransform()
{
	GameObject* canvas = App->GOs->GetCanvas();

	std::vector<GameObject*> gos;
	canvas->GetChildrenAndThisVectorFromLeaf(gos);

	for (GameObject* go_rend : gos)
	{
		ComponentRectTransform* cmp_rect = (ComponentRectTransform*)go_rend->GetComponent(ComponentTypes::RectTransformComponent);
		cmp_rect->CheckParentRect();
	}
}

// Shader methods
void ModuleUI::use(unsigned int ID)
{
	glUseProgram(ID);
}

void ModuleUI::Delete(unsigned int ID)
{
	glUseProgram(ID);
}

void ModuleUI::setBool(unsigned int ID, const char* name, bool value)
{
	glUniform1i(glGetUniformLocation(ID, name), (int)value);
}

void ModuleUI::setBool(unsigned int ID, const char* name, bool value, bool value2)
{
	glUniform2i(glGetUniformLocation(ID, name), (int)value, (int)value2);
}

void ModuleUI::setBool(unsigned int ID, const char* name, bool value, bool value2, bool value3)
{
	glUniform3i(glGetUniformLocation(ID, name), (int)value, (int)value2, (int)value3);
}

void ModuleUI::setBool(unsigned int ID, const char* name, bool value, bool value2, bool value3, bool value4)
{
	glUniform4i(glGetUniformLocation(ID, name), (int)value, (int)value2, (int)value3, (int)value4);
}

void ModuleUI::setInt(unsigned int ID, const char* name, int value)
{
	glUniform1i(glGetUniformLocation(ID, name), value);
}

void ModuleUI::setInt(unsigned int ID, const char * name, int value, int value2)
{
	glUniform2i(glGetUniformLocation(ID, name), value, value2);
}

void ModuleUI::setInt(unsigned int ID, const char * name, int value, int value2, int value3)
{
	glUniform3i(glGetUniformLocation(ID, name), value, value2, value3);
}

void ModuleUI::setInt(unsigned int ID, const char * name, int value, int value2, int value3, int value4)
{
	glUniform4i(glGetUniformLocation(ID, name), value, value2, value3, value4);
}

void ModuleUI::setFloat(unsigned int ID, const char*name, float value)
{
	glUniform1f(glGetUniformLocation(ID, name), value);
}

void ModuleUI::setFloat(unsigned int ID, const char * name, float value, float value2)
{
	glUniform2f(glGetUniformLocation(ID, name), value, value2);
}

void ModuleUI::setFloat(unsigned int ID, const char * name, float value, float value2, float value3)
{
	glUniform3f(glGetUniformLocation(ID, name), value, value2, value3);
}

void ModuleUI::setFloat(unsigned int ID, const char * name, float value, float value2, float value3, float value4)
{
	glUniform4f(glGetUniformLocation(ID, name), value, value2, value3, value4);
}

void ModuleUI::setFloat(unsigned int ID, const char * name, math::float3 value)
{
	glUniform3f(glGetUniformLocation(ID, name), value.x, value.y, value.z);
}

void ModuleUI::setUnsignedInt(unsigned int ID, const char * name, unsigned int value)
{
	glUniform1ui(glGetUniformLocation(ID, name), value);
}

void ModuleUI::setUnsignedInt(unsigned int ID, const char * name, unsigned int value, unsigned int value2)
{
	glUniform2ui(glGetUniformLocation(ID, name), value, value2);
}

void ModuleUI::setUnsignedInt(unsigned int ID, const char * name, unsigned int value, unsigned int value2, unsigned int value3)
{
	glUniform3ui(glGetUniformLocation(ID, name), value, value2, value3);
}

void ModuleUI::setUnsignedInt(unsigned int ID, const char * name, unsigned int value, unsigned int value2, unsigned int value3, unsigned int value4)
{
	glUniform4ui(glGetUniformLocation(ID, name), value, value2, value3, value4);
}

void ModuleUI::setFloat3x3(unsigned int ID, const char * name, const float * trans)
{
	glUniformMatrix3fv(glGetUniformLocation(ID, name), 1, GL_FALSE, trans);
}

void ModuleUI::setFloat4x4(unsigned int ID, const char * name, const float* trans)
{
	glUniformMatrix4fv(glGetUniformLocation(ID, name), 1, GL_FALSE, trans);
}