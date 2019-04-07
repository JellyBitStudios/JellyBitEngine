#ifndef __MODULE_UI_H__
#define __MODULE_UI_H__

#include "Module.h"

#include "MathGeoLib/include/Math/float4.h"
#include "MathGeoLib/include/Math/float2.h"
#include <list>
#include <vector>
#include <queue>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "ComponentLabel.h"

#define UI_MAX_COMPONENTS_IMAGE 16348
#define UI_MAX_COMPONENTS_LABEL 320
#define UI_MAX_LABEL_LETTERS 256
#define UI_BYTES_IMAGE sizeof(float) * 16
#define UI_BYTES_LABEL UI_BYTES_IMAGE * UI_MAX_LABEL_LETTERS
#define UI_BUFFER_SIZE UI_BYTES_IMAGE * UI_MAX_COMPONENTS_IMAGE + UI_BYTES_LABEL * UI_MAX_COMPONENTS_LABEL
#define UI_BIND_INDEX 1

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

	void OnWindowResize(uint width, uint height);

	uint* GetRectUI();
	uint* GetScreen();

	bool IsUIHovered();

	static GameObject* FindCanvas(GameObject* from); //TODO J Check if I can make this static

	void FillBufferRange(uint offset, uint size, char* buffer);
	void RegisterBufferIndex(uint *offset, int* index, ComponentTypes cType, Component* cmp);
	void UnRegisterBufferIndex(uint offset, ComponentTypes cType);

	void ReAssignButtonOnClicks();

private:

	bool Init(JSON_Object* jObject);
	bool Start();
	update_status PreUpdate();
	update_status Update();
	update_status PostUpdate();
	bool CleanUp();

	void OnSystemEvent(System_Event event);

	void initRenderData();
	void DrawScreenCanvas();
	void DrawWorldCanvas();
	void DrawUIImage(math::float3 corners[4], math::float4& color, uint texture, math::float2& mask);
	void DrawUILabel(std::vector<ComponentLabel::LabelLetter>* workd, std::vector<uint>* GetTexturesWord, math::float4& color);

	void UpdateRenderStates();

public:
	std::list<GameObject*> canvas;
	std::list<GameObject*> canvas_screen;
	std::list<GameObject*> canvas_worldScreen;
	std::list<GameObject*> canvas_world;
	std::list<ComponentButton*> buttons_ui;
	FT_Library library;

private:
	uint uiWorkSpace[4];
	uint ui_size_draw[4];
	//math::float4x4 orthonormalMatrix = math::float4x4::identity;
	uint reference_vertex;

	uint ui_shader = 0;

	bool uiMode = true;
	
	bool anyItemIsHovered = false;

	bool depthTest, cullFace, lighting, blend;

	//UI in one buffer - Shader Storage Buffer Object
	uint countImages = 0;
	uint countLabels = 0;
	uint ssboUI = 0;
	uint offsetImage = 0;
	uint offsetLabel = UI_BUFFER_SIZE - UI_BYTES_LABEL;
	std::vector<uint> free_image_offsets;
	std::vector<uint> free_label_offsets;
	std::queue<ComponentImage*> queueImageToBuffer;
	std::queue<ComponentLabel*> queueLabelToBuffer;
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