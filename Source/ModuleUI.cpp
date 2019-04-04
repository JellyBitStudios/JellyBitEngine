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

bool ModuleUI::Init(JSON_Object * jObject)
{

	return true;
}

bool ModuleUI::Start()
{
	//Shader
	ui_shader = App->resHandler->UIShaderProgram;
	uiLabel_shader = App->resHandler->UILabelShaderProgram;
	initRenderData();

	ui_size_draw[Screen::X] = 0;
	ui_size_draw[Screen::Y] = 0;
	ui_size_draw[Screen::WIDTH] = App->window->GetWindowWidth();
	ui_size_draw[Screen::HEIGHT] = App->window->GetWindowHeight();

	uiWorkSpace[Screen::WIDTH] = 1280;
	uiWorkSpace[Screen::HEIGHT] = 720;
	uiWorkSpace[Screen::X] = (((int)ui_size_draw[Screen::WIDTH] - (int)uiWorkSpace[Screen::WIDTH]) < 0) ? 0 : (ui_size_draw[Screen::WIDTH] - uiWorkSpace[Screen::WIDTH]);
	uiWorkSpace[Screen::Y] = 0;


#ifdef GAMEMODE
	uiMode = true;
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
	return true;
}

void ModuleUI::OnSystemEvent(System_Event event)
{
	switch (event.type)
	{
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

	//-------- Uniform Buffer Object -------------
	GLuint bind_UI_index = 1;
	// Create shaderstorage buffer object
	glGenBuffers(1, &uboUI);
	glBindBufferBase(GL_UNIFORM_BUFFER, bind_UI_index, uboUI);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(uiShader_data), nullptr, GL_DYNAMIC_DRAW);

	// Bind UBO to shader Interface Block
	GLuint bloc = glGetUniformBlockIndex(ui_shader, "UIBlock");
	glUniformBlockBinding(ui_shader, bloc, bind_UI_index);
	//-------------
		//-------- Shader Storage Buffer Object -------------
	GLuint bind_UILabel_index = 2;
	// Create shaderstorage buffer object
	glGenBuffers(1, &ssboLabel);
	glBindBufferRange(GL_SHADER_STORAGE_BUFFER, bind_UILabel_index, ssboLabel, 0, maxBufferLabelStorage);
	glBufferData(GL_SHADER_STORAGE_BUFFER, maxBufferLabelStorage, nullptr, GL_DYNAMIC_DRAW);
	// Bind UBO to shader Interface Block
	GLuint sloc = glGetProgramResourceIndex(uiLabel_shader, GL_SHADER_STORAGE_BLOCK, "UIWord");
	glShaderStorageBlockBinding(uiLabel_shader, sloc, bind_UILabel_index);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, maxBufferLabelStorage,nullptr, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
	//-------------
}

void ModuleUI::DrawCanvas()
{
	if (depthTest) glDisable(GL_DEPTH_TEST);
	if (cullFace)  glDisable(GL_CULL_FACE);
	if (lighting) glDisable(GL_LIGHTING);

	if (!canvas_screen.empty())
	{
		use(ui_shader);
		setBool(ui_shader, "isScreen", 1);
		use(uiLabel_shader);
		setBool(uiLabel_shader, "isScreen", 1);

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
						case ComponentCanvasRenderer::RenderTypes::LABEL:
							DrawUILabel(rend->GetBufferWord(), rend->GetBufferSize(), rend->GetWordSize(), rend->texturesWord(), rend->GetColor());
							break;
						}

						rend = renderer->GetDrawAvaiable();
					}
				}
			}
		}
		use(0);
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

	if (!canvas_world.empty())
	{
		math::float4x4 view = ((ComponentCamera*)App->renderer3D->GetCurrentCamera())->GetOpenGLViewMatrix();
		math::float4x4 projection = ((ComponentCamera*)App->renderer3D->GetCurrentCamera())->GetOpenGLProjectionMatrix();
		math::float4x4 mvp = view * projection;
		use(ui_shader);
		setBool(ui_shader, "isScreen", 0);
		setFloat4x4(ui_shader, "mvp_matrix", mvp.ptr());
		use(uiLabel_shader);
		setBool(uiLabel_shader, "isScreen", 0);
		setFloat4x4(uiLabel_shader, "mvp_matrix", mvp.ptr());

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
						case ComponentCanvasRenderer::RenderTypes::LABEL:
							DrawUILabel(rend->GetBufferWord(), rend->GetBufferSize(), rend->GetWordSize(),rend->texturesWord(), rend->GetColor());
							break;
						}

						rend = renderer->GetDrawAvaiable();
					}
				}
			}
		}
		use(0);
	}

	if (lighting) glEnable(GL_LIGHTING);
}

void ModuleUI::DrawUIImage(ComponentRectTransform * rect, math::float4& color, uint id_texture, math::float2& mask, float rotation)
{
	use(ui_shader);

	SetRectToShader(rect);

	int useMask = (mask.x < 0) ? false : true;
	memcpy(&uiShader_data.useMask, &useMask, sizeof(uiShader_data.useMask));
	if(useMask)
		memcpy(&uiShader_data.coordsMask, mask.ptr(), sizeof(uiShader_data.coordsMask));


	use(ui_shader);
	glUniform1i(glGetUniformLocation(ui_shader, "isLabel"), 0);
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

	//-------- Uniform Buffer Object -------------
	// Update buffer
	glBindBuffer(GL_UNIFORM_BUFFER, uboUI);
	void* buff_ptr = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
	std::memcpy(buff_ptr, &uiShader_data, sizeof(uiShader_data));
	glUnmapBuffer(GL_UNIFORM_BUFFER);
	// -------------

	glBindVertexArray(reference_vertex);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);

}

void ModuleUI::DrawUILabel(GLbyte* bufferWord, uint sizeBuffer, uint wordSize, std::vector<uint>* texturesWord, math::float4& color)
{
	use(uiLabel_shader);
	setBool(uiLabel_shader, "isLabel", true);
	setBool(uiLabel_shader, "using_texture", true);
	setUnsignedInt(uiLabel_shader, "image", 0);
	setFloat(uiLabel_shader, "spriteColor", color.x, color.y, color.z, color.w);

	//-------- Shader Storage Buffer Object -------------
	void* buff_ptr = glMapNamedBufferRange(ssboLabel, 0, sizeBuffer, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);
	std::memcpy(buff_ptr, bufferWord, sizeBuffer);
	glFlushMappedNamedBufferRange(ssboLabel, 0, sizeBuffer);
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	//-----

	for (uint i = 0; i < wordSize; i++)
	{
		use(uiLabel_shader);
		setFloat(uiLabel_shader, "letter_index", (float)i);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texturesWord->at(i));

		glBindVertexArray(reference_vertex);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glBindTexture(GL_TEXTURE_2D, 0);
		glBindVertexArray(0);
	}
}

void ModuleUI::SetRectToShader(ComponentRectTransform * rect, int rFrom, math::float3* cornersLetter)
{
	math::float3* rect_world = nullptr;
	ComponentRectTransform::RectFrom from;
	(rect) ? from = rect->GetFrom() : from = (ComponentRectTransform::RectFrom)rFrom;

	switch (from)
	{
	case ComponentRectTransform::RectFrom::RECT:
		(rect) ? rect_world = rect->GetCorners() : rect_world = cornersLetter;

		memcpy(&uiShader_data.topLeft, &rect_world[ComponentRectTransform::Rect::RTOPLEFT], sizeof(uiShader_data.topLeft));
		memcpy(&uiShader_data.topRight, &rect_world[ComponentRectTransform::Rect::RTOPRIGHT], sizeof(uiShader_data.topRight));
		memcpy(&uiShader_data.bottomLeft, &rect_world[ComponentRectTransform::Rect::RBOTTOMLEFT], sizeof(uiShader_data.bottomLeft));
		memcpy(&uiShader_data.bottomRight, &rect_world[ComponentRectTransform::Rect::RBOTTOMRIGHT], sizeof(uiShader_data.bottomRight));

		break;
	case ComponentRectTransform::RectFrom::WORLD:
	case ComponentRectTransform::RectFrom::RECT_WORLD:
		(rect) ? rect_world = rect->GetCorners() : rect_world = cornersLetter;

		memcpy(&uiShader_data.topLeft, &rect_world[ComponentRectTransform::Rect::RTOPLEFT], sizeof(uiShader_data.topLeft));
		memcpy(&uiShader_data.topRight, &rect_world[ComponentRectTransform::Rect::RTOPRIGHT], sizeof(uiShader_data.topRight));
		memcpy(&uiShader_data.bottomLeft, &rect_world[ComponentRectTransform::Rect::RBOTTOMLEFT], sizeof(uiShader_data.bottomLeft));
		memcpy(&uiShader_data.bottomRight, &rect_world[ComponentRectTransform::Rect::RBOTTOMRIGHT], sizeof(uiShader_data.bottomRight));

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
	System_Event windowChanged;
	windowChanged.type = System_Event_Type::ScreenChanged;
	for (GameObject* goScreenCanvas : canvas_screen)
		goScreenCanvas->OnSystemEvent(windowChanged);

#else
	int diff_x = uiWorkSpace[Screen::X];
	uiWorkSpace[Screen::X] = (((int)ui_size_draw[Screen::WIDTH] - (int)uiWorkSpace[Screen::WIDTH]) < 0) ? 0 : (ui_size_draw[Screen::WIDTH] - uiWorkSpace[Screen::WIDTH]);
	uiWorkSpace[Screen::Y] = 0;

	diff_x = (int)uiWorkSpace[Screen::X] - (int)diff_x;

	if (diff_x != 0)
		for (GameObject* goScreenCanvas : canvas_screen)
			if (goScreenCanvas->cmp_rectTransform)
				goScreenCanvas->cmp_rectTransform->WorkSpaceChanged(diff_x);
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

 uint * ModuleUI::GetScreen()
 {
	 return ui_size_draw;
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
