#include "GLCache.h"
#include "ComponentImage.h"
#include "ComponentLabel.h"

#include "glew/include/GL/glew.h"

GLCache::GLCache()
{
	if (isNVIDIA)
	{
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

GLCache::~GLCache()
{
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

	if (cType == ComponentTypes::ImageComponent)
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
	}
	else if (cType == ComponentTypes::LabelComponent)
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
	}
}

void GLCache::UnRegisterBufferIndex(uint offset, ComponentTypes cType)
{
	if (!isNVIDIA)
		return;

	if (cType == ComponentTypes::ImageComponent)
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
	}
	else if (cType == ComponentTypes::LabelComponent)
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
	}
}
