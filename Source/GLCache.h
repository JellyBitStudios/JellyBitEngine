#ifndef __GLCACHE_H__
#define __GLCACHE_H__

#include "Globals.h"
#include "Component.h"
#include <vector>
#include <queue>

#pragma region UIMEMORYDATA
#define UI_MAX_COMPONENTS_IMAGE 16348
#define UI_MAX_COMPONENTS_LABEL 320
#define UI_MAX_LABEL_LETTERS 256
#define UI_BYTES_RECT sizeof(float) * 16
#define UI_BYTES_LABEL UI_BYTES_RECT * UI_MAX_LABEL_LETTERS
#define UI_BUFFER_SIZE UI_BYTES_RECT * UI_MAX_COMPONENTS_IMAGE + UI_BYTES_LABEL * UI_MAX_COMPONENTS_LABEL //6mb
#define UI_BIND_INDEX 1
#pragma endregion

class GLCache
{
public:

	GLCache();
	~GLCache();

	void FillBufferRange(uint offset, uint size, char* buffer);
	void RegisterBufferIndex(uint *offset, int* index, ComponentTypes cType, Component* cmp);
	void UnRegisterBufferIndex(uint offset, ComponentTypes cType);

	bool isNvidia()const;

	void ResetUIBufferValues();

private:
	//
	bool isNVIDIA = false;


	uint ui_shader;
	//UI in one buffer - Shader Storage Buffer Object
	uint countImages = 0;
	uint countLabels = 0;
	uint ssboUI = 0; // buffer ID
	uint offsetImage = 0; // current offset images buffer ui
	uint offsetLabel = UI_BUFFER_SIZE - UI_BYTES_LABEL; // current offset labels reverse order buffer ui
	std::vector<uint> free_image_offsets;
	std::vector<uint> free_label_offsets;
	std::queue<class ComponentImage*> queueImageToBuffer;
	std::queue<class ComponentLabel*> queueLabelToBuffer;
};

#endif