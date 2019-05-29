#include "ResourceShaderProgram.h"

#include "Application.h"
#include "ModuleFileSystem.h"
#include "ModuleResourceManager.h"
#include "ShaderImporter.h"
#include "ResourceShaderObject.h"
#include "ModuleScene.h"

#include "glew\include\GL\glew.h"

#include "imgui\imgui.h"

#include <assert.h>

ResourceShaderProgram::ResourceShaderProgram(ResourceTypes type, uint uuid, ResourceData data, ResourceShaderProgramData shaderProgramData) : Resource(type, uuid, data), shaderProgramData(shaderProgramData) {}

ResourceShaderProgram::~ResourceShaderProgram()
{
	DeleteShaderProgram(shaderProgram);
}

void ResourceShaderProgram::OnPanelAssets()
{
#ifndef GAMEMODE
	ImGuiTreeNodeFlags flags = 0;
	flags |= ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_Leaf;

	if (App->scene->selectedObject == this)
		flags |= ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_Selected;

	char id[DEFAULT_BUF_SIZE];
	sprintf(id, "%s##%d", data.name.data(), uuid);

	if (ImGui::TreeNodeEx(id, flags))
		ImGui::TreePop();

	if (ImGui::IsMouseReleased(0) && ImGui::IsItemHovered() /*&& (mouseDelta.x == 0 && mouseDelta.y == 0)*/)
	{
		SELECT(this);
	}

	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
	{
		Resource* res = this;
		ImGui::SetDragDropPayload("SHADER_PROGRAM", &res, sizeof(Resource*));
		ImGui::EndDragDropSource();
	}
#endif
}

// ----------------------------------------------------------------------------------------------------

bool ResourceShaderProgram::ImportFile(const char* file, std::string& name, std::vector<uint>& shaderObjectsUuids, ShaderProgramTypes& shaderProgramType, uint& format, std::string& outputFile)
{
	assert(file != nullptr);

	// Search for the meta associated to the file
	char metaFile[DEFAULT_BUF_SIZE];
	strcpy_s(metaFile, strlen(file) + 1, file); // file
	strcat_s(metaFile, strlen(metaFile) + strlen(EXTENSION_META) + 1, EXTENSION_META); // extension

	if (App->fs->Exists(metaFile))
	{
		uint uuid = 0;
		int64_t lastModTime = 0;
		std::string shaderName;
		bool result = ResourceShaderProgram::ReadMeta(metaFile, lastModTime, uuid, shaderName, shaderObjectsUuids, shaderProgramType, format);
		assert(result);

		name = shaderName.data();

		char entry[DEFAULT_BUF_SIZE];
		sprintf_s(entry, "%u", uuid);
		outputFile = entry;
	}

	return true;
}

bool ResourceShaderProgram::ExportFile(ResourceData& data, ResourceShaderProgramData& shaderProgramData, std::string& outputFile, bool overwrite)
{
	return App->shaderImporter->SaveShaderProgram(data, shaderProgramData, outputFile, overwrite);
}

bool ResourceShaderProgram::LoadFile(const char* file, ResourceShaderProgramData& outputShaderProgramData, uint& shaderProgram)
{
	return App->shaderImporter->LoadShaderProgram(file, outputShaderProgramData, shaderProgram);
}

// Returns the last modification time of the file
uint ResourceShaderProgram::CreateMeta(const char* file, uint shaderProgramUuid, std::string& name, std::vector<uint>& shaderObjectsUuids, ShaderProgramTypes shaderProgramType, uint format, std::string& outputMetaFile)
{
	assert(file != nullptr);

	uint uuidsSize = 1;
	uint nameSize = DEFAULT_BUF_SIZE;
	uint shaderObjectsUuidsSize = shaderObjectsUuids.size();
	assert(shaderObjectsUuidsSize > 0);

	// Name
	char shaderName[DEFAULT_BUF_SIZE];
	strcpy_s(shaderName, DEFAULT_BUF_SIZE, name.data());

	uint size =
		sizeof(int64_t) +
		sizeof(uint) +
		sizeof(uint) * uuidsSize +

		sizeof(uint) + // name size
		sizeof(char) * nameSize + // name
		sizeof(uint) + // shader objects uuids size
		sizeof(uint) * shaderObjectsUuidsSize + // shader objects uuids
		sizeof(int) + // shader program type
		sizeof(uint); // binary format

	char* data = new char[size];
	char* cursor = data;

	// 1. Store last modification time
	int64_t lastModTime = App->fs->GetLastModificationTime(file);
	assert(lastModTime > 0);
	uint bytes = sizeof(int64_t);
	memcpy(cursor, &lastModTime, bytes);

	cursor += bytes;

	// 2. Store uuids size
	bytes = sizeof(uint);
	memcpy(cursor, &uuidsSize, bytes);

	cursor += bytes;

	// 3. Store shader program uuid
	bytes = sizeof(uint) * uuidsSize;
	memcpy(cursor, &shaderProgramUuid, bytes);

	cursor += bytes;

	// 4. Store shader program name size
	bytes = sizeof(uint);
	memcpy(cursor, &nameSize, bytes);

	cursor += bytes;

	// 5. Store shader program name
	bytes = sizeof(char) * nameSize;
	memcpy(cursor, shaderName, bytes);

	cursor += bytes;

	// 6. Store shader objects uuids size
	bytes = sizeof(uint);
	memcpy(cursor, &shaderObjectsUuidsSize, bytes);

	cursor += bytes;

	// 7. Store shader objects uuids
	bytes = sizeof(uint) * shaderObjectsUuidsSize;
	memcpy(cursor, &shaderObjectsUuids[0], bytes);
	cursor += bytes;

	// 8. Store shader program type
	bytes = sizeof(int);
	memcpy(cursor, &shaderProgramType, bytes);

	cursor += bytes;

	// 9. Store shader program format
	bytes = sizeof(uint);
	memcpy(cursor, &format, bytes);

	// --------------------------------------------------

	// Build the path of the meta file and save it
	outputMetaFile = file;
	outputMetaFile.append(EXTENSION_META);
	uint resultSize = App->fs->Save(outputMetaFile.data(), data, size);
	if (resultSize > 0)
	{
		CONSOLE_LOG(LogTypes::Normal, "Resource Shader Program: Successfully saved meta '%s'", outputMetaFile.data());
	}
	else
	{
		CONSOLE_LOG(LogTypes::Error, "Resource Shader Program: Could not save meta '%s'", outputMetaFile.data());
		return 0;
	}

	return lastModTime;
}

bool ResourceShaderProgram::ReadMeta(const char* metaFile, int64_t& lastModTime, uint& shaderProgramUuid, std::string& name, std::vector<uint>& shaderObjectsUuids, ShaderProgramTypes& shaderProgramType, uint& format)
{
	assert(metaFile != nullptr);

	char* buffer;
	uint size = App->fs->Load(metaFile, &buffer);
	if (size > 0)
	{
		char* cursor = (char*)buffer;

		// 1. Load last modification time
		uint bytes = sizeof(int64_t);
		memcpy(&lastModTime, cursor, bytes);

		cursor += bytes;

		// 2. Load uuids size
		uint uuidsSize = 0;
		bytes = sizeof(uint);
		memcpy(&uuidsSize, cursor, bytes);
		assert(uuidsSize > 0);

		cursor += bytes;

		// 3. Load shader program uuid
		bytes = sizeof(uint) * uuidsSize;
		memcpy(&shaderProgramUuid, cursor, bytes);

		cursor += bytes;

		// 4. Load shader program name size
		uint nameSize = 0;
		bytes = sizeof(uint);
		memcpy(&nameSize, cursor, bytes);
		assert(nameSize > 0);

		cursor += bytes;

		// 5. Load shader program name
		name.resize(nameSize);
		bytes = sizeof(char) * nameSize;
		memcpy(&name[0], cursor, bytes);

		cursor += bytes;

		// 6. Load shader objects uuids size
		uint shaderObjectsUuidsSize = 0;
		bytes = sizeof(uint);
		memcpy(&shaderObjectsUuidsSize, cursor, bytes);
		assert(shaderObjectsUuidsSize > 0);

		cursor += bytes;

		// 7. Load shader objects uuids
		bytes = sizeof(uint) * shaderObjectsUuidsSize;
		shaderObjectsUuids.resize(shaderObjectsUuidsSize);
		memcpy(&shaderObjectsUuids[0], cursor, bytes);
		cursor += bytes;

		// 8. Load shader program type
		bytes = sizeof(int);
		memcpy(&shaderProgramType, cursor, bytes);

		cursor += bytes;

		// 9. Load shader program format
		bytes = sizeof(uint);
		memcpy(&format, cursor, bytes);

		CONSOLE_LOG(LogTypes::Normal, "Resource Shader Program: Successfully loaded meta '%s'", metaFile);
		RELEASE_ARRAY(buffer);
	}
	else
	{
		CONSOLE_LOG(LogTypes::Error, "Resource Shader Program: Could not load meta '%s'", metaFile);
		return false;
	}

	return true;
}

// Returns the last modification time of the file
uint ResourceShaderProgram::SetNameToMeta(const char* metaFile, const std::string& name)
{
	assert(metaFile != nullptr);

	int64_t lastModTime = 0;
	uint shaderProgramUuid = 0;
	std::string oldName;
	std::vector<uint> shaderObjectsUuids;
	ShaderProgramTypes shaderProgramType = ShaderProgramTypes::Custom;
	uint format = 0;
	ReadMeta(metaFile, lastModTime, shaderProgramUuid, oldName, shaderObjectsUuids, shaderProgramType, format);

	uint uuidsSize = 1;
	uint nameSize = DEFAULT_BUF_SIZE;
	uint shaderObjectsUuidsSize = shaderObjectsUuids.size();
	assert(shaderObjectsUuidsSize > 0);

	// Name
	char shaderName[DEFAULT_BUF_SIZE];
	strcpy_s(shaderName, DEFAULT_BUF_SIZE, name.data());

	uint size =
		sizeof(int64_t) +
		sizeof(uint) +
		sizeof(uint) * uuidsSize +

		sizeof(uint) + // name size
		sizeof(char) * nameSize + // name
		sizeof(uint) + // shader objects uuids size
		sizeof(uint) * shaderObjectsUuidsSize + // shader objects uuids
		sizeof(int) + // shader program type
		sizeof(uint); // binary format

	char* data = new char[size];
	char* cursor = data;

	// 1. Store last modification time
	uint bytes = sizeof(int64_t);
	memcpy(cursor, &lastModTime, bytes);

	cursor += bytes;

	// 2. Store uuids size
	bytes = sizeof(uint);
	memcpy(cursor, &uuidsSize, bytes);

	cursor += bytes;

	// 3. Store shader program uuid
	bytes = sizeof(uint) * uuidsSize;
	memcpy(cursor, &shaderProgramUuid, bytes);

	cursor += bytes;

	// 4. Store shader program name size
	bytes = sizeof(uint);
	memcpy(cursor, &nameSize, bytes);

	cursor += bytes;

	// 5. Store shader program name
	bytes = sizeof(char) * nameSize;
	memcpy(cursor, shaderName, bytes);

	cursor += bytes;

	// 6. Store shader objects uuids size
	bytes = sizeof(uint);
	memcpy(cursor, &shaderObjectsUuidsSize, bytes);

	cursor += bytes;

	// 7. Store shader objects uuids
	bytes = sizeof(uint) * shaderObjectsUuidsSize;
	memcpy(cursor, &shaderObjectsUuids[0], bytes);
	cursor += bytes;

	// 8. Store shader program type
	bytes = sizeof(int);
	memcpy(cursor, &shaderProgramType, bytes);

	cursor += bytes;

	// 9. Store shader program format
	bytes = sizeof(uint);
	memcpy(cursor, &format, bytes);

	// --------------------------------------------------

	// Build the path of the meta file and save it
	uint resultSize = App->fs->Save(metaFile, data, size);
	if (resultSize > 0)
	{
		CONSOLE_LOG(LogTypes::Normal, "Resource Shader Program: Successfully saved meta '%s'", metaFile);
	}
	else
	{
		CONSOLE_LOG(LogTypes::Error, "Resource Shader Program: Could not save meta '%s'", metaFile);
		return 0;
	}

	return lastModTime;
}

bool ResourceShaderProgram::GenerateLibraryFiles() const
{
	assert(data.file.data() != nullptr);

	// Search for the meta associated to the file
	char metaFile[DEFAULT_BUF_SIZE];
	strcpy_s(metaFile, strlen(data.file.data()) + 1, data.file.data()); // file
	strcat_s(metaFile, strlen(metaFile) + strlen(EXTENSION_META) + 1, EXTENSION_META); // extension

	// 1. Copy meta
	if (App->fs->Exists(metaFile))
	{
		std::string outputFile;
		uint size = App->fs->Copy(metaFile, DIR_LIBRARY_SHADERS_PROGRAMS, outputFile);

		if (size > 0)
		{
			// 2. Copy shader program
			outputFile.clear();
			uint size = App->fs->Copy(data.file.data(), DIR_LIBRARY_SHADERS_PROGRAMS, outputFile);

			if (size > 0)
				return true;
		}
	}

	return false;
}

// ----------------------------------------------------------------------------------------------------

bool ResourceShaderProgram::Link()
{
	bool ret = true;

	// Create a Shader Program
	DeleteShaderProgram(shaderProgram);
	shaderProgram = glCreateProgram();

	for (uint i = 0; i < shaderProgramData.shaderObjectsUuids.size(); ++i)
	{
		ResourceShaderObject* shaderObject = (ResourceShaderObject*)App->res->GetResource(shaderProgramData.shaderObjectsUuids[i]);
		glAttachShader(shaderProgram, shaderObject->shaderObject);
	}

	// Link the Shader Program
	glLinkProgram(shaderProgram);

	for (uint i = 0; i < shaderProgramData.shaderObjectsUuids.size(); ++i)
	{
		ResourceShaderObject* shaderObject = (ResourceShaderObject*)App->res->GetResource(shaderProgramData.shaderObjectsUuids[i]);
		glDetachShader(shaderProgram, shaderObject->shaderObject);
	}

	if (!IsProgramLinked(shaderProgram))
	{
		DeleteShaderProgram(shaderProgram);
		ret = false;
	}

	return ret;
}

uint ResourceShaderProgram::Link(const std::vector<uint>& shaderObjectsUuids)
{
	// Create a Shader Program
	GLuint shaderProgram = glCreateProgram();

	for (uint i = 0; i < shaderObjectsUuids.size(); ++i)
	{
		ResourceShaderObject* shaderObject = (ResourceShaderObject*)App->res->GetResource(shaderObjectsUuids[i]);
		glAttachShader(shaderProgram, shaderObject->shaderObject);
	}

	// Link the Shader Program
	glLinkProgram(shaderProgram);

	for (uint i = 0; i < shaderObjectsUuids.size(); ++i)
	{
		ResourceShaderObject* shaderObject = (ResourceShaderObject*)App->res->GetResource(shaderObjectsUuids[i]);
		glDetachShader(shaderProgram, shaderObject->shaderObject);
	}

	GLint success = 0;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (success == GL_FALSE)
	{
		GLint logSize = 0;
		glGetShaderiv(shaderProgram, GL_INFO_LOG_LENGTH, &logSize);

		GLchar* infoLog = new GLchar[logSize];
		glGetProgramInfoLog(shaderProgram, logSize, NULL, infoLog);

		CONSOLE_LOG(LogTypes::Error, "Shader Program could not be linked. ERROR: %s", infoLog);

		DeleteShaderProgram(shaderProgram);
	}
	else
		CONSOLE_LOG(LogTypes::Normal, "Successfully linked Shader Program");

	return shaderProgram;
}

// Returns the length of the binary
uint ResourceShaderProgram::GetBinary(uint shaderProgram, uchar** buffer, uint& format)
{
	// Get the binary length
	GLint length = 0;
	glGetProgramiv(shaderProgram, GL_PROGRAM_BINARY_LENGTH, &length);

	if (length > 0)
	{
		// Get the binary code
		*buffer = new uchar[length];

		glGetProgramBinary(shaderProgram, length, NULL, &format, *buffer);
	}

	return length;
}

uint ResourceShaderProgram::LoadBinary(const void* buffer, int size, uint format)
{
	// Create a Shader Program
	GLuint shaderProgram = glCreateProgram();

	// Install the binary
	glProgramBinary(shaderProgram, format, buffer, size);

	if (!IsProgramLinked(shaderProgram))
		DeleteShaderProgram(shaderProgram);

	return shaderProgram;
}

bool ResourceShaderProgram::DeleteShaderProgram(uint& shaderProgram)
{
	bool ret = false;

	if (glIsProgram(shaderProgram))
	{
		glDeleteProgram(shaderProgram);
		shaderProgram = 0;
		ret = true;
	}

	return ret;
}

// ----------------------------------------------------------------------------------------------------

void ResourceShaderProgram::SetShaderProgramType(ShaderProgramTypes shaderProgramType)
{
	shaderProgramData.shaderProgramType = shaderProgramType;
}

ShaderProgramTypes ResourceShaderProgram::GetShaderProgramType() const
{
	return shaderProgramData.shaderProgramType;
}

void ResourceShaderProgram::SetShaderObjects(const std::vector<uint>& shaderObjectsUuids)
{
	shaderProgramData.shaderObjectsUuids = shaderObjectsUuids;
}

void ResourceShaderProgram::GetShaderObjects(std::vector<uint>& shaderObjectsUuids, ShaderObjectTypes shaderType) const
{
	for (uint i = 0; i < shaderProgramData.shaderObjectsUuids.size(); ++i)
	{
		ResourceShaderObject* shaderObject = (ResourceShaderObject*)App->res->GetResource(shaderProgramData.shaderObjectsUuids[i]);

		if (shaderType == ShaderObjectTypes::NoShaderObjectType || shaderType == shaderObject->GetShaderObjectType())
			shaderObjectsUuids.push_back(shaderProgramData.shaderObjectsUuids[i]);
	}
}

void ResourceShaderProgram::GetUniforms(std::vector<Uniform>& uniforms)
{
	int count;
	glGetProgramiv(shaderProgram, GL_ACTIVE_UNIFORMS, &count);
	if (count == 0)
		return;

	uniforms.clear();
	uniforms.reserve(count);

	GLuint program;
	GLuint index;
	GLsizei length;
	GLint size;
	GLenum type;
	GLchar name[DEFAULT_BUF_SIZE];

	for (uint i = 0; i < count; ++i)
	{
		glGetActiveUniform(shaderProgram, i, DEFAULT_BUF_SIZE, &length, &size, &type, name);

		for (uint j = 0; j < shaderProgramData.ignoreUniforms.size(); ++j)
		{
			if (strcmp(name, shaderProgramData.ignoreUniforms[j]) == 0)
				goto hereWeGo;
		}

		if (strcmp(name, "animate") == 0
			|| strcmp(name, "fog.color") == 0
			|| strcmp(name, "fog.density") == 0
			|| strcmp(name, "gInfoTexture") == 0
			|| strcmp(name, "dot") == 0
			|| strcmp(name, "screenSize") == 0
			|| strcmp(name, "Time") == 0)
			continue;

		Uniform uniform;
		memset(&uniform, 0, sizeof(Uniform));
		switch (type)
		{
		case Uniforms_Values::FloatU_value:
			strcpy_s(uniform.floatU.name, name);
			uniform.floatU.type = type;
			uniform.floatU.location = glGetUniformLocation(shaderProgram, uniform.common.name);
			uniforms.push_back(uniform);
			break;
		case Uniforms_Values::IntU_value:
			strcpy_s(uniform.intU.name, name);
			uniform.intU.type = type;
			uniform.intU.location = glGetUniformLocation(shaderProgram, uniform.common.name);
			uniforms.push_back(uniform);
			break;
		case Uniforms_Values::Vec2FU_value:
			strcpy_s(uniform.vec2FU.name, name);
			uniform.vec2FU.type = type;
			uniform.vec2FU.location = glGetUniformLocation(shaderProgram, uniform.common.name);
			uniforms.push_back(uniform);
			break;
		case Uniforms_Values::Vec3FU_value:
			strcpy_s(uniform.vec3FU.name, name);
			uniform.vec3FU.type = type;
			uniform.vec3FU.location = glGetUniformLocation(shaderProgram, uniform.common.name);
			uniforms.push_back(uniform);
			break;
		case Uniforms_Values::Vec4FU_value:
			strcpy_s(uniform.vec4FU.name, name);
			uniform.vec4FU.type = type;
			uniform.vec4FU.location = glGetUniformLocation(shaderProgram, uniform.common.name);
			uniforms.push_back(uniform);
			break;
		case Uniforms_Values::Vec2IU_value:
			strcpy_s(uniform.vec2IU.name, name);
			uniform.vec2IU.type = type;
			uniform.vec2IU.location = glGetUniformLocation(shaderProgram, uniform.common.name);
			uniforms.push_back(uniform);
			break;
		case Uniforms_Values::Vec3IU_value:
			strcpy_s(uniform.vec3IU.name, name);
			uniform.vec3IU.type = type;
			uniform.vec3IU.location = glGetUniformLocation(shaderProgram, uniform.common.name);
			uniforms.push_back(uniform);
			break;
		case Uniforms_Values::Vec4IU_value:
			strcpy_s(uniform.vec4IU.name, name);
			uniform.vec4IU.type = type;
			uniform.vec4IU.location = glGetUniformLocation(shaderProgram, uniform.common.name);
			uniforms.push_back(uniform);
			break;
		case Uniforms_Values::Sampler2U_value:
			strcpy_s(uniform.sampler2DU.name, name);
			uniform.sampler2DU.type = type;
			uniform.sampler2DU.location = glGetUniformLocation(shaderProgram, uniform.common.name);
			uniforms.push_back(uniform);
			break;
		}

	hereWeGo:;
	}

	uniforms.shrink_to_fit();
}

void ResourceShaderProgram::GetIgnoreUniforms(std::vector<const char*>& ignoreUniforms)
{
	ignoreUniforms = shaderProgramData.ignoreUniforms;
}

// ----------------------------------------------------------------------------------------------------

bool ResourceShaderProgram::LoadInMemory()
{
	return true;
}

bool ResourceShaderProgram::UnloadFromMemory()
{
	return true;
}

bool ResourceShaderProgram::IsProgramValid(uint shaderProgram)
{
	GLint success = 0;
	glGetProgramiv(shaderProgram, GL_VALIDATE_STATUS, &success);
	if (success == GL_FALSE)
	{
		GLint logSize = 0;
		glGetShaderiv(shaderProgram, GL_INFO_LOG_LENGTH, &logSize);

		GLchar* infoLog = new GLchar[logSize];
		glGetProgramInfoLog(shaderProgram, logSize, NULL, infoLog);

		CONSOLE_LOG(LogTypes::Error, "Shader Program is not valid. ERROR: %s", infoLog);
	}
	else
		CONSOLE_LOG(LogTypes::Normal, "Shader Program is valid");

	return success;
}

bool ResourceShaderProgram::IsProgramLinked(uint shaderProgram)
{
	GLint success = 0;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (success == GL_FALSE)
	{
		GLint logSize = 512;
		//glGetShaderiv(shaderProgram, GL_INFO_LOG_LENGTH, &logSize);

		GLchar* infoLog = new GLchar[logSize];
		glGetProgramInfoLog(shaderProgram, logSize, NULL, infoLog);

		CONSOLE_LOG(LogTypes::Error, "Shader Program could not be linked. ERROR: %s", infoLog);
	}
	else
		CONSOLE_LOG(LogTypes::Normal, "Successfully linked Shader Program");

	return success;
}