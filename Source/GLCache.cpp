#include "GLCache.h"

#include "Application.h"
#include "ModuleInternalResHandler.h"

#include "ComponentImage.h"
#include "ComponentLabel.h"
#include "ComponentSlider.h"

#include "glew/include/GL/glew.h"

GLCache::GLCache()
{
}

GLCache::~GLCache()
{
}

void GLCache::Init()
{
	//Get Info Hardware - Fer-ho al module renderer amb flags? vendor i extensions
	//std::string vendor = (char*)glGetString(GL_VENDOR);
	//if (vendor == "NVIDIA Corporation")
	isNVIDIA = true;
	//GLint params;
	//glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &params);
	//glGetIntegerv(GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT, &params);
	//glGetIntegerv(GL_SHADER_STORAGE_BUFFER_SIZE, &params);

	//glGenBuffers(1, &ubo_viewProj);
	//glBindBufferBase(GL_UNIFORM_BUFFER, UBO_PROJVIEW_INDEX, ubo_viewProj);
	//glBufferData(GL_UNIFORM_BUFFER, VIEWPROJ_SIZE, 0, GL_DYNAMIC_DRAW);

	if (isNVIDIA)
	{
		ui_shader = App->resHandler->UIShaderProgram;
		//--- One Buffer UI - Shader Storage Buffer Object ----
		glGenBuffers(1, &ssboUI);
		glBindBufferRange(GL_SHADER_STORAGE_BUFFER, UI_BIND_INDEX, ssboUI, 0, UI_BUFFER_SIZE);
		glBufferData(GL_SHADER_STORAGE_BUFFER, UI_BUFFER_SIZE, nullptr, GL_DYNAMIC_DRAW);
		// Bind UBO to shader Interface Block
		GLuint uloc = glGetProgramResourceIndex(ui_shader, GL_SHADER_STORAGE_BLOCK, "UICorners");
		glShaderStorageBlockBinding(ui_shader, uloc, UI_BIND_INDEX);
		glBufferStorage(GL_SHADER_STORAGE_BUFFER, UI_BUFFER_SIZE, nullptr, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
		//-------------
	}
}

void GLCache::FillBufferRange(uint offset, uint size, char * buffer)
{
	if (!isNVIDIA)
		return;

	//-------- Shader Storage Buffer Object Update -------------
	void* buff_ptr = glMapNamedBufferRange(ssboUI, offset, size, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);
	std::memcpy(buff_ptr, buffer, size);
	glFlushMappedNamedBufferRange(ssboUI, offset, size);
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	//-----
}

void GLCache::RegisterBufferIndex(uint * offset, int * index, ComponentTypes cType, Component * cmp)
{
	if (!isNVIDIA)
		return;

	switch (cType)
	{
	case ComponentTypes::ImageComponent:
	{
		if (countImages <= UI_MAX_COMPONENTS_IMAGE)
		{
			if (!free_image_offsets.empty())
			{
				*offset = free_image_offsets.at(free_image_offsets.size() - 1);
				free_image_offsets.pop_back();
			}
			else
			{
				*offset = offsetImage;
				offsetImage += UI_BYTES_RECT;
			}
			*index = *offset / (sizeof(float) * 4);
			countImages++;
		}
		else
		{
			*index = -1;
			queueImageToBuffer.push((ComponentImage*)cmp);
			CONSOLE_LOG(LogTypes::Warning, "Component Image range buffer is full, adding to queue. Total %i images.", queueImageToBuffer.size());
		}
		break;
	}
	case ComponentTypes::SliderComponent:
	{
		if (countImages <= UI_MAX_COMPONENTS_IMAGE)
		{
			if (!free_image_offsets.empty())
			{
				*offset = free_image_offsets.at(free_image_offsets.size() - 1);
				free_image_offsets.pop_back();
			}
			else
			{
				*offset = offsetImage;
				offsetImage += UI_BYTES_RECT;
			}
			*index = *offset / (sizeof(float) * 4);
			countImages++;
		}
		else
		{
			*index = -1;
			queueSliderToBuffer.push((ComponentSlider*)cmp);
			CONSOLE_LOG(LogTypes::Warning, "Component Slider range buffer is full, adding to queue. Total %i images.", queueSliderToBuffer.size());
		}
		break;
	}
	case ComponentTypes::LabelComponent:
	{
		if (countLabels <= UI_MAX_COMPONENTS_LABEL)
		{
			if (!free_label_offsets.empty())
			{
				*offset = free_label_offsets.at(free_label_offsets.size() - 1);
				free_label_offsets.pop_back();
			}
			else
			{
				*offset = offsetLabel;
				offsetLabel -= UI_BYTES_LABEL;
			}
			*index = *offset / (sizeof(float) * 4);
			countLabels++;
		}
		else
		{
			*index = -1;
			queueLabelToBuffer.push((ComponentLabel*)cmp);
			CONSOLE_LOG(LogTypes::Warning, "Component Label range buffer is full, adding to queue. Total %i labels.", queueLabelToBuffer.size());
		}
		break;
	}
	}
}

void GLCache::UnRegisterBufferIndex(uint offset, ComponentTypes cType)
{
	if (!isNVIDIA)
		return;

	switch (cType)
	{
	case ComponentTypes::ImageComponent:
	{
		if (countImages != 0)
		{
			countImages--;
			if (offset == offsetImage - UI_BYTES_RECT)
				offsetImage = offset;
			else
				free_image_offsets.push_back(offset);
			if (!queueImageToBuffer.empty())
			{
				ComponentImage* img = queueImageToBuffer.front();
				queueImageToBuffer.pop();
				uint offsetSend;
				if (!free_image_offsets.empty())
				{
					offsetSend = free_image_offsets.at(free_image_offsets.size() - 1);
					free_image_offsets.pop_back();
				}
				else
				{
					offsetSend = offsetImage;
					offsetImage += UI_BYTES_RECT;
				}
				img->SetBufferRangeAndFIll(offsetSend, offsetSend / sizeof(float) * 4);
				countImages++;
			}
		}
		break;
	}
	case ComponentTypes::SliderComponent:
	{
		if (countImages != 0)
		{
			countImages--;
			if (offset == offsetImage - UI_BYTES_RECT)
				offsetImage = offset;
			else
				free_image_offsets.push_back(offset);
			if (!queueSliderToBuffer.empty())
			{
				ComponentSlider* slider = queueSliderToBuffer.front();
				queueSliderToBuffer.pop();
				uint offsetSend;
				if (!free_image_offsets.empty())
				{
					offsetSend = free_image_offsets.at(free_image_offsets.size() - 1);
					free_image_offsets.pop_back();
				}
				else
				{
					offsetSend = offsetImage;
					offsetImage += UI_BYTES_RECT;
				}
				slider->SetBufferRangeAndFIll(offsetSend, offsetSend / sizeof(float) * 4);
				countImages++;
			}
		}
		break;
	}
	case ComponentTypes::LabelComponent:
	{
		if (countLabels != 0)
		{
			countLabels--;
			if (offset == offsetLabel + UI_BYTES_LABEL)
				offsetLabel = offset;
			else
				free_label_offsets.push_back(offset);
			if (!queueLabelToBuffer.empty())
			{
				ComponentLabel* lb = queueLabelToBuffer.front();
				queueLabelToBuffer.pop();
				uint offsetSend;
				if (!free_label_offsets.empty())
				{
					offsetSend = free_label_offsets.at(free_label_offsets.size() - 1);
					free_label_offsets.pop_back();
				}
				else
				{
					offsetSend = offsetLabel;
					offsetLabel -= UI_BYTES_LABEL;
				}
				lb->SetBufferRangeAndFIll(offsetSend, offsetSend / sizeof(float) * 4);
				countLabels++;
			}
		}
		break;
	}
	}
}

bool GLCache::isNvidia() const
{
	return isNVIDIA;
}

void GLCache::ResetUIBufferValues()
{
	if (!isNVIDIA)
		return;

	countImages = 0;
	countLabels = 0;
	offsetImage = 0;
	offsetLabel = UI_BUFFER_SIZE - UI_BYTES_LABEL;
	free_image_offsets.clear();
	free_label_offsets.clear();
	std::queue<ComponentImage*> emptyI;
	std::swap(queueImageToBuffer, emptyI);
	std::queue<ComponentLabel*> emptyL;
	std::swap(queueLabelToBuffer, emptyL);
}

void GLCache::SwitchShader(uint id)
{
	if (currentShader == id)
		return;
	glUseProgram(id);
	currentShader = id;
}
