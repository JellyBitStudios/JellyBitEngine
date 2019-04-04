#ifndef __MODULE_UI_H__
#define __MODULE_UI_H__

#include "Module.h"

#include "MathGeoLib/include/Math/float4.h"
#include "MathGeoLib/include/Math/float2.h"
#include <list>

#include "ComponentLabel.h"

#include <ft2build.h>
#include FT_FREETYPE_H

class GameObject;

//Possible Solution
//https://stackoverflow.com/questions/47026863/opengl-geometry-shader-with-orthographic-projection

enum UIState
{
	IDLE,
	HOVERED,
	R_CLICK,
	L_CLICK
};

class ResourceShaderProgram;
class ResourceShaderObject;
class ComponentRectTransform;
class TextureImportSettings;
class ResourceTexture;

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

	void DrawCanvas();
	void DrawWorldCanvas();

	bool GetUIMode() const;
	void SetUIMode(bool stat);

	void OnWindowResize(uint width, uint height);

	uint* GetRectUI();
	uint* GetScreen();

	bool IsUIHovered();

	static GameObject* FindCanvas(GameObject* from); //TODO J Check if I can make this static

private:

	bool Init(JSON_Object* jObject);
	bool Start();
	update_status PreUpdate();
	update_status Update();
	update_status PostUpdate();
	bool CleanUp();

	void OnSystemEvent(System_Event event);

	void initRenderData();
	void DrawUIImage(ComponentRectTransform* rect, math::float4& color, uint texture, math::float2& mask, float rotation = 0.0f);
	void DrawUILabel(signed char* bufferWord, uint sizeBuffer, uint wordSize, std::vector<uint>* texturesWord, math::float4& color);

	void SetRectToShader(ComponentRectTransform* rect, int rFrom = -1, math::float3* cornersLetter = nullptr);

	void UpdateRenderStates();

public:
	std::list<GameObject*> canvas;
	std::list<GameObject*> canvas_screen;
	std::list<GameObject*> canvas_worldScreen;
	std::list<GameObject*> canvas_world;

	FT_Library library;

private:
	uint uiWorkSpace[4];
	uint ui_size_draw[4];
	//math::float4x4 orthonormalMatrix = math::float4x4::identity;
	uint reference_vertex;

	uint ui_shader = 0;
	uint uiLabel_shader = 0;

	bool uiMode = true;
	
	bool anyItemIsHovered = false;

	bool depthTest, cullFace, lighting, blend;

	//uniform buffer
	uint uboUI = 0;
	struct uiShader_data_t
	{
		int useMask;
		int offsetint1;
		float coordsMask[2];
		float topLeft[3];
		float toffsetfloat1;
		float topRight[3];
		float toffsetfloat2;
		float bottomLeft[3];
		float toffsetfloat3;
		float bottomRight[3];
		float tofsetfloat4;
	} uiShader_data;

	//shader storage buffer onject
	uint ssboLabel;
	uint maxBufferLabelStorage = 16348;//16kb

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

	static void setUnsignedInt(unsigned int ID, const char* name, unsigned int value);
	static void setUnsignedInt(unsigned int ID, const char* name, unsigned int value, unsigned int value2);
	static void setUnsignedInt(unsigned int ID, const char* name, unsigned int value, unsigned int value2, unsigned int value3);
	static void setUnsignedInt(unsigned int ID, const char* name, unsigned int value, unsigned int value2, unsigned int value3, unsigned int value4);

	static void setFloat3x3(unsigned int ID, const char* name, const float* trans);
	static void setFloat4x4(unsigned int ID, const char* name, const float* trans);
};

#endif