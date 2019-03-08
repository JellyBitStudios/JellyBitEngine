#include "ModuleUI.h"

#include "Application.h"
#include "ModuleRenderer3D.h"
#include "ModuleResourceManager.h"
#include "ModuleInternalResHandler.h"
#include "ModuleGOs.h"
#include "ModuleWindow.h"
#include "ModuleCameraEditor.h"
#include "ModuleInput.h"
#include "ModuleScene.h"

#include "GameObject.h"

#include "ComponentCamera.h"
#include "ComponentRectTransform.h"
#include "ComponentCanvasRenderer.h"
#include "ComponentButton.h"
#include "ComponentImage.h"
#include "ComponentLabel.h"
#include "ComponentScript.h"

#include "ResourceMaterial.h"
#include "Brofiler/Brofiler.h"

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
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);
	for (std::list<Component*>::iterator iteratorUI = componentsScreenRendererUI.begin(); iteratorUI != componentsScreenRendererUI.end(); ++iteratorUI)
	{
		ComponentCanvasRenderer* renderer = (ComponentCanvasRenderer*)*iteratorUI;
		ComponentCanvasRenderer::ToUIRend* rend = renderer->GetDrawAvaiable();
		while (rend != nullptr)
		{
			switch (rend->GetType())
			{
			case ComponentCanvasRenderer::RenderTypes::COLOR_VECTOR:
				DrawUIColor((ComponentRectTransform*)renderer->GetParent()->GetComponent(ComponentTypes::RectTransformComponent), rend->GetColor());
				break;
			case ComponentCanvasRenderer::RenderTypes::TEXTURE:
				DrawUITexture((ComponentRectTransform*)renderer->GetParent()->GetComponent(ComponentTypes::RectTransformComponent), rend->GetTexture());
				break;
			}

			rend = renderer->GetDrawAvaiable();
		}
	}
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_LIGHTING);
}

void ModuleUI::DrawWorldCanvas()
{
	glDisable(GL_LIGHTING);
	for (std::list<Component*>::iterator iteratorUI = componentsWorldRendererUI.begin(); iteratorUI != componentsWorldRendererUI.end(); ++iteratorUI)
	{
		ComponentCanvasRenderer* renderer = (ComponentCanvasRenderer*)*iteratorUI;
		ComponentCanvasRenderer::ToUIRend* rend = renderer->GetDrawAvaiable();
		while (rend != nullptr)
		{
			switch (rend->GetType())
			{
			case ComponentCanvasRenderer::RenderTypes::COLOR_VECTOR:
				DrawUIColor((ComponentRectTransform*)renderer->GetParent()->GetComponent(ComponentTypes::RectTransformComponent), rend->GetColor());
				break;
			case ComponentCanvasRenderer::RenderTypes::TEXTURE:
				DrawUITexture((ComponentRectTransform*)renderer->GetParent()->GetComponent(ComponentTypes::RectTransformComponent), rend->GetTexture());
				break;
			}

			rend = renderer->GetDrawAvaiable();
		}
	}
	glEnable(GL_LIGHTING);
}

bool ModuleUI::Init(JSON_Object * jObject)
{
	return true;
}

bool ModuleUI::Start()
{
	initRenderData();

	//Shader
	ui_shader = App->resHandler->UIShaderProgram;

	ui_size_draw[Screen::X] = 0;
	ui_size_draw[Screen::Y] = 0;
	ui_size_draw[Screen::WIDTH] = App->window->GetWindowWidth();
	ui_size_draw[Screen::HEIGHT] = App->window->GetWindowHeight();

#ifdef GAMEMODE
	uiMode = true;
#endif // GAMEMODE

	return true;
}

update_status ModuleUI::PreUpdate()
{
#ifndef GAMEMODE
	BROFILER_CATEGORY(__FUNCTION__, Profiler::Color::PapayaWhip);
#endif // !GAMEMODE
	anyItemIsHovered = MouseInScreen();
	return update_status::UPDATE_CONTINUE;
}

update_status ModuleUI::Update()
{
#ifndef GAMEMODE
	BROFILER_CATEGORY(__FUNCTION__, Profiler::Color::PapayaWhip);
#endif // !GAMEMODE
	for (std::list<Component*>::iterator iteratorUI = componentsScreenRendererUI.begin(); iteratorUI != componentsScreenRendererUI.end(); ++iteratorUI)
		(*iteratorUI)->Update();
	for (std::list<Component*>::iterator iteratorUI = componentsWorldRendererUI.begin(); iteratorUI != componentsWorldRendererUI.end(); ++iteratorUI)
		(*iteratorUI)->Update();
	for (std::list<Component*>::iterator iteratorUI = componentsWorldUI.begin(); iteratorUI != componentsWorldUI.end(); ++iteratorUI)
		(*iteratorUI)->Update();

	if (App->GetEngineState() == engine_states::ENGINE_PLAY)
		for (std::list<Component*>::iterator iteratorUI = componentsUI.begin(); iteratorUI != componentsUI.end(); ++iteratorUI)
			(*iteratorUI)->Update();

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
	if (event.type == System_Event_Type::LoadFinished)
	{
		std::vector<GameObject*> children;
		App->scene->root->GetChildrenVector(children);
		for each (GameObject* child in children)
			if (std::strcmp(child->GetName(), "Canvas") == 0)
				App->GOs->SetCanvas(child);

		LinkAllRectsTransform();
	}
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
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void ModuleUI::DrawUIColor(ComponentRectTransform* rect, math::float4& color, float rotation)
{
	use(ui_shader);
	SetRectToShader(rect);
	setBool(ui_shader, "use_color", true);
	setFloat(ui_shader, "spriteColor", color.x, color.y, color.z, color.w);

	glBindVertexArray(reference_vertex);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindVertexArray(0);

	use(0);
}

void ModuleUI::DrawUITexture(ComponentRectTransform * rect, uint id_texture, float rotation)
{
	use(ui_shader);
	SetRectToShader(rect);
	setBool(ui_shader, "use_color", false);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, id_texture);
	setUnsignedInt(ui_shader, "image", 0);

	glBindVertexArray(reference_vertex);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);

	use(0);
}

void ModuleUI::SetRectToShader(ComponentRectTransform * rect)
{
	uint* rect_points = nullptr;
	math::float3* rect_world = nullptr;
	math::float2 pos;
	float w_width;
	float w_height;
	math::float4x4 view = math::float4x4::identity;
	math::float4x4 projection = math::float4x4::identity;
	math::float4x4 mvp = math::float4x4::identity;

	switch (rect->GetFrom())
	{
	case ComponentRectTransform::RectFrom::RECT:
		rect_points = rect->GetRect();
		setBool(ui_shader, "isScreen", 1);

		w_width = ui_size_draw[Screen::WIDTH];
		w_height = ui_size_draw[Screen::HEIGHT];

		pos = math::Frustum::ScreenToViewportSpace({ (float)rect_points[ComponentRectTransform::Rect::X], (float)rect_points[ComponentRectTransform::Rect::Y] }, w_width, w_height);
		setFloat(ui_shader, "topLeft", pos.x, pos.y, 0.0f);
		pos = math::Frustum::ScreenToViewportSpace({ (float)rect_points[ComponentRectTransform::Rect::X] + (float)rect_points[ComponentRectTransform::Rect::XDIST], (float)rect_points[ComponentRectTransform::Rect::Y] }, w_width, w_height);
		setFloat(ui_shader, "topRight", pos.x, pos.y, 0.0f);
		pos = math::Frustum::ScreenToViewportSpace({ (float)rect_points[ComponentRectTransform::Rect::X], (float)rect_points[ComponentRectTransform::Rect::Y] + (float)rect_points[ComponentRectTransform::Rect::YDIST] }, w_width, w_height);
		setFloat(ui_shader, "bottomLeft", pos.x, pos.y, 0.0f);
		pos = math::Frustum::ScreenToViewportSpace({ (float)rect_points[ComponentRectTransform::Rect::X] + (float)rect_points[ComponentRectTransform::Rect::XDIST], (float)rect_points[ComponentRectTransform::Rect::Y] + (float)rect_points[ComponentRectTransform::Rect::YDIST] }, w_width, w_height);
		setFloat(ui_shader, "bottomRight", pos.x, pos.y, 0.0f);
		break;

	case ComponentRectTransform::RectFrom::WORLD:
		rect_world = rect->GetCorners();
		setBool(ui_shader, "isScreen", 0);
		view = ((ComponentCamera*)App->renderer3D->GetCurrentCamera())->GetOpenGLViewMatrix();
		projection = ((ComponentCamera*)App->renderer3D->GetCurrentCamera())->GetOpenGLProjectionMatrix();
		mvp = view * projection;
		
		setFloat4x4(ui_shader, "mvp_matrix", mvp.ptr());

		setFloat(ui_shader, "topLeft", rect_world[ComponentRectTransform::Rect::RTOPLEFT]);
		setFloat(ui_shader, "topRight", rect_world[ComponentRectTransform::Rect::RTOPRIGHT]);
		setFloat(ui_shader, "bottomLeft", rect_world[ComponentRectTransform::Rect::RBOTTOMLEFT]);
		setFloat(ui_shader, "bottomRight", rect_world[ComponentRectTransform::Rect::RBOTTOMRIGHT]);
		break;

	case ComponentRectTransform::RectFrom::RECT_WORLD:
		rect_world = rect->GetCorners();
		setBool(ui_shader, "isScreen", 0);
		view = ((ComponentCamera*)App->renderer3D->GetCurrentCamera())->GetOpenGLViewMatrix();
		projection = ((ComponentCamera*)App->renderer3D->GetCurrentCamera())->GetOpenGLProjectionMatrix();
		mvp = view * projection;

		setFloat4x4(ui_shader, "mvp_matrix", mvp.ptr());

		setFloat(ui_shader, "topLeft", rect_world[ComponentRectTransform::Rect::RTOPLEFT]);
		setFloat(ui_shader, "topRight", rect_world[ComponentRectTransform::Rect::RTOPRIGHT]);
		setFloat(ui_shader, "bottomLeft", rect_world[ComponentRectTransform::Rect::RBOTTOMLEFT]);
		setFloat(ui_shader, "bottomRight", rect_world[ComponentRectTransform::Rect::RBOTTOMRIGHT]);
		break;
	}
}

bool ModuleUI::IsUIHovered()
{
	return anyItemIsHovered;
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
	ui_size_draw[Screen::WIDTH] = width;
	ui_size_draw[Screen::HEIGHT] = height;

#ifdef GAMEMODE
	LinkAllRectsTransform();
#endif // GAMEMODE
}

 uint* ModuleUI::GetRectUI()
{
	return ui_size_draw;
}

 void ModuleUI::LinkAllRectsTransform()
 {
	 std::vector<GameObject*> gos;
	 GameObject* canvas = App->GOs->GetCanvas();
	 if (canvas)
	 {
#ifdef GAMEMODE
		 ComponentRectTransform* cmp_rect = (ComponentRectTransform*)canvas->GetComponent(ComponentTypes::RectTransformComponent);
		 uint* rect = cmp_rect->GetRect();
		 rect[ComponentRectTransform::Rect::X] = 0;
		 rect[ComponentRectTransform::Rect::Y] = 0;
		 rect[ComponentRectTransform::Rect::XDIST] = ui_size_draw[Screen::WIDTH];
		 rect[ComponentRectTransform::Rect::YDIST] = ui_size_draw[Screen::HEIGHT];
#endif // GAMEMODE

		 canvas->GetChildrenAndThisVectorFromLeaf(gos);
		 std::reverse(gos.begin(), gos.end());
		 for (GameObject* go_rect : gos)
		 {
			 ComponentRectTransform* cmp_rect = (ComponentRectTransform*)go_rect->GetComponent(ComponentTypes::RectTransformComponent);
			 cmp_rect->CheckParentRect();
		 }
		 gos.clear();
	 }
	 for (GameObject* world_canvas : GOsWorldCanvas)
	 {
		 world_canvas->GetChildrenAndThisVectorFromLeaf(gos);
		 for (GameObject* go_rect : gos)
		 {
			 ComponentRectTransform* cmp_rect = (ComponentRectTransform*)go_rect->GetComponent(ComponentTypes::RectTransformComponent);
			 cmp_rect->CheckParentRect();
		 }
		 gos.clear();
	 }
}

bool ModuleUI::MouseInScreen()
{
	if (App->GOs->ExistCanvas())
	{
		GameObject* canvas = App->GOs->GetCanvas();

		std::vector<GameObject*> gos;
		canvas->GetChildrenAndThisVectorFromLeaf(gos);
		std::reverse(gos.begin(), gos.end());

		for (GameObject* go_rect : gos)
		{
			uint* rect = ((ComponentRectTransform*)go_rect->GetComponent(ComponentTypes::RectTransformComponent))->GetRect();

			if (rect)
			{
				uint mouseX = App->input->GetMouseX();
				uint mouseY = App->input->GetMouseY();

				if (mouseX > rect[ComponentRectTransform::Rect::X] && mouseX < rect[ComponentRectTransform::Rect::X] + rect[ComponentRectTransform::Rect::XDIST]
					&& mouseY > rect[ComponentRectTransform::Rect::Y] && mouseY < rect[ComponentRectTransform::Rect::Y] + rect[ComponentRectTransform::Rect::YDIST])
					return true;
			}
		}
	}
	return false;
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
