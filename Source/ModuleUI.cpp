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

#include "GLCache.h"
#include "GameObject.h"

#include "ComponentCamera.h"
#include "ComponentCanvas.h"
#include "ComponentRectTransform.h"
#include "ComponentCanvasRenderer.h"
#include "ComponentButton.h"
#include "ComponentImage.h"
#include "ComponentLabel.h"
#include "ComponentScript.h"
#include "ComponentSlider.h"

#include "ResourceMaterial.h"
#include "Brofiler/Brofiler.h"

#include "MathGeoLib/include/Geometry/Frustum.h"

#define glError() { \
    GLenum err = glGetError(); \
    while (err != GL_NO_ERROR) { \
    CONSOLE_LOG(LogTypes::Normal, "glError: %s", \
           (char*)gluErrorString(err)); \
    err = glGetError(); \
    } \
    }

ModuleUI::ModuleUI(bool start_enabled) : Module(start_enabled)
{
	//math::Frustum::ViewportToScreenSpace();
}

ModuleUI::~ModuleUI()
{
}

void ModuleUI::DrawUI()
{
#ifndef GAMEMODE
	BROFILER_CATEGORY(__FUNCTION__, Profiler::Color::Orchid);
#endif
	if (!uiMode)
		return;
	use(ui_shader);
	math::float4x4 view = ((ComponentCamera*)App->renderer3D->GetCurrentCamera())->GetOpenGLViewMatrix();
	math::float4x4 projection = ((ComponentCamera*)App->renderer3D->GetCurrentCamera())->GetOpenGLProjectionMatrix();
	math::float4x4 mvp = view * projection;
	setFloat4x4(ui_shader, "mvp_matrix", mvp.ptr());
	glBindVertexArray(reference_vertex);
	glActiveTexture(GL_TEXTURE0);
	UpdateRenderStates();
	DrawWorldCanvas();
	DrawScreenCanvas();
	glBindVertexArray(0);
	use(0);
}

bool ModuleUI::Init(JSON_Object * jObject)
{

	return true;
}

bool ModuleUI::Start()
{
	//Shader
	ui_shader = App->resHandler->UIShaderProgram;
	use(ui_shader);
	setUnsignedInt(ui_shader, "image", 0);
	use(0);

	initRenderData();

	ui_size_draw[Screen::X] = 0;
	ui_size_draw[Screen::Y] = 0;
	ui_size_draw[Screen::WIDTH] = App->window->GetWindowWidth();
	ui_size_draw[Screen::HEIGHT] = App->window->GetWindowHeight();

#ifdef GAMEMODE
	uiMode = true;
	screenInWorld = false;
#else

	WorldHolder = new GameObject("UIHolder", nullptr);
	WorldHolder->AddComponent(ComponentTypes::TransformComponent);
	WorldHolder->transform->SetPosition({ 8.0f, 4.5f, 0.0f });
	WorldHolder->transform->SetScale({ 16.0f, 9.0f, 1.0f });

	ComponentCanvas* c = (ComponentCanvas*)WorldHolder->AddComponent(ComponentTypes::CanvasComponent);
	c->Update();
	c->Change(ComponentCanvas::CanvasType::WORLD);
	c->Update();
	WorldHolder->AddComponent(ComponentTypes::RectTransformComponent);
	((ComponentImage*)WorldHolder->AddComponent(ComponentTypes::ImageComponent))->SetResImageUuid(App->resHandler->screenInWorldTexture);

	uiSizeEditor[Screen::X] = 0;
	uiSizeEditor[Screen::Y] = 0;
	uiSizeEditor[Screen::WIDTH] = 1600;
	uiSizeEditor[Screen::HEIGHT] = 900;


#endif // GAMEMODE

	if (FT_Init_FreeType(&library))
		CONSOLE_LOG(LogTypes::Error, "Error when it's initialization FreeType");

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
	FT_Done_FreeType(library);
#ifndef GAMEMODE
	RELEASE(WorldHolder);
#endif // !GAMEMODE
	return true;
}

void ModuleUI::OnSystemEvent(System_Event event)
{
	switch (event.type)
	{
	case System_Event_Type::LoadFinished:
	{
#ifdef GAMEMODE
		System_Event windowChanged;
		windowChanged.type = System_Event_Type::ScreenChanged;
		for (GameObject* goScreenCanvas : canvas_screen)
			goScreenCanvas->OnSystemEvent(windowChanged);
#else //Chekear scene changes without gamemode
		if (App->GetEngineState() == engine_states::ENGINE_EDITOR)
		{
			screenInWorld = true;
			canvas.push_front(WorldHolder);
			canvas_world.push_front(WorldHolder);

			System_Event updateCornersToScreen;
			updateCornersToScreen.type = System_Event_Type::RectTransformUpdated;
			for (GameObject* goScreenCanvas : canvas_screen)
			{
				std::vector<GameObject*> childs;
				goScreenCanvas->GetChildrenAndThisVectorFromLeaf(childs);
				for (std::vector<GameObject*>::reverse_iterator go = childs.rbegin(); go != childs.rend(); ++go)
					(*go)->OnSystemEvent(updateCornersToScreen);
			}
		}
#endif // GAMEMODE
	}
	break;
	case System_Event_Type::Play:
	{
#ifndef GAMEMODE
		if (App->GetEngineState() == engine_states::ENGINE_EDITOR)
		{
			canvas_world.remove(WorldHolder);
			canvas.remove(WorldHolder);
		}
#endif // GAMEMODE

		screenInWorld = false;
		System_Event updateCornersToScreen;
		updateCornersToScreen.type = System_Event_Type::RectTransformUpdated;
		for (GameObject* goScreenCanvas : canvas_screen)
		{
			std::vector<GameObject*> childs;
			goScreenCanvas->GetChildrenAndThisVectorFromLeaf(childs);
			for (std::vector<GameObject*>::reverse_iterator go = childs.rbegin(); go != childs.rend(); ++go)
				(*go)->OnSystemEvent(updateCornersToScreen);
		}
		break;
	}
	case System_Event_Type::LoadScene:
	{
		App->glCache->ResetUIBufferValues();

#ifndef GAMEMODE
		if (App->GetEngineState() == engine_states::ENGINE_EDITOR)
		{
			uint temp; int tm;
			App->glCache->RegisterBufferIndex(&temp, &tm, ComponentTypes::ImageComponent, WorldHolder->cmp_image);
		}
#endif
	}
	case System_Event_Type::Stop:
	{
		canvas.clear();
		canvas_screen.clear();
		canvas_worldScreen.clear();
		canvas_world.clear();
		buttons_ui.clear();
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
	case System_Event_Type::UpdateBillboard:
	{
		for (GameObject* goWorldCanvas : canvas_world)
			goWorldCanvas->OnSystemEvent(event);
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

void ModuleUI::DrawScreenCanvas()
{
	if (depthTest) glDisable(GL_DEPTH_TEST);
	if (cullFace)  glDisable(GL_CULL_FACE);
	if (lighting) glDisable(GL_LIGHTING);

	if (!canvas_screen.empty())
	{
		setBool(ui_shader, "isScreen", !screenInWorld);

		for (GameObject* canvas : canvas_screen)
		{
			std::vector<GameObject*> renderers;
			canvas->GetChildrenAndThisVectorFromLeaf(renderers);
			for (std::vector<GameObject*>::reverse_iterator render = renderers.rbegin(); render != renderers.rend(); ++render)
			{
				ComponentCanvasRenderer* renderer = (*render)->cmp_canvasRenderer;
				if (renderer)
				{
					ComponentCanvasRenderer::ToUIRend* rend = renderer->GetDrawAvaiable();
					while (rend != nullptr)
					{
						switch (rend->GetType())
						{
						case ComponentCanvasRenderer::RenderTypes::IMAGE:
							DrawUIImage(rend->GetIndex(), renderer->GetParent()->cmp_rectTransform->GetCorners(), rend->GetColor(), rend->GetTexture(), rend->GetMaskValues());
							break;
						case ComponentCanvasRenderer::RenderTypes::LABEL:
							DrawUILabel(rend->GetIndex(), rend->GetWord(), rend->GetColor());
							break;
						case ComponentCanvasRenderer::RenderTypes::SLIDER:
						{
							math::float3* corners = (*render)->cmp_slider->GetCorners();
							math::float2 null(-1.0f, -1.0f);
							DrawUIImage((*render)->cmp_slider->GetBackBufferIndex(), corners, rend->GetColor(), (*render)->cmp_slider->GetBackTexture(), null);
							DrawUIImage((*render)->cmp_slider->GetFrontBufferIndex(), &corners[4], rend->GetColor(), (*render)->cmp_slider->GetFrontTexture(), rend->GetMaskValues());
							break;
						}
						}

						rend = renderer->GetDrawAvaiable();
					}
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
	if (!blend) glEnable(GL_BLEND);
	if (lighting) glDisable(GL_LIGHTING);

	if (!canvas_world.empty())
	{
		setBool(ui_shader, "isScreen", 0);
		for (GameObject* canvas : canvas_world)
		{
			std::vector<GameObject*> renderers;
			canvas->GetChildrenAndThisVectorFromLeaf(renderers);
			for (std::vector<GameObject*>::reverse_iterator render = renderers.rbegin(); render != renderers.rend(); ++render)
			{
				ComponentCanvasRenderer* renderer = (*render)->cmp_canvasRenderer;
				if (renderer)
				{
					ComponentCanvasRenderer::ToUIRend* rend = renderer->GetDrawAvaiable();
					while (rend != nullptr)
					{
						switch (rend->GetType())
						{
						case ComponentCanvasRenderer::RenderTypes::IMAGE:
							DrawUIImage(rend->GetIndex(), renderer->GetParent()->cmp_rectTransform->GetCorners(), rend->GetColor(), rend->GetTexture(), rend->GetMaskValues());
							break;
						case ComponentCanvasRenderer::RenderTypes::LABEL:
							DrawUILabel(rend->GetIndex(), rend->GetWord(), rend->GetColor());
							break;
						case ComponentCanvasRenderer::RenderTypes::SLIDER:
						{
							math::float3* corners = (*render)->cmp_slider->GetCorners();
							math::float2 null(-1.0f, -1.0f);
							DrawUIImage((*render)->cmp_slider->GetBackBufferIndex(), corners, rend->GetColor(), (*render)->cmp_slider->GetBackTexture(), null);
							DrawUIImage((*render)->cmp_slider->GetFrontBufferIndex(), &corners[4], rend->GetColor(), (*render)->cmp_slider->GetFrontTexture(), rend->GetMaskValues());
							break;
						}
					}
						rend = renderer->GetDrawAvaiable();
					}
				}
			}
		}
	}

	if (lighting) glEnable(GL_LIGHTING);
}

void ModuleUI::DrawUIImage(int index, math::float3 corners[4], math::float4& color, uint texture, math::float2& mask)
{
	if (App->glCache->isShaderStorage())
	{
		setFloat(ui_shader, "indexCorner", float(index));
	}
	else
	{
		setFloat(ui_shader, "topLeft", { corners[ComponentRectTransform::Rect::RTOPLEFT], 1.0f });
		setFloat(ui_shader, "topRight", { corners[ComponentRectTransform::Rect::RTOPRIGHT], 1.0f });
		setFloat(ui_shader, "bottomLeft", { corners[ComponentRectTransform::Rect::RBOTTOMLEFT], 1.0f });
		setFloat(ui_shader, "bottomRight", { corners[ComponentRectTransform::Rect::RBOTTOMRIGHT], 1.0f });
	}

	bool useMask = (mask.x < 0) ? false : true;
	setBool(ui_shader, "useMask", useMask);
	if (useMask)
		setFloat(ui_shader, "coordsMask", mask.x, mask.y);

	setBool(ui_shader, "isLabel", false);
	setFloat(ui_shader, "spriteColor", color.x, color.y, color.z, color.w);
	if (texture > 0)
	{
		setBool(ui_shader, "using_texture", true);
		glBindTexture(GL_TEXTURE_2D, texture);
	}
	else
		setBool(ui_shader, "using_texture", false);

	glDrawArrays(GL_TRIANGLES, 0, 6);
	if (texture > 0) glBindTexture(GL_TEXTURE_2D, 0);
}

void ModuleUI::DrawUILabel(int index, std::vector<LabelLetter*>* word, math::float4& color)
{
	setBool(ui_shader, "isLabel", true);
	setBool(ui_shader, "using_texture", true);
	setFloat(ui_shader, "spriteColor", color.x, color.y, color.z, color.w);

	uint wordSize = word->size();
	for (uint i = 0; i < wordSize; i++)
	{
		if (App->glCache->isShaderStorage())
		{
			setFloat(ui_shader, "indexCorner", float(index + (4.0f * (float)i)));
		}
		else
		{
			setFloat(ui_shader, "topLeft",{ word->at(i)->rect->GetCorners()[ComponentRectTransform::Rect::RTOPLEFT], 1.0f });
			setFloat(ui_shader, "topRight",{ word->at(i)->rect->GetCorners()[ComponentRectTransform::Rect::RTOPRIGHT], 1.0f });
			setFloat(ui_shader, "bottomLeft",{ word->at(i)->rect->GetCorners()[ComponentRectTransform::Rect::RBOTTOMLEFT], 1.0f });
			setFloat(ui_shader, "bottomRight",{ word->at(i)->rect->GetCorners()[ComponentRectTransform::Rect::RBOTTOMRIGHT], 1.0f });
		}
		glBindTexture(GL_TEXTURE_2D, word->at(i)->textureID);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
	glBindTexture(GL_TEXTURE_2D, 0);
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

GameObject * ModuleUI::FindCanvas(GameObject * from, uint& count)
{
	GameObject* ret = nullptr;
	GameObject* temp = from;
	count = 0;
	while (ret == nullptr && temp != nullptr)
	{
		if (temp->cmp_canvas)
			ret = temp;
		if (!ret)
		{
			temp = temp->GetParent();
			++count;
		}
	}
	return (ret) ? ret : nullptr;
}

#ifndef GAMEMODE
math::float4x4 ModuleUI::GetUIMatrix()
{
	return WorldHolder->transform->GetGlobalMatrix();
}
math::float3 * ModuleUI::GetWHCorners()
{
	return WorldHolder->cmp_rectTransform->GetCorners();
}

int * ModuleUI::GetWHRect()
{
	return WorldHolder->cmp_rectTransform->GetRect();
}
math::float3 ModuleUI::GetPositionWH()
{
	return WorldHolder->transform->GetPosition();
}
void ModuleUI::SetPositionWH(math::float3 pos)
{
	WorldHolder->transform->SetPosition(pos);
	System_Event WHMoved;
	WHMoved.type = System_Event_Type::RectTransformUpdated;
	WorldHolder->cmp_rectTransform->OnSystemEvent(WHMoved);
	WorldHolder->cmp_rectTransform->Update();
	WorldHolder->cmp_image->OnSystemEvent(WHMoved);
	WorldHolder->cmp_image->Update();
	for (GameObject* goScreenCanvas : canvas_screen)
		goScreenCanvas->OnSystemEvent(WHMoved);
}
#endif

bool ModuleUI::GetUIMode() const
{
	return uiMode;
}

void ModuleUI::SetUIMode(bool stat)
{
	uiMode = stat;
}

bool ModuleUI::ScreenOnWorld() const
{
	return screenInWorld;
}

void ModuleUI::OnWindowResize(uint width, uint height)
{
	ui_size_draw[Screen::WIDTH] = width;
	ui_size_draw[Screen::HEIGHT] = height;


#ifdef GAMEMODE
	System_Event windowChanged;
	windowChanged.type = System_Event_Type::ScreenChanged;
	for (GameObject* goScreenCanvas : canvas_screen)
		goScreenCanvas->OnSystemEvent(windowChanged);

#else

#endif // GAMEMODE
}

int* ModuleUI::GetRectUI()
{
#ifdef GAMEMODE
	return ui_size_draw;
#else
	return uiSizeEditor;
#endif
}

int * ModuleUI::GetScreen()
{
	return ui_size_draw;
}

bool ModuleUI::MouseInScreen()
{
	for (GameObject* goScreenCanvas : canvas_screen)
	{
		if (goScreenCanvas->IsActive())
		{
			std::vector<GameObject*> childs;
			goScreenCanvas->GetChildrenAndThisVectorFromLeaf(childs);

			for (GameObject* child : childs)
			{
				if(child->cmp_rectTransform && (child->cmp_image || child->cmp_button || child->cmp_label || child->cmp_slider))
				{
					if (child->cmp_rectTransform->IsTreeActive())
					{
						int* rect = child->cmp_rectTransform->GetRect();

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
		}
	}
	return false;
}

// Shader methods
void ModuleUI::use(unsigned int ID)
{
	App->glCache->SwitchShader(ID);
}

void ModuleUI::Delete(unsigned int ID)
{
	glDeleteProgram(ID);
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

void ModuleUI::setFloat(unsigned int ID, const char * name, math::float4 value)
{
	glUniform4f(glGetUniformLocation(ID, name), value.x, value.y, value.z, value.w);
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
