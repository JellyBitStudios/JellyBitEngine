#include "ShaderImporter.h"

#include "Application.h"
#include "Globals.h"
#include "ModuleFileSystem.h"
#include "ResourceShaderObject.h"
#include "ResourceShaderProgram.h"

#include "glew\include\GL\glew.h"

#include <assert.h>

ShaderImporter::ShaderImporter() {}

ShaderImporter::~ShaderImporter() {}

bool ShaderImporter::SaveShaderObject(ResourceData& data, ResourceShaderObjectData& outputShaderObjectData, std::string& outputFile, bool overwrite) const
{
	bool ret = false;

	if (overwrite)
		outputFile = data.file;
	else
		outputFile = data.name;

	uint size = strlen(outputShaderObjectData.GetSource());
	if (size > 0)
	{
		CONSOLE_LOG(LogTypes::Normal, "SHADER IMPORTER: Successfully read Shader Object '%s'", outputFile.data());
		ret = SaveShaderObject(outputShaderObjectData.GetSource(), size, outputShaderObjectData.shaderObjectType, outputFile, overwrite);
	}
	else
		CONSOLE_LOG(LogTypes::Error, "SHADER IMPORTER: Could not read Shader Object '%s'", outputFile.data());

	return ret;
}

bool ShaderImporter::SaveShaderObject(const void* buffer, uint size, ShaderObjectTypes shaderType, std::string& outputFile, bool overwrite) const
{
	bool ret = false;

	FileTypes fileType = FileTypes::NoFileType;
	switch (shaderType)
	{
	case ShaderObjectTypes::VertexType:
		fileType = FileTypes::VertexShaderObjectFile;
		break;
	case ShaderObjectTypes::FragmentType:
		fileType = FileTypes::FragmentShaderObjectFile;
		break;
	case ShaderObjectTypes::GeometryType:
		fileType = FileTypes::GeometryShaderObjectFile;
		break;
	}

	if (App->fs->SaveInGame((char*)buffer, size, fileType, outputFile, overwrite) > 0)
	{
		CONSOLE_LOG(LogTypes::Normal, "SHADER IMPORTER: Successfully saved Shader Object '%s'", outputFile.data());
		ret = true;
	}
	else
		CONSOLE_LOG(LogTypes::Error, "SHADER IMPORTER: Could not save Shader Object '%s'", outputFile.data());

	return ret;
}

bool ShaderImporter::SaveShaderProgram(ResourceData& data, ResourceShaderProgramData& outputShaderProgramData, std::string& outputFile, bool overwrite) const
{
	bool ret = false;

	if (overwrite)
		outputFile = data.file;
	else
		outputFile = data.name;

	uchar* buffer;
	uint link = ResourceShaderProgram::Link(outputShaderProgramData.shaderObjectsUuids);
	if (link > 0)
	{
		uint size = ResourceShaderProgram::GetBinary(link, &buffer, outputShaderProgramData.format);
		if (size > 0)
		{
			CONSOLE_LOG(LogTypes::Normal, "SHADER IMPORTER: Successfully got Binary Program '%s'", outputFile.data());
			ret = SaveShaderProgram(buffer, size, outputFile, overwrite);
			RELEASE_ARRAY(buffer);
		}
		else
			CONSOLE_LOG(LogTypes::Error, "SHADER IMPORTER: Could not get Binary Program '%s'", outputFile.data());

		ResourceShaderProgram::DeleteShaderProgram(link);
	}
	else
		CONSOLE_LOG(LogTypes::Error, "SHADER IMPORTER: Could not link Shader Program '%s'", outputFile.data());

	return ret;
}

bool ShaderImporter::SaveShaderProgram(const void* buffer, uint size, std::string& outputFile, bool overwrite) const
{
	bool ret = false;

	// Verify that the driver supports at least one shader binary format
	if (GetBinaryFormats() == 0)
		return ret;

	if (App->fs->SaveInGame((char*)buffer, size, FileTypes::ShaderProgramFile, outputFile, overwrite) > 0)
	{
		CONSOLE_LOG(LogTypes::Normal, "SHADER IMPORTER: Successfully saved Binary Program '%s'", outputFile.data());
		ret = true;
	}
	else
		CONSOLE_LOG(LogTypes::Error, "SHADER IMPORTER: Could not save Binary Program '%s'", outputFile.data());

	return ret;
}

bool ShaderImporter::LoadShaderObject(const char* objectFile, ResourceShaderObjectData& outputShaderObjectData, uint& shaderObject) const
{
	bool ret = false;

	assert(objectFile != nullptr);

	char* buffer;
	uint size = App->fs->Load(objectFile, &buffer);
	if (size > 0)
	{
		CONSOLE_LOG(LogTypes::Normal, "SHADER IMPORTER: Successfully loaded Shader Object '%s'", objectFile);
		ret = LoadShaderObject(buffer, size, outputShaderObjectData, shaderObject);
		RELEASE_ARRAY(buffer);
	}
	else
		CONSOLE_LOG(LogTypes::Error, "SHADER IMPORTER: Could not load Shader Object '%s'", objectFile);

	return ret;
}

bool ShaderImporter::LoadShaderObject(const void* buffer, uint size, ResourceShaderObjectData& outputShaderObjectData, uint& shaderObject) const
{
	assert(buffer != nullptr && size > 0);

	char* buf = new char[size + 1];
	memcpy(buf, buffer, size);
	buf[size] = 0;

	outputShaderObjectData.SetSource(buf, size);
	RELEASE_ARRAY(buf);

	// Try to compile the shader object
	shaderObject = ResourceShaderObject::Compile(outputShaderObjectData.GetSource(), outputShaderObjectData.shaderObjectType);

	if (shaderObject > 0)
	{
		CONSOLE_LOG(LogTypes::Normal, "SHADER IMPORTER: New Shader Object loaded with: size %u", size);
		return true;
	}
	else
		CONSOLE_LOG(LogTypes::Error, "SHADER IMPORTER: Shader Object with size %u could not be loaded", size);

	return false;
}

bool ShaderImporter::LoadShaderProgram(const char* programFile, ResourceShaderProgramData& outputShaderProgramData, uint& shaderProgram) const
{
	bool ret = false;

	assert(programFile != nullptr);

	char* buffer;
	uint size = App->fs->Load(programFile, &buffer);
	if (size > 0)
	{
		CONSOLE_LOG(LogTypes::Normal, "SHADER IMPORTER: Successfully loaded Shader Program '%s'", programFile);
		ret = LoadShaderProgram(buffer, size, outputShaderProgramData, shaderProgram);
		RELEASE_ARRAY(buffer);
	}
	else
		CONSOLE_LOG(LogTypes::Error, "SHADER IMPORTER: Could not load Shader Program '%s'", programFile);

	return ret;
}

// ----------------------------------------------------------------------------------------------------

uint ShaderImporter::GetBinaryFormats() const
{
	GLint formats = 0;
	glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &formats);

	return formats;
}

// ----------------------------------------------------------------------------------------------------

bool ShaderImporter::LoadShaderProgram(const void* buffer, uint size, ResourceShaderProgramData& outputShaderProgramData, uint& shaderProgram) const
{
	assert(buffer != nullptr && size > 0);

	// Try to link the shader program
	shaderProgram = ResourceShaderProgram::LoadBinary(buffer, size, outputShaderProgramData.format);

	if (shaderProgram > 0)
	{
		CONSOLE_LOG(LogTypes::Normal, "SHADER IMPORTER: New Shader Program loaded with: size %u", size);
		return true;
	}
	else
		CONSOLE_LOG(LogTypes::Error, "SHADER IMPORTER: Shader Program with size %u could not be loaded", size);

	return false;
}