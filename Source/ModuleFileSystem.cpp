#include "ModuleFileSystem.h"

#include "Application.h"
#include "Globals.h"

#include "SceneImporter.h"
#include "MaterialImporter.h"
#include "ShaderImporter.h"
#include "ModuleResourceManager.h"
#include "ResourceMesh.h"
#include "ModuleScene.h"
#include "ModuleTimeManager.h"

#include "physfs\include\physfs.h"
#include "libzip/include/zip.h"

#include <assert.h>

#include <stdio.h>

#pragma comment(lib, "physfs/libx86/physfs.lib")

#ifdef _DEBUG
#pragma comment( lib, "libzip/zip_d.lib" )
#else
#pragma comment( lib, "libzip/zip_r.lib" )
#endif // _DEBUG

ModuleFileSystem::ModuleFileSystem(bool start_enabled) : Module(start_enabled)
{
	name = "FileSystem";

	if (PHYSFS_init(nullptr) == 0)
	{
		const char* error = PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode());
		CONSOLE_LOG(LogTypes::Error, "Could not initialize PHYSFS. Error: %s", error);
	}

	AddPath(".");
#ifndef GAMEMODE
	AddPath("./Assets/", "Assets");
#endif
	AddPath("./Settings/", "Settings");

	//Internal Directory: Engine stuff
	AddPath("./internal.f", "Internal");

	if (PHYSFS_setWriteDir(".") == 0)
		CONSOLE_LOG(LogTypes::Error, "Could not set Write Dir. ERROR: %s", PHYSFS_getLastError());

#ifndef GAMEMODE
	CreateDir(DIR_ASSETS_MESHES);
	CreateDir(DIR_ASSETS_TEXTURES);
	CreateDir(DIR_ASSETS_MATERIALS);
	CreateDir(DIR_ASSETS_AVATARS);
	CreateDir(DIR_ASSETS_ANIMATORS);
	CreateDir(DIR_ASSETS_SHADERS);
	CreateDir(DIR_ASSETS_SHADERS_OBJECTS);
	CreateDir(DIR_ASSETS_SHADERS_PROGRAMS);
	CreateDir(DIR_ASSETS_PREFAB);
	CreateDir(DIR_ASSETS_SCENES);
	CreateDir(DIR_ASSETS_SCRIPTS);
	CreateDir(DIR_ASSETS_AUDIO);
	CreateDir(DIR_ASSETS_FONT);
#endif

	if (CreateDir(DIR_LIBRARY))
	{
		AddPath("./Library/", "Library");

		CreateDir(DIR_LIBRARY_FONT);
		CreateDir(DIR_LIBRARY_MESHES);
		CreateDir(DIR_LIBRARY_ANIMATIONS);
		CreateDir(DIR_LIBRARY_ANIMATORS);
		CreateDir(DIR_LIBRARY_AVATARS);
		CreateDir(DIR_LIBRARY_BONES);
		CreateDir(DIR_LIBRARY_TEXTURES);
		CreateDir(DIR_LIBRARY_MATERIALS);
		CreateDir(DIR_LIBRARY_SHADERS);
		CreateDir(DIR_LIBRARY_SHADERS_OBJECTS);
		CreateDir(DIR_LIBRARY_SHADERS_PROGRAMS);
		CreateDir(DIR_LIBRARY_PREFAB);
		CreateDir(DIR_LIBRARY_SCENES);
		CreateDir(DIR_LIBRARY_SCRIPTS);
		CreateDir(DIR_LIBRARY_AUDIO);
	}
}

ModuleFileSystem::~ModuleFileSystem() {}

update_status ModuleFileSystem::PreUpdate()
{
	// Now is a button.

	return update_status::UPDATE_CONTINUE;
}

bool ModuleFileSystem::Init(JSON_Object * data)
{
	AddPath(GetPrefDir(), "PrefDir");
	return true;
}

bool ModuleFileSystem::Start()
{
#ifndef GAMEMODE
	rootDir = RecursiveGetFilesFromDir("Assets");
#else
	rootDir = RecursiveGetFilesFromDir("Library");
#endif

	ImportMainDir();

#ifdef GAMEMODE
	System_Event event;
	event.type = System_Event_Type::LoadGMScene;
	App->PushSystemEvent(event);

#endif
	

#ifndef GAMEMODE
	System_Event event;
	event.type = System_Event_Type::DeleteUnusedFiles;
	App->PushSystemEvent(event);
#endif

	return true;
}

bool ModuleFileSystem::CleanUp()
{
	CONSOLE_LOG(LogTypes::Normal, "Freeing File System subsystem");
	PHYSFS_deinit();

	return true;
}

void ModuleFileSystem::ImportMainDir(bool reimport)
{
	std::vector<std::string> lateEvents;
	std::vector<std::string> lateLateEvents;
	ImportFilesEvents(rootDir, lateEvents, lateLateEvents, reimport);

	for (int i = 0; i < lateEvents.size(); ++i)
	{
		// The ResourceManager already manages the .meta, already imported files etc. on his own.
		System_Event event;
#ifndef GAMEMODE
		event.fileEvent.type = reimport ? System_Event_Type::ReImportFile : System_Event_Type::ImportFile;
#else
		event.fileEvent.type = System_Event_Type::ImportLibraryFile;
#endif
		event.fileEvent.build = false;
		strcpy(event.fileEvent.file, lateEvents[i].data());
		App->PushSystemEvent(event);
	}

	for (int i = 0; i < lateLateEvents.size(); ++i)
	{
		// The ResourceManager already manages the .meta, already imported files etc. on his own.
		System_Event event;
#ifndef GAMEMODE
		event.fileEvent.type = reimport ? System_Event_Type::ReImportFile : System_Event_Type::ImportFile;
#else
		event.fileEvent.type = System_Event_Type::ImportLibraryFile;
#endif
		event.fileEvent.build = false;
		strcpy(event.fileEvent.file, lateLateEvents[i].data());
		App->PushSystemEvent(event);
	}
}

void ModuleFileSystem::OnSystemEvent(System_Event event)
{
	switch (event.type)
	{
		case System_Event_Type::Build:
		{
			if (build)
			{
#ifndef GAMEMODE
				char pathBuild[DEFAULT_BUF_SIZE];
				strcpy(pathBuild, "Build");
				//std::string zip_path = PHYSFS_getBaseDir();
				//zip_path += "Build.zip";
				Directory root = RecursiveGetFilesFromDir("");
				//if (Exists("Build.zip"))
					//deleteFile("Build.zip");
				if (Exists(pathBuild))
					deleteFiles(pathBuild, "", true, true);

				if (CreateDir(pathBuild))
					RecursiveBuild(root, pathBuild);

				build = false;
#endif			
			}
			break;
		}
	
		case System_Event_Type::FileDropped:
		{
#ifndef GAMEMODE
			char* fileOrigin = event.fileEvent.file;

			std::string extension;
			App->fs->GetExtension(fileOrigin, extension);

			ResourceTypes type = App->res->GetResourceTypeByExtension(extension.data());

			char destinationDir[DEFAULT_BUF_SIZE];

			switch (type)
			{
				case ResourceTypes::MeshResource:
				{
					strcpy(destinationDir, DIR_ASSETS_MESHES);
					break;
				}
				case ResourceTypes::ShaderObjectResource:
				{
					strcpy(destinationDir, DIR_ASSETS_SHADERS_OBJECTS);
					break;
				}
				case ResourceTypes::ShaderProgramResource:
				{
					strcpy(destinationDir, DIR_ASSETS_SHADERS_PROGRAMS);
					break;
				}
				case ResourceTypes::ScriptResource:
				{
					strcpy(destinationDir, DIR_ASSETS_SCRIPTS);
					break;
				}
				case ResourceTypes::TextureResource:
				{
					strcpy(destinationDir, DIR_ASSETS_TEXTURES);
					break;
				}
				case ResourceTypes::FontResource:
				{
					strcpy(destinationDir, DIR_ASSETS_FONT);
					break;
				}
				// TODO ADD NEW RESOURCES
			}

			std::string fileName;
			App->fs->GetFileName(fileOrigin, fileName, true);

			char destinationFile[DEFAULT_BUF_SIZE];
			strcpy(destinationFile, destinationDir);
			strcat(destinationFile, "//");
			strcat(destinationFile, fileName.data());

			std::string originDir = fileOrigin;
			originDir = originDir.substr(0, originDir.find_last_of("\\"));

			BeginTempException(originDir);
			char originExFile[DEFAULT_BUF_SIZE];
			sprintf(originExFile, "Exception/%s", fileName.data());
			MoveFileInto(originExFile, destinationFile);
			EndTempException();
#endif
			break;
		}
	}
}

bool ModuleFileSystem::CreateDir(const char* dirName) const
{
	bool ret = true;

	ret = PHYSFS_mkdir(dirName) != 0;

	if (ret)
	{
		CONSOLE_LOG(LogTypes::Normal, "FILE SYSTEM: Successfully created the directory '%s'", dirName);
	}
	else
		CONSOLE_LOG(LogTypes::Error, "FILE SYSTEM: Couldn't create the directory '%s'. ERROR: %s", dirName, PHYSFS_getLastError());

	return ret;
}

bool ModuleFileSystem::AddPath(const char* newDir, const char* mountPoint) const
{
	bool ret = false;

	if (PHYSFS_mount(newDir, mountPoint, 1) != 0)
		ret = true;
	else
		CONSOLE_LOG(LogTypes::Error, "FILE SYSTEM: Error while adding a path or zip '%s': %s", newDir, PHYSFS_getLastError());

	return ret;
}

bool ModuleFileSystem::DeleteFileOrDir(const char* path) const
{
	bool ret = false;

	if (PHYSFS_delete(path) != 0)
		ret = true;
	else
		CONSOLE_LOG(LogTypes::Error, "FILE SYSTEM: Error while deleting a file or directory '%s': %s", path, PHYSFS_getLastError());

	return ret;
}

const char* ModuleFileSystem::GetBasePath() const
{
	return PHYSFS_getBaseDir();
}

const char* ModuleFileSystem::GetPrefDir() const
{
	return PHYSFS_getPrefDir(App->GetOrganizationName(), App->GetAppName());
}

const char* ModuleFileSystem::GetReadPaths() const
{
	static char paths[MAX_BUF_SIZE];
	paths[0] = '\0'; // null-terminated byte string

	static char tmp_string[MAX_BUF_SIZE];

	char** path;
	for (path = PHYSFS_getSearchPath(); *path != nullptr; ++path)
	{
		sprintf_s(tmp_string, MAX_BUF_SIZE, "%s", *path);
		strcat_s(paths, MAX_BUF_SIZE, tmp_string);
	}

	return paths;
}

const char* ModuleFileSystem::GetWritePath() const
{
	return PHYSFS_getWriteDir();
}

const char** ModuleFileSystem::GetFilesFromDir(const char* dir) const
{
	return (const char**)PHYSFS_enumerateFiles(dir);
}

bool ModuleFileSystem::IsDirectory(const char* file) const
{
	return PHYSFS_isDirectory(file);
}

bool ModuleFileSystem::Exists(std::string file) const
{
	return PHYSFS_exists(file.data());
}

bool ModuleFileSystem::RecursiveExists(const char* fileName, const char* dir, std::string& path) const
{
	assert(dir != nullptr);

	path.append("/");

	std::string file = path;
	file.append(fileName);

	bool exists = Exists(file.data());

	if (exists)
	{
		path.append(fileName);
		return exists;
	}

	const char** files = GetFilesFromDir(path.data());
	const char** it;

	for (it = files; *it != nullptr; ++it)
	{
		path.append(*it);

		if (IsDirectory(path.data()))
		{
			exists = RecursiveExists(fileName, *it, path);

			if (exists)
				return exists;
		}

		uint found = path.rfind(*it);
		if (found != std::string::npos)
			path = path.substr(0, found);
	}

	return exists;
}

int64_t ModuleFileSystem::GetLastModificationTime(const char* file) const
{
	return PHYSFS_getLastModTime(file);
}

void ModuleFileSystem::GetFileName(const char* file, std::string& fileName, bool extension) const
{
	fileName = file;

	int found = fileName.find_last_of("\\");
	if (found != std::string::npos)
		fileName = fileName.substr(found + 1, fileName.size());

	found = fileName.find_last_of("//");
	if (found != std::string::npos)
		fileName = fileName.substr(found + 1, fileName.size());

	if (!extension)
	{
		found = fileName.find_last_of(".");
		if (found != std::string::npos)
			fileName = fileName.substr(0, found);
	}
}

void ModuleFileSystem::GetExtension(const char* file, std::string& extension) const
{
	extension = file;

	int found = extension.find_last_of(".");
	if (found != std::string::npos)
		extension = extension.substr(found);
}

void ModuleFileSystem::GetMetaExtension(const char* file, std::string& extension) const
{
	std::string ex = file;
	extension = ex;

	int foundMeta = extension.find_last_of(".");
	if (foundMeta != std::string::npos)
	{
		ex = ex.substr(foundMeta);
		extension = extension.substr(0, foundMeta);
	}
	int foundExtension = extension.find_last_of(".");
	if (foundExtension != std::string::npos)
		extension = extension.substr(foundExtension);
}

void ModuleFileSystem::GetFileFromMeta(const char* metaFile, std::string& file) const
{
	file = metaFile;

	int foundMeta = file.find_last_of(".");
	if (foundMeta != std::string::npos)
		file = file.substr(0, foundMeta);
}

void ModuleFileSystem::GetPath(const char* file, std::string& path, bool bar) const
{
	path = file;

	uint add = bar ? 1 : 0;

	int found = path.find_last_of("\\");
	if (found != std::string::npos)
		path = path.substr(0, found + add);

	found = path.find_last_of("//");
	if (found != std::string::npos)
		path = path.substr(0, found + add);
}

uint ModuleFileSystem::Copy(const char* file, const char* dir, std::string& outputFile) const
{
	assert(file != nullptr && dir != nullptr);

	uint size = 0;

	std::FILE* filehandle;
	fopen_s(&filehandle, file, "rb");

	if (filehandle != nullptr)
	{
		fseek(filehandle, 0, SEEK_END);
		size = ftell(filehandle);
		rewind(filehandle);

		char* buffer = new char[size];
		size = fread(buffer, 1, size, filehandle);
		if (size > 0)
		{
			GetFileName(file, outputFile, true);
			outputFile.insert(0, "/");
			outputFile.insert(0, dir);

			size = Save(outputFile.data(), buffer, size);
			if (size > 0)
			{
				CONSOLE_LOG(LogTypes::Normal, "FILE SYSTEM: Successfully copied file '%s' in dir '%s'", file, dir);
			}
			else
				CONSOLE_LOG(LogTypes::Error, "FILE SYSTEM: Could not copy file '%s' in dir '%s'", file, dir);
		}
		else
			CONSOLE_LOG(LogTypes::Error, "FILE SYSTEM: Could not read from file '%s'", file);

		RELEASE_ARRAY(buffer);
		fclose(filehandle);
	}
	else
		CONSOLE_LOG(LogTypes::Error, "FILE SYSTEM: Could not open file '%s' to read", file);

	return size;
}

uint ModuleFileSystem::SaveInGame(char* buffer, uint size, FileTypes fileType, std::string& outputFile, bool overwrite) const
{
	uint ret = 0;

	if (!overwrite)
	{
		switch (fileType)
		{
		case FileTypes::MeshFile:
			outputFile.insert(0, DIR_LIBRARY_MESHES);
			outputFile.insert(strlen(DIR_LIBRARY_MESHES), "/");
			outputFile.append(EXTENSION_MESH);
			break;
		case FileTypes::AnimationFile:
			outputFile.insert(0, DIR_LIBRARY_ANIMATIONS);
			outputFile.insert(strlen(DIR_LIBRARY_ANIMATIONS), "/");
			outputFile.append(EXTENSION_ANIMATION);
			break;
		case FileTypes::AnimatorFile:
			outputFile.insert(0, DIR_ASSETS_ANIMATORS);
			outputFile.insert(strlen(DIR_ASSETS_ANIMATORS), "/");
			outputFile.append(EXTENSION_ANIMATOR);
			break;
		case FileTypes::BoneFile:
			outputFile.insert(0, DIR_LIBRARY_BONES);
			outputFile.insert(strlen(DIR_LIBRARY_BONES), "/");
			outputFile.append(EXTENSION_BONE);
			break;
		case FileTypes::TextureFile:
			outputFile.insert(0, DIR_LIBRARY_TEXTURES);
			outputFile.insert(strlen(DIR_LIBRARY_TEXTURES), "/");
			outputFile.append(EXTENSION_TEXTURE);
			break;
		case FileTypes::MaterialFile:
			outputFile.insert(0, DIR_ASSETS_MATERIALS);
			outputFile.insert(strlen(DIR_ASSETS_MATERIALS), "/");
			outputFile.append(EXTENSION_MATERIAL);
			break;
		case FileTypes::AvatarFile:
			outputFile.insert(0, DIR_ASSETS_AVATARS);
			outputFile.insert(strlen(DIR_ASSETS_AVATARS), "/");
			outputFile.append(EXTENSION_AVATAR);
			break;
		case FileTypes::VertexShaderObjectFile:
			outputFile.insert(0, DIR_ASSETS_SHADERS_OBJECTS);
			outputFile.insert(strlen(DIR_ASSETS_SHADERS_OBJECTS), "/");
			outputFile.append(EXTENSION_VERTEX_SHADER_OBJECT);
			break;
		case FileTypes::FragmentShaderObjectFile:
			outputFile.insert(0, DIR_ASSETS_SHADERS_OBJECTS);
			outputFile.insert(strlen(DIR_ASSETS_SHADERS_OBJECTS), "/");
			outputFile.append(EXTENSION_FRAGMENT_SHADER_OBJECT);
			break;
		case FileTypes::GeometryShaderObjectFile:
			outputFile.insert(0, DIR_ASSETS_SHADERS_OBJECTS);
			outputFile.insert(strlen(DIR_ASSETS_SHADERS_OBJECTS), "/");
			outputFile.append(EXTENSION_GEOMETRY_SHADER_OBJECT);
			break;
		case FileTypes::ShaderProgramFile:
			outputFile.insert(0, DIR_ASSETS_SHADERS_PROGRAMS);
			outputFile.insert(strlen(DIR_ASSETS_SHADERS_PROGRAMS), "/");
			outputFile.append(EXTENSION_SHADER_PROGRAM);
			break;
		case FileTypes::PrefabFile:
			outputFile.insert(0, DIR_ASSETS_PREFAB);
			outputFile.insert(strlen(DIR_ASSETS_PREFAB), "/");
			outputFile.append(EXTENSION_PREFAB);
			break;
		case FileTypes::SceneFile:
			outputFile.insert(0, DIR_ASSETS_SCENES);
			outputFile.insert(strlen(DIR_ASSETS_SCENES), "/");
			outputFile.append(EXTENSION_SCENE);
			break;
		case FileTypes::FontFile:
			outputFile.insert(0, DIR_LIBRARY_FONT);
			outputFile.insert(strlen(DIR_LIBRARY_FONT), "/");
			outputFile.append(EXTENSION_FONT);
			break;
		}
	}

	ret = Save(outputFile.data(), buffer, size);

	return ret;
}

uint ModuleFileSystem::Save(std::string file, char* buffer, uint size, bool append) const
{
	uint objCount = 0;

	std::string fileName;
	GetFileName(file.data(), fileName, true);

	bool exists = Exists(file.data());

	PHYSFS_file* filehandle = nullptr;
	if (append)
		filehandle = PHYSFS_openAppend(file.data());
	else
		filehandle = PHYSFS_openWrite(file.data());

	if (filehandle != nullptr)
	{
		objCount = PHYSFS_writeBytes(filehandle, (const void*)buffer, size);

		if (objCount == size)
		{
			if (exists)
			{
				if (append)
				{
					CONSOLE_LOG(LogTypes::Normal, "FILE SYSTEM: Append %u bytes to file '%s'", objCount, fileName.data());
				}
				else
					CONSOLE_LOG(LogTypes::Normal, "FILE SYSTEM: File '%s' overwritten with %u bytes", fileName.data(), objCount);
			}
			else
				CONSOLE_LOG(LogTypes::Normal, "FILE SYSTEM: New file '%s' created with %u bytes", fileName.data(), objCount);
		}
		else
			CONSOLE_LOG(LogTypes::Error, "FILE SYSTEM: Could not write to file '%s'. ERROR: %s", fileName.data(), PHYSFS_getLastError());

		if (PHYSFS_close(filehandle) == 0)
			CONSOLE_LOG(LogTypes::Error, "FILE SYSTEM: Could not close file '%s'. ERROR: %s", fileName.data(), PHYSFS_getLastError());
	}
	else
		CONSOLE_LOG(LogTypes::Error, "FILE SYSTEM: Could not open file '%s' to write. ERROR: %s", fileName.data(), PHYSFS_getLastError());

	return objCount;
}

void ModuleFileSystem::WriteFile(const char * zip_path, const char * filename, const char * buffer, unsigned int size)
{
	if (PHYSFS_removeFromSearchPath(zip_path) == 0)
		CONSOLE_LOG(LogTypes::Error, "Errot when unmount: %s", PHYSFS_getLastError());

	struct zip *f_zip = NULL;
	int error = 0;
	f_zip = zip_open(zip_path, ZIP_CHECKCONS + ZIP_CREATE, &error); /* on ouvre l'archive zip */
	if (error)
	{
		zip_error zerror;
		zip_error_init_with_code(&zerror, error);
		CONSOLE_LOG(LogTypes::Error, "could not open or create archive: %s, %s", zip_path, zip_error_strerror(&zerror));
	}
	else
	{
		zip_source_t *s;

		s = zip_source_buffer(f_zip, buffer, size, 0);

		if (s == NULL ||
			zip_file_add(f_zip, filename, s, ZIP_FL_OVERWRITE + ZIP_FL_ENC_UTF_8) < 0)
		{
			zip_source_free(s);
			CONSOLE_LOG(LogTypes::Error, "error adding file: %s\n", zip_strerror(f_zip));
		}
	}

	zip_close(f_zip);
	f_zip = NULL;

	PHYSFS_mount(zip_path, NULL, 1);
}

uint ModuleFileSystem::Load(std::string file, char** buffer) const
{
	uint objCount = 0;

	std::string fileName;
	GetFileName(file.data(), fileName, true);

	bool exists = Exists(file.data());

	if (exists)
	{
		PHYSFS_file* filehandle = PHYSFS_openRead(file.data());

		if (filehandle != nullptr)
		{
			PHYSFS_sint64 size = PHYSFS_fileLength(filehandle);

			if (size > 0)
			{
				*buffer = new char[size];
				objCount = PHYSFS_readBytes(filehandle, *buffer, size);

				if (objCount == size)
				{
					//CONSOLE_LOG("FILE SYSTEM: Read %u bytes from file '%s'", objCount, fileName.data());
				}
				else
				{
					RELEASE(buffer);
					CONSOLE_LOG(LogTypes::Error, "FILE SYSTEM: Could not read from file '%s'. ERROR: %s", fileName.data(), PHYSFS_getLastError());
				}

				if (PHYSFS_close(filehandle) == 0)
					CONSOLE_LOG(LogTypes::Error, "FILE SYSTEM: Could not close file '%s'. ERROR: %s", fileName.data(), PHYSFS_getLastError());
			}
		}
		else
			CONSOLE_LOG(LogTypes::Error, "FILE SYSTEM: Could not open file '%s' to read. ERROR: %s", fileName.data(), PHYSFS_getLastError());
	}
	else
		CONSOLE_LOG(LogTypes::Error, "FILE SYSTEM: Could not load file '%s' to read because it doesn't exist", fileName.data());

	return objCount;
}

std::string ModuleFileSystem::getAppPath()
{
	std::string baseDir = PHYSFS_getBaseDir();

	PHYSFS_unmount(".");

	AddPath((char*)baseDir.data(), "");

	if (Exists("physfs.dll"))
	{
		PHYSFS_unmount(baseDir.data());
		PHYSFS_mount(".", "", 0);
		return PHYSFS_getBaseDir();
	}

	else
	{
		PHYSFS_unmount(baseDir.data());

		for (int i = 0; i < 2; ++i)
		{
			baseDir = baseDir.substr(0, baseDir.find_last_of("\\"));
		}

		baseDir += "\\Game\\";

		AddPath((char*)baseDir.data(), "");

		std::string moretemp = baseDir + "physfs.dll";

		if (Exists("physfs.dll"))
		{
			PHYSFS_unmount(baseDir.data());
			PHYSFS_mount(".", "", 0);
			return baseDir;
		}

		PHYSFS_unmount(baseDir.data());
	}

	PHYSFS_unmount(baseDir.data());

	PHYSFS_mount(".", "", 0);

	return "";
}

void ModuleFileSystem::UpdateAssetsDir()
{
	Directory newAssetsDir = RecursiveGetFilesFromDir("Assets");

	if (newAssetsDir != rootDir)
	{
		SendEvents(newAssetsDir);
		rootDir = newAssetsDir;
	}
}

bool ModuleFileSystem::MoveFileInto(const std::string& file, const std::string& newLocation)
{
	char* buffer;
	int size = Load(file, &buffer);
	if (size <= 0)
	{
		CONSOLE_LOG(LogTypes::Error, "Couldn't move the file");
		return false;
	}

	if (Save(newLocation, buffer, size) <= 0)
	{
		CONSOLE_LOG(LogTypes::Error, "Couldn't move the file");
		delete[] buffer;
		return false;
	}

	if (!deleteFile(file))
	{
		//CONSOLE_LOG(LogTypes::Error, "Couldn't delete the file from origin");
		delete[] buffer;
		return false;
	}

	CONSOLE_LOG(LogTypes::Normal, "File %s moved succesfully to %s.", file.data(), newLocation.data());

	delete[] buffer;
	return true;
}

bool ModuleFileSystem::CopyDirectoryAndContentsInto(const std::string& origin, const std::string& destination, bool keepRoot)
{
	Directory originDir = RecursiveGetFilesFromDir((char*)origin.data());

	for (int i = 0; i < originDir.files.size(); ++i)
	{
		std::string file = originDir.files[i].name;

		char* buffer;
		int size;

		size = Load(origin + "/" + file, &buffer);
		if (size <= 0)
			return false;

		std::string destinationWithRoot = destination + "/" + originDir.name + "/" + file;

		std::string destinationWithoutRoot = destinationWithRoot.at(0) == '/' ? destinationWithRoot.substr(1) : destinationWithRoot;
		destinationWithoutRoot = destinationWithoutRoot.substr(destinationWithoutRoot.find_first_of("/") + 1);

		std::string realDestination = keepRoot ? destinationWithRoot : destinationWithoutRoot;

		if (Save(realDestination, buffer, size) <= 0)
		{
			delete[] buffer;
			return false;
		}

		delete[] buffer;
	}

	for (int i = 0; i < originDir.directories.size(); ++i)
	{
		bool success = CopyDirectoryAndContentsInto(originDir.directories[i].fullPath, destination + "/" + originDir.name, keepRoot);
		if (!success)
			return false;
	}

	return true;
}

Directory ModuleFileSystem::RecursiveGetFilesFromDir(char* dir) const
{
	Directory ret;
	ret.fullPath = dir;
	std::string dirstr(dir);
	std::string name;
	int pos = dirstr.find_last_of("/");
	if (pos != std::string::npos)
	{
		name = dirstr.substr(pos + 1, std::string::npos);
	}
	else
	{
		name = dirstr;
	}

	ret.name = name;

	char** files = PHYSFS_enumerateFiles(dir);
	for (int i = 0; files[i] != nullptr; ++i)
	{
		std::string fulldir(dir + std::string("/") + std::string(files[i]));

		//First, check if PHYSFS can recognize this file as a real file or a directory. If a UNKNOWN_TYPE is thrown, then search the extension and use it to detect. This may fail sometimes.
		PHYSFS_Stat stats;
		PHYSFS_stat(fulldir.data(), &stats);

		if (stats.filetype == PHYSFS_FileType::PHYSFS_FILETYPE_OTHER)
		{
			//Here use the extension as recognition-method.
			CONSOLE_LOG(LogTypes::Warning, "Physfs could not recognize if \"%s\" is a file or a directory. Using the extension as recognition-method. Weird behaviors may happen.", files[i]);

			std::string file(files[i]);
			if (file.find(".") == std::string::npos) //It's a directory, have not extension
			{
				stats.filetype = PHYSFS_FileType::PHYSFS_FILETYPE_DIRECTORY;
				CONSOLE_LOG(LogTypes::Warning, "File \"%s\" was recognized as a directory", files[i]);
			}
			else
			{
				stats.filetype = PHYSFS_FileType::PHYSFS_FILETYPE_REGULAR;
				CONSOLE_LOG(LogTypes::Warning, "File \"%s\" was recognized as a regular file", files[i]);
			}
		}

		if (stats.filetype == PHYSFS_FileType::PHYSFS_FILETYPE_DIRECTORY)
		{
			Directory child = RecursiveGetFilesFromDir((char*)fulldir.data());
			child.fullPath = fulldir;
			ret.directories.push_back(child);
		}
		else if (stats.filetype == PHYSFS_FileType::PHYSFS_FILETYPE_REGULAR)
		{
			std::string extension;
			GetExtension(files[i], extension);

			if (extension == ".meta")
				continue;

			File file;
			file.lastModTime = stats.modtime; //Save the last modification time in order to know when a file has changed
			file.name = files[i];
			ret.files.push_back(file);
		}

	}
	PHYSFS_freeList(files);

	return ret;
}

bool ModuleFileSystem::deleteFile(const std::string& filePath) const
{
	return PHYSFS_delete(filePath.c_str()) != 0;
}

bool ModuleFileSystem::deleteFiles(const std::string& root, const std::string& extension, bool deleteDir, bool build) const
{
	bool ret = true;

	Directory directory = RecursiveGetFilesFromDir((char*)root.c_str());

	for (int i = 0; i < directory.files.size(); ++i)
	{
		std::string fileExt; GetExtension(directory.files[i].name.data(), fileExt);
		if (!build)
			if (fileExt == ".dll")
				continue;

		if (extension.empty() || fileExt == extension)
		{
			ret = PHYSFS_delete(std::string(directory.fullPath + "/" + directory.files[i].name).c_str()) != 0;

			if (build)
				if(Exists(directory.fullPath + "/" + directory.files[i].name + ".meta"))
					ret = PHYSFS_delete(std::string(directory.fullPath + "/" + directory.files[i].name + ".meta").c_str()) != 0;
		}

		if (ret == false)
			return false;
	}

	for (int i = 0; i < directory.directories.size(); ++i)
	{
		ret = deleteFiles(directory.directories[i].fullPath, extension, deleteDir, build);

		if (ret == false)
			return false;
	}

	if (ret && deleteDir)
		PHYSFS_delete(directory.fullPath.data());
	else
		return false;

	return ret;
}

void ModuleFileSystem::SendEvents(const Directory& newAssetsDir)
{
	//To get the moved elements, initialize here 2 vectors with both fullpaths. Below here when a file is detected as added or deleted,
	//delete their reference in both fullpaths. At the end of the added and deleted fors, compare both fullpaths.
	//The different ones have to have been moved.

	//Two arrays with a complete path for each file contained in the directory
	std::vector<std::string> newFullPaths;
	std::vector<std::string> oldFullPaths;
	newAssetsDir.getFullPaths(newFullPaths);
	rootDir.getFullPaths(oldFullPaths);

	//Get the file names, without the path, for each file contained in the directory
	std::vector<File> newFiles;
	std::vector<File> oldFiles;
	rootDir.getFiles(oldFiles);
	newAssetsDir.getFiles(newFiles);

	//Check for deleted files
	for (int i = 0; i < oldFiles.size(); ++i)
	{
		bool exists = false;
		bool modified = false;
		int64_t lastModTime;
		for (int j = 0; j < newFiles.size(); ++j)
		{
			if (oldFiles[i] == newFiles[j])
				exists = true;
			else
				if (oldFiles[i].name == newFiles[j].name) //Exists, but the file has been modified
				{
					exists = true;
					modified = true;
					lastModTime = newFiles[j].lastModTime;
				}
		}
		if (!exists)
		{
			//A file has been deleted, get the full path and send the event.
			//Delete this file from the oldFullPaths vector.

			std::string file = oldFiles[i].name;
			std::string fullPathFile;
			for (int j = 0; j < oldFullPaths.size(); ++j)
			{
				if (oldFullPaths[j].find(file) != std::string::npos)
				{
					//We founded the path
					fullPathFile = oldFullPaths[j];
					oldFullPaths.erase(oldFullPaths.begin() + j);
					break;
				}
			}
			System_Event event;
			event.fileEvent.type = System_Event_Type::FileRemoved;
			strcpy(event.fileEvent.file, fullPathFile.c_str());
			App->PushSystemEvent(event);
		}
		if (modified)
		{
			//Send the event
			std::string file = oldFiles[i].name;
			std::string fullPathFile;
			for (int j = 0; j < oldFullPaths.size(); ++j)
			{
				if (oldFullPaths[j].find(file) != std::string::npos)
				{
					//We founded the path. Do not erase from the vector
					fullPathFile = oldFullPaths[j];
					break;
				}
			}

			System_Event event;
			event.fileEvent.type = System_Event_Type::FileOverwritten;
			strcpy(event.fileEvent.file, fullPathFile.c_str());
			App->PushSystemEvent(event);

			Resource::SetLastModTimeToMeta((fullPathFile + ".meta").data(), lastModTime);
		}
	}

	//Check for added files
	for (int i = 0; i < newFiles.size(); ++i)
	{
		bool existed = false;
		bool modified = false;
		for (int j = 0; j < oldFiles.size(); ++j)
		{
			if (newFiles[i].name == oldFiles[j].name) //We already checked and sent events for file modifications above.
				existed = true;
		}
		if (!existed)
		{
			//A file has been added, get the full path and send the event
			//Delete this file from the newFullPaths vector.

			std::string file = newFiles[i].name;
			std::string fullPathFile;
			for (int j = 0; j < newFullPaths.size(); ++j)
			{
				if (newFullPaths[j].find(file) != std::string::npos)
				{
					//We founded the path
					fullPathFile = newFullPaths[j];
					newFullPaths.erase(newFullPaths.begin() + j);
					break;
				}
			}

			if (!Exists(fullPathFile + ".meta"))
			{
				System_Event event;
				event.fileEvent.type = System_Event_Type::NewFile;
				strcpy(event.fileEvent.file, fullPathFile.c_str());
				App->PushSystemEvent(event);
			}		
		}
	}

	//Now we only have complete paths referencing the same files in the FullPath vectors.
	//Compare same-named files' paths and send events if they are different.
	for (int i = 0; i < oldFullPaths.size(); ++i)
	{
		int oldPos = oldFullPaths[i].find_last_of("/") + 1;
		std::string oldfile = oldFullPaths[i].substr(oldPos);

		for (int j = 0; j < newFullPaths.size(); ++j)
		{
			int newPos = newFullPaths[j].find_last_of("/") + 1;
			std::string newfile = newFullPaths[j].substr(newPos);

			if (oldfile != newfile) //Different files, don't care their path, continue searching
				continue;

			std::string oldPath = oldFullPaths[i].substr(0, oldPos);
			std::string newPath = newFullPaths[j].substr(0, newPos);

			if (oldPath != newPath) //Same file, different paths
			{
				System_Event event;
				event.fileEvent.type = System_Event_Type::FileMoved;
				strcpy(event.fileEvent.newFileLocation, newFullPaths[j].c_str());

				strcpy(event.fileEvent.file, oldFullPaths[i].c_str());
				App->PushSystemEvent(event);
				break;
			}
		}
	}
}

void ModuleFileSystem::ImportFilesEvents(const Directory& directory, std::vector<std::string>& lateEvents, std::vector<std::string>& lateLateEvents, bool reimport)
{
	for (int i = 0; i < directory.files.size(); ++i)
	{
		//char metaFile[DEFAULT_BUF_SIZE];
		char filePath[DEFAULT_BUF_SIZE];

		strcpy(filePath, directory.fullPath.data());
		strcat(filePath, "/");
		strcat(filePath, directory.files[i].name.data());

		//Send the ResourceMaterials at the end
		std::string extension;
		GetExtension(filePath, extension);

		ResourceTypes resourceType = ResourceTypes::NoResourceType;
#ifndef GAMEMODE
		resourceType = App->res->GetResourceTypeByExtension(extension.data());
#else
		resourceType = App->res->GetLibraryResourceTypeByExtension(extension.data());
#endif
		switch (resourceType)
		{
			case ResourceTypes::ShaderProgramResource:
			{
				lateEvents.push_back(filePath);
				break;
			}
			case ResourceTypes::MaterialResource:
			{
				lateLateEvents.push_back(filePath);
				break;
			}
			default:
			{
				//The ResourceManager already manages the .meta, already imported files etc. on his own.
				System_Event event;
#ifndef GAMEMODE
				event.fileEvent.type = reimport ? System_Event_Type::ReImportFile : System_Event_Type::ImportFile;
#else
				event.fileEvent.type = System_Event_Type::ImportLibraryFile;
#endif
				event.fileEvent.build = false;
				strcpy(event.fileEvent.file, filePath);
				App->PushSystemEvent(event);

				break;
			}			
		}

		/*
		strcpy(metaFile, filePath);
		strcat(metaFile, ".meta");

		if (!Exists(metaFile))
		{
			//A new file has been added

			System_Event event;
			event.fileEvent.type = System_Event_Type::NewFile;
			strcpy(event.fileEvent.file, filePath);
			App->PushSystemEvent(event);

			continue;
		}

		//Check if the last mod time is different that the last exported time
		PHYSFS_Stat fileStats;
		PHYSFS_sint64 lastExportedTime;

		if (PHYSFS_stat(filePath, &fileStats) == 0)
		{
			CONSOLE_LOG(LogTypes::Error, "FILESYSTEM: Error reading stats from a file. Error: %s", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
			continue;
		}

		//Get the last exported time from the .meta
		char* metaBuffer;
		uint metaSize = Load(metaFile, &metaBuffer);
		if (metaSize <= 0)
			continue;

		memcpy(&lastExportedTime, metaBuffer, sizeof(PHYSFS_sint64));

		delete[] metaBuffer;

		if (fileStats.modtime != lastExportedTime)
		{
			//The file has been modified

			System_Event event;
			event.fileEvent.type = System_Event_Type::FileOverwritten;
			strcpy(event.fileEvent.file, filePath);
			App->PushSystemEvent(event);
		}

		else
		{
			//Just load the resources

			System_Event event;
			event.fileEvent.type = System_Event_Type::ImportFile;
			strcpy(event.fileEvent.file, filePath);
			App->PushSystemEvent(event);
		}
		*/

	}

	for (int i = 0; i < directory.directories.size(); ++i)
	{
		ImportFilesEvents(directory.directories[i], lateEvents, lateLateEvents, reimport);
	}
}

void ModuleFileSystem::ForceReImport(const Directory& assetsDir)
{
	for (int i = 0; i < assetsDir.files.size(); ++i)
	{
		char filePath[DEFAULT_BUF_SIZE];

		strcpy(filePath, assetsDir.fullPath.data());
		strcat(filePath, "/");
		strcat(filePath, assetsDir.files[i].name.data());

		//The ResourceManager already manages the .meta, already imported files etc. on his own.
		System_Event event;
		event.fileEvent.type = System_Event_Type::ForceReImport;
		strcpy(event.fileEvent.file, filePath);
		App->PushSystemEvent(event);
	}

	for (int i = 0; i < assetsDir.directories.size(); ++i)
	{
		ForceReImport(assetsDir.directories[i]);
	}
}

void ModuleFileSystem::BeginTempException(std::string directory)
{
	if (tempException.empty())
	{
		tempException = directory;

		//Add to the search path
		if (PHYSFS_mount(directory.data(), "Exception", 1) == 0)
		{
			char* error = (char*)PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode());
		}
	}
	else
	{
		CONSOLE_LOG(LogTypes::Error, "FILESYSTEM: YOU CAN ONLY HAVE 1 EXCEPTION LOADED AT THE SAME TIME");
	}
}

void ModuleFileSystem::EndTempException()
{
	if (tempException.empty())
		return;

	PHYSFS_unmount(tempException.data());
	tempException.clear();
}

bool ModuleFileSystem::SetWriteDir(std::string writeDir) const
{
	return PHYSFS_setWriteDir(writeDir.data()) != 0;
}

void ModuleFileSystem::RenameDirectory(const char* dir, const char* newName)
{
	rename(dir, newName);
}

void ModuleFileSystem::RecursiveBuild(const Directory& dir, char * toPath, bool meta, bool inZIP)
{
	for (File file : dir.files)
	{
		if (file.name == "JellyBitEngine.exe")
			continue;

		std::string extension;
		GetExtension(file.name.data(), extension);
		if (extension == ".txt" || extension == ".log" || 
			extension == ".sln" || extension == ".csproj" ||
			extension == ".bnk")
			continue;
		else
		{
			std::string filePath = dir.fullPath + "/" + file.name;

			char* buffer = nullptr;
			uint size = Load(filePath.data(), &buffer);
			if (size > 0)
			{
				char temp[DEFAULT_BUF_SIZE];
				strcpy(temp, toPath);
				strcat(temp, "/");
				strcat(temp, file.name.data());

				//if(inZIP)
					//WriteFile(zip_path.data(), *file, buffer, size);

				Save(temp, buffer, size);
				if (meta && Exists(filePath + ".meta"))
				{
					RELEASE_ARRAY(buffer);

					size = Load(filePath + ".meta", &buffer);
					if (size > 0)
					{
						strcat(temp, ".meta");
						Save(temp, buffer, size);
					}
				}
				RELEASE_ARRAY(buffer);
			}
		}
	}

	for (Directory toDir : dir.directories)
	{
		if (toDir.name == ".vs" || toDir.name == "Assets" || toDir.name == "obj" || toDir.name == "Internal" || toDir.name == "PrefDir")
			continue;
		else
		{
			char temp[DEFAULT_BUF_SIZE];
			strcpy(temp, toPath);
			strcat(temp, "/");
			strcat(temp, toDir.name.data());
			if (!Exists(temp))
				CreateDir(temp);
			
			if (strstr(toDir.fullPath.data(),"Library") != nullptr)
				meta = true;
			RecursiveBuild(toDir, temp, meta, inZIP);
			meta = false;
		}
	}
}

std::string ModuleFileSystem::PathToWindowsNotation(std::string path)
{
	std::string ret = path;

	uint pos = ret.find("/");
	while (pos != std::string::npos)
	{
		ret.replace(pos, 1, "\\");

		pos = ret.find("/");
	}

	return ret;
}
