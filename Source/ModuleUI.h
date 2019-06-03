#ifndef __MODULE_UI_H__
#define __MODULE_UI_H__

#include "Module.h"

#include "MathGeoLib/include/Math/float4x4.h"
#include "MathGeoLib/include/Math/float4.h"
#include "MathGeoLib/include/Math/float2.h"
#include <list>
#include <vector>
#include <queue>

#include <ft2build.h>
#include FT_FREETYPE_H

struct LabelLetter;

//Possible Solution
//https://stackoverflow.com/questions/47026863/opengl-geometry-shader-with-orthographic-projection

enum UIState
{
	IDLE,
	HOVERED,
	R_CLICK,
	L_CLICK
};

class GameObject;
class ResourceShaderProgram;
class ResourceShaderObject;
class ComponentRectTransform;
class TextureImportSettings;
class ResourceTexture;
class ComponentImage;
class ComponentLabel;
class ComponentButton;
class ComponentCanvasRenderer;
class ComponentSlider;

enum ComponentTypes;

class ModuleUI : public Module
{
public:

	enum Screen
	{
		X,
		Y,
		WIDTH,
		HEIGHT
	};

public:
	ModuleUI(bool start_enabled = true);
	~ModuleUI();

	void DrawUI();

	bool GetUIMode() const;
	void SetUIMode(bool stat);
	bool ScreenOnWorld() const;

	void OnWindowResize(uint width, uint height);

	int* GetRectUI();
	int* GetScreen();

	bool IsUIHovered();
	static GameObject* FindCanvas(GameObject* from, uint& count);

	void SetCurrentSlider(ComponentSlider* slider);
	bool IsSliderCurrent() const;

#ifndef GAMEMODE
	//Engine world
	math::float4x4 GetUIMatrix();
	math::float3* GetWHCorners();
	int* GetWHRect();
	math::float3 GetPositionWH();
	void SetPositionWH(math::float3 pos);
#endif

private:

	bool Init(JSON_Object* jObject);
	bool Start();
	update_status PreUpdate();
	update_status Update();
	update_status PostUpdate();
	bool CleanUp();

	void OnSystemEvent(System_Event event);

	void initRenderData();
	void DrawWorldCanvas(std::vector<ComponentCanvasRenderer*>& sDraws, std::vector<ComponentCanvasRenderer*>& dDraws);
	void DrawScreenCanvas(std::vector<ComponentCanvasRenderer*>& sDraws, std::vector<ComponentCanvasRenderer*>& dDraws);

	void DrawStaticUIImage(int index, math::float4& color, uint texture, math::float2& mask);
	void DrawStaticUILabel(int index, std::vector<LabelLetter*>* word, math::float4& color);
	void DrawDynamicUIImage(math::float3 corners[4], math::float4& color, uint texture, math::float2& mask);
	void DrawDynamicUILabel(std::vector<LabelLetter*>* word, math::float4& color, bool worldDraw = false);

	void UpdateRenderStates();

public:
	std::list<GameObject*> canvas;
	std::list<GameObject*> canvas_screen;
	std::list<GameObject*> canvas_world;
	std::vector<ComponentButton*> buttons_ui;
	FT_Library library;

private:
	int ui_size_draw[4];
	int uiSizeEditor[4];
	uint reference_vertex;

	uint uiStatic_shader = 0;
	uint uiDynamic_shader = 0;

	bool uiMode = true;
	bool screenInWorld = true;
	bool anyItemIsHovered = false;

	bool depthTest, cullFace, lighting, blend;

	ComponentSlider* currentSlider = nullptr;

#ifndef GAMEMODE
	//UI like Unity
	GameObject* WorldHolder = nullptr;
#endif

private:
	bool MouseInScreen();

	//Shader functions
		// use/activate the shader
	static void use(unsigned int ID);
	//Delete manually shader
	static void Delete(unsigned int ID);

	// utility uniform functions
	static void setBool(unsigned int ID, const char* name, bool value);
	static void setBool(unsigned int ID, const char* name, bool value, bool value2);
	static void setBool(unsigned int ID, const char* name, bool value, bool value2, bool value3);
	static void setBool(unsigned int ID, const char* name, bool value, bool value2, bool value3, bool value4);

	static void setInt(unsigned int ID, const char* name, int value);
	static void setInt(unsigned int ID, const char* name, int value, int value2);
	static void setInt(unsigned int ID, const char* name, int value, int value2, int value3);
	static void setInt(unsigned int ID, const char* name, int value, int value2, int value3, int value4);

	static void setFloat(unsigned int ID, const char* name, float value);
	static void setFloat(unsigned int ID, const char* name, float value, float value2);
	static void setFloat(unsigned int ID, const char* name, float value, float value2, float value3);
	static void setFloat(unsigned int ID, const char* name, float value, float value2, float value3, float value4);
	static void setFloat(unsigned int ID, const char* name, math::float3 value);
	static void setFloat(unsigned int ID, const char* name, math::float4 value);

	static void setUnsignedInt(unsigned int ID, const char* name, unsigned int value);
	static void setUnsignedInt(unsigned int ID, const char* name, unsigned int value, unsigned int value2);
	static void setUnsignedInt(unsigned int ID, const char* name, unsigned int value, unsigned int value2, unsigned int value3);
	static void setUnsignedInt(unsigned int ID, const char* name, unsigned int value, unsigned int value2, unsigned int value3, unsigned int value4);

	static void setFloat3x3(unsigned int ID, const char* name, const float* trans);
	static void setFloat4x4(unsigned int ID, const char* name, const float* trans);
};

#endif
