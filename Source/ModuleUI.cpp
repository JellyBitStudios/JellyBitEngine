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
#include "ComponentCanvas.h"
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
	if (depthTest) glDisable(GL_DEPTH_TEST);
	if (cullFace)  glDisable(GL_CULL_FACE);
	if (lighting) glDisable(GL_LIGHTING);

	for (GameObject* canvas : canvas_screen)
	{

		std::vector<GameObject*> renderers;
		canvas->GetChildrenAndThisVectorFromLeaf(renderers);
		std::reverse(renderers.begin(), renderers.end());

		for (GameObject* render : renderers)
		{
			ComponentCanvasRenderer* renderer = render->cmp_canvasRenderer;
			if (renderer)
			{
				ComponentCanvasRenderer::ToUIRend* rend = renderer->GetDrawAvaiable();
				while (rend != nullptr)
				{
					switch (rend->GetType())
					{
					case ComponentCanvasRenderer::RenderTypes::IMAGE:
						DrawUIImage(render->cmp_rectTransform, rend->GetColor(), rend->GetTexture(), rend->GetMaskValues());
						break;
					}

					rend = renderer->GetDrawAvaiable();
				}
			}
		}
	}
	
	if (depthTest) glEnable(GL_DEPTH_TEST);
	if (cullFace) glEnable(GL_CULL_FACE);
	if (lighting) glEnable(GL_LIGHTING);
	if (!blend) glDisable(GL_BLEND);

}

void ModuleUI::DrawWorldCanvas()
{
	UpdateRenderStates();
	if (!blend) glEnable(GL_BLEND);
	if (lighting) glDisable(GL_LIGHTING);

	for (GameObject* canvas : canvas_world)
	{

		std::vector<GameObject*> renderers;
		canvas->GetChildrenAndThisVectorFromLeaf(renderers);

		for (GameObject* render : renderers)
		{
			ComponentCanvasRenderer* renderer = render->cmp_canvasRenderer;
			if (renderer)
			{
				ComponentCanvasRenderer::ToUIRend* rend = renderer->GetDrawAvaiable();
				while (rend != nullptr)
				{
					switch (rend->GetType())
					{
					case ComponentCanvasRenderer::RenderTypes::IMAGE:
						DrawUIImage(render->cmp_rectTransform, rend->GetColor(), rend->GetTexture(), rend->GetMaskValues());
						break;
					}

					rend = renderer->GetDrawAvaiable();
				}
			}
		}
	}

	if (lighting) glEnable(GL_LIGHTING);
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

	uiWorkSpace[Screen::WIDTH] = 1280;
	uiWorkSpace[Screen::HEIGHT] = 720;
	uiWorkSpace[Screen::X] = (((int)ui_size_draw[Screen::WIDTH] - (int)uiWorkSpace[Screen::WIDTH]) < 0) ? 0 : (ui_size_draw[Screen::WIDTH] - uiWorkSpace[Screen::WIDTH]);
	uiWorkSpace[Screen::Y] = (((int)ui_size_draw[Screen::HEIGHT] - (int)uiWorkSpace[Screen::HEIGHT]) < 0) ? 0 : (ui_size_draw[Screen::HEIGHT] - uiWorkSpace[Screen::HEIGHT]);


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
	for (std::list<GameObject*>::iterator iteratorUI = canvas.begin(); iteratorUI != canvas.end(); ++iteratorUI)
		(*iteratorUI)->cmp_canvas->Update();
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
	switch (event.type)
	{
		case System_Event_Type::Play:
		{
			/* Set mask on play, but now is on inspector.
			for (GameObject* go_canvas : canvas)
			{
				std::vector<GameObject*> go_images;
				go_canvas->GetChildrenAndThisVectorFromLeaf(go_images);

				for (GameObject* cImage : go_images)
					if (cImage->cmp_image)
						cImage->cmp_image->SetMask();
			}
			*/
			break;
		}
		case System_Event_Type::LoadScene:
		case System_Event_Type::Stop:
		{
			canvas.clear();
			canvas_screen.clear();
			canvas_worldScreen.clear();
			canvas_world.clear();
			break;
		}
		case System_Event_Type::ComponentDestroyed:
		{
			switch (event.compEvent.component->GetType())
			{
				case ComponentTypes::CanvasComponent:
				{
					ComponentCanvas* cmp_canvas = (ComponentCanvas*)event.compEvent.component;
					GameObject* parentC = cmp_canvas->GetParent();
					canvas.remove(parentC);
					switch (cmp_canvas->GetType())
					{
					case ComponentCanvas::CanvasType::SCREEN:
						canvas_screen.remove(parentC);
						break;
					case ComponentCanvas::CanvasType::WORLD_SCREEN:
						canvas_worldScreen.remove(parentC);
						break;
					case ComponentCanvas::CanvasType::WORLD:
						canvas_world.remove(parentC);
						break;
					}
					break;
				}
			}
			break;
		}
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

void ModuleUI::DrawUIImage(ComponentRectTransform * rect, math::float4& color, uint id_texture, math::float2& mask, float rotation)
{
	use(ui_shader);
	SetRectToShader(rect);
	
	setBool(ui_shader, "useMask", (mask.x < 0) ? false : true);
	setFloat(ui_shader, "coordsMask", mask.x, mask.y);

	setFloat(ui_shader, "spriteColor", color.x, color.y, color.z, color.w);

	if (id_texture > 0)
	{
		setBool(ui_shader, "using_texture", true);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, id_texture);
		setUnsignedInt(ui_shader, "image", 0);
	}
	else
		setBool(ui_shader, "using_texture", false);

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

void ModuleUI::UpdateRenderStates()
{
	GLenum capability = 0;

	capability = GL_DEPTH_TEST;
	depthTest = App->renderer3D->GetCapabilityState(capability);

	capability = GL_CULL_FACE;
	cullFace = App->renderer3D->GetCapabilityState(capability);

	capability = GL_LIGHTING;
	lighting = App->renderer3D->GetCapabilityState(capability);

	capability = GL_BLEND;
	blend = App->renderer3D->GetCapabilityState(capability);
}

bool ModuleUI::IsUIHovered()
{
	return anyItemIsHovered;
}

GameObject * ModuleUI::FindCanvas(GameObject * from)
{
	GameObject* ret = nullptr;
	GameObject* temp = from;

	while (ret == nullptr && temp != nullptr)
	{
		if (temp->cmp_canvas)
			ret = temp;
		temp = temp->GetParent();
	}
	return (ret) ? ret : nullptr;
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
	for (GameObject* goScreenCanvas : canvas_screen)
		goScreenCanvas->cmp_canvas->ScreenChanged();

#else
	int diff_x = uiWorkSpace[Screen::X];
	uiWorkSpace[Screen::X] = (((int)ui_size_draw[Screen::WIDTH] - (int)uiWorkSpace[Screen::WIDTH]) < 0) ? 0 : (ui_size_draw[Screen::WIDTH] - uiWorkSpace[Screen::WIDTH]);
	uiWorkSpace[Screen::Y] = 0;

	diff_x = (int)uiWorkSpace[Screen::X] - (int)diff_x;

	if (diff_x != 0)
		for (GameObject* goScreenCanvas : canvas_screen)
			if (goScreenCanvas->cmp_rectTransform)
				goScreenCanvas->cmp_rectTransform->WorkSpaceChanged(abs(diff_x), (diff_x > 0) ? true : false);
#endif // GAMEMODE
}

 uint* ModuleUI::GetRectUI()
{
#ifdef GAMEMODE
	 return ui_size_draw;
#else
	return uiWorkSpace;
#endif // GAMEMODE

}

bool ModuleUI::MouseInScreen()
{
	for (GameObject* goScreenCanvas : canvas_screen)
	{
		if (goScreenCanvas->IsActive())
		{
			if (goScreenCanvas->cmp_rectTransform)
			{
				uint* rect = goScreenCanvas->cmp_rectTransform->GetRect();

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
