#include "ScriptingModule.h"
#include "ComponentScript.h"
#include "ResourceScript.h"
#include "ComponentTransform.h"
#include "ComponentNavAgent.h"
#include "ComponentAnimation.h"
#include "ComponentEmitter.h"
#include "ComponentRectTransform.h"
#include "ComponentButton.h"
#include "ComponentAudioSource.h"
#include "ComponentAudioListener.h"
#include "ComponentRigidDynamic.h"

#include "GameObject.h"

#include <mono/metadata/assembly.h>
#include <mono/jit/jit.h>
#include <mono/metadata/mono-config.h>

#include <array>
#include <iostream>

#include "Application.h"
#include "ModuleFileSystem.h"
#include "ModuleInput.h"
#include "ModuleTimeManager.h"
#include "ModuleResourceManager.h"
#include "ModuleRenderer3D.h"
#include "ModuleGOs.h"
#include "ModuleGui.h"
#include "ModuleLayers.h"
#include "ModulePhysics.h"
#include "SceneQueries.h"
#include "ModuleInput.h"
#include "ModuleScene.h"
#include "ModuleUI.h"

#include "MathGeoLib/include/MathGeoLib.h"
#include "Brofiler/Brofiler.h"

#include "parson/parson.h"

bool exec(const char* cmd, std::string& error)
{
	std::array<char, 128> buffer;
	bool result;
	auto pipe = _popen(cmd, "r");

	if (!pipe) throw std::runtime_error("popen() failed!");

	while (!feof(pipe))
	{
		if (fgets(buffer.data(), 128, pipe) != nullptr)
			error += buffer.data();
	}

	auto rc = _pclose(pipe);

	if (rc == EXIT_SUCCESS)
	{
		std::cout << "SUCCESS\n";
		result = true;
	}
	else
	{
		std::cout << "FAILED\n";
		result = false;
	}

	return result;
}

bool ScriptingModule::Init(JSON_Object* data)
{
	//Locate the lib and etc folders in the mono installation
	std::string MonoLib, MonoEtc, gamePath = App->fs->getAppPath();
	MonoLib = gamePath + "Mono\\lib";
	MonoEtc = gamePath + "Mono\\etc";

	mono_set_dirs(MonoLib.data(), MonoEtc.data());
	mono_config_parse(NULL);

	//Initialize the mono domain
	domain = mono_jit_init("Scripting");
	if (!domain)
		return false;

	CreateDomain();

	if (!internalAssembly)
		return true;

	char* args[1];
	args[0] == "InternalAssembly";
	mono_jit_exec(domain, internalAssembly, 1, args);

	return true;
}

bool ScriptingModule::Start()
{
	InitPlayerPrefs();

#ifndef GAMEMODE
	CreateScriptingProject();
	IncludeCSFiles();
#endif

	return true;
}

update_status ScriptingModule::PreUpdate()
{
	return UPDATE_CONTINUE;
}

update_status ScriptingModule::Update()
{
#ifndef GAMEMODE
	BROFILER_CATEGORY(__FUNCTION__, Profiler::Color::PapayaWhip);
#endif
	if (App->GetEngineState() == engine_states::ENGINE_PLAY)
	{
		UpdateMethods();
	}

	return UPDATE_CONTINUE;
}

update_status ScriptingModule::PostUpdate()
{
#ifndef GAMEMODE
	BROFILER_CATEGORY(__FUNCTION__, Profiler::Color::PapayaWhip);
#endif // !GAMEMODE
	if (someScriptModified || engineOpened)
	{
#ifndef GAMEMODE
		RecompileScripts();
		someScriptModified = false;
		engineOpened = false;
#else
		//Engine opened recently, import the .dll if found

		/*System_Event event;
		event.type = System_Event_Type::ScriptingDomainReloaded;
		App->PushSystemEvent(event);

		App->scripting->CreateDomain();
		App->scripting->UpdateScriptingReferences();

		std::vector<Resource*> scriptResources = App->res->GetResourcesByType(ResourceTypes::ScriptResource);
		for (int i = 0; i < scriptResources.size(); ++i)
		{
			ResourceScript* scriptRes = (ResourceScript*)scriptResources[i];
			scriptRes->referenceMethods();
		}

		App->scripting->ReInstance();

		engineOpened = false;*/

#endif
	}

	return UPDATE_CONTINUE;
}

bool ScriptingModule::CleanUp()
{
	for (int i = 0; i < scripts.size(); ++i)
	{
		scripts[i]->GetParent()->EraseComponent(scripts[i]);
		delete scripts[i];
	}

	scripts.clear();

	if(domain)
		mono_jit_cleanup(domain);

	domain = nullptr;

	return true;
}

void ScriptingModule::OnSystemEvent(System_Event event)
{
	switch (event.type)
	{
		case System_Event_Type::LoadFinished:
		{
			if (App->GetEngineState() == engine_states::ENGINE_PLAY)
			{
				for (int i = 0; i < scripts.size(); ++i)
				{
					scripts[i]->OnEnableMethod();
				}

				for (int i = 0; i < scripts.size(); ++i)
				{
					scripts[i]->Awake();
				}

				for (int i = 0; i < scripts.size(); ++i)
				{
					scripts[i]->Start();
				}
			}
			break;
		}

		case System_Event_Type::Play:
		{
			//TODO: Check if some files have compile errors and don't let the user hit the play.

			for (int i = 0; i < scripts.size(); ++i)
			{
				scripts[i]->OnEnableMethod();
			}

			for (int i = 0; i < scripts.size(); ++i)
			{
				scripts[i]->Awake();
			}

			for (int i = 0; i < scripts.size(); ++i)
			{
				scripts[i]->Start();
			}
		
			//Call the Awake and Start for all the Enabled script in the Play instant.
			break;
		}
	
		case System_Event_Type::Stop:
		{
			for (int i = 0; i < scripts.size(); ++i)
			{
				if (scripts[i]->IsTreeActive())
				{
					scripts[i]->OnStop();
				}
			}

			ClearMap();

			break;
		}

		case System_Event_Type::LoadScene:
		{
			ClearMap();
			break;
		}

		case System_Event_Type::ResourceDestroyed:
		{		
			for (int i = 0; i < scripts.size(); ++i)
			{
				bool somethingDestroyed = false;

				if (scripts[i]->scriptResUUID == event.resEvent.resource->GetUuid())
				{
					somethingDestroyed = true;
					scripts[i]->GetParent()->EraseComponent(scripts[i]);
					delete scripts[i];
					scripts.erase(scripts.begin() + i);

					i--;
				}
				if (somethingDestroyed)
				{
					IncludeCSFiles();
				}
			}			
		}

		case System_Event_Type::GameObjectDestroyed:
		{
			for (int i = 0; i < monoObjectHandles.size(); ++i)
			{
				MonoObject* monoObject = mono_gchandle_get_target(monoObjectHandles[i]);

				int address;
				mono_field_get_value(monoObject, mono_class_get_field_from_name(mono_object_get_class(monoObject), "cppAddress"), &address);

				GameObject* gameObject = (GameObject*)address;
					
				if (gameObject == event.goEvent.gameObject)
				{
					MonoClass* monoObjectClass = mono_object_get_class(monoObject);

					MonoClassField* deletedField = mono_class_get_field_from_name(monoObjectClass, "destroyed");

					bool temp = true;
					mono_field_set_value(monoObject, deletedField, &temp);

					mono_gchandle_free(monoObjectHandles[i]);

					monoObjectHandles.erase(monoObjectHandles.begin() + i);
					i--;

					//Erase this gameObject from all the public variables in scripts
					for (int i = 0; i < scripts.size(); ++i)
					{
						scripts[i]->OnSystemEvent(event);
					}
				}
			}		
			break;
		}

		case System_Event_Type::ComponentDestroyed:
		{
			Component* toDelete = event.compEvent.component;
			MonoObject* monoComponent = toDelete->GetMonoComponent();
			if (monoComponent)
			{
				bool destroyed = true;
				mono_field_set_value(monoComponent, mono_class_get_field_from_name(mono_object_get_class(monoComponent), "destroyed"), &destroyed);

				for (int i = 0; i < monoComponentHandles.size(); ++i)
				{
					if (monoComponent == mono_gchandle_get_target(monoComponentHandles[i]))
					{
						mono_gchandle_free(monoComponentHandles[i]);
						monoComponentHandles.erase(monoComponentHandles.begin() + i);
						i--;
					}						
				}
			}
			break;
		}

		case System_Event_Type::LoadGMScene:
		{
			//Engine opened recently, import the .dll if found
			if (engineOpened)
			{
				App->scripting->CreateDomain();
				App->scripting->UpdateScriptingReferences();

				std::vector<Resource*> scriptResources = App->res->GetResourcesByType(ResourceTypes::ScriptResource);
				for (int i = 0; i < scriptResources.size(); ++i)
				{
					ResourceScript* scriptRes = (ResourceScript*)scriptResources[i];
					scriptRes->referenceMethods();
				}

				App->scripting->ReInstance();

				engineOpened = false;
			}		
			break;
		}
	}
}

ComponentScript* ScriptingModule::CreateScriptComponent(std::string scriptName, bool createCS)
{
	while (scriptName.find(" ") != std::string::npos)
	{
		scriptName = scriptName.replace(scriptName.find(" "), 1, "");
	}

	ComponentScript* script = new ComponentScript(scriptName);
	char* buffer;
	int size;

	if (createCS)
	{
		size = App->fs->Load("Internal/SampleScript/SampleScript.cs", &buffer);

		std::string scriptStream = buffer;
		scriptStream.resize(size);

		while (scriptStream.find("SampleScript") != std::string::npos)
		{
			scriptStream = scriptStream.replace(scriptStream.find("SampleScript"), 12, scriptName);
		}

		App->fs->Save("Assets/Scripts/" + scriptName + ".cs", (char*)scriptStream.c_str(), scriptStream.size());

		IncludeCSFiles();

		delete[] buffer;
	}

	ResourceScript* scriptRes = nullptr;

	if (App->fs->Exists("Assets/Scripts/" + scriptName + ".cs.meta"))
	{
		char* metaBuffer;
		uint metaSize;

		metaSize = App->fs->Load("Assets/Scripts/" + scriptName + ".cs.meta", &metaBuffer);

		char* cursor = metaBuffer;
		cursor += sizeof(int64_t) + sizeof(uint);

		uint32_t UID;
		memcpy(&UID, cursor, sizeof(uint32_t));

		scriptRes = (ResourceScript*)App->res->GetResource(UID);

		delete[] metaBuffer;
	}
	
	if (!scriptRes)
	{
		//Here we have to reference a new ResourceScript with the .cs we have created, but the ResourceManager will still be sending file created events, and we would have data duplication.
		//We disable this behavior and control the script creation only with this method, so we do not care for external files out-of-engine created.
		
		ResourceData data;
		data.name = scriptName;
		data.file = "Assets/Scripts/" + scriptName + ".cs";
		data.exportedFile = "";
	
		scriptRes = (ResourceScript*)App->res->CreateResource(ResourceTypes::ScriptResource, data, &ResourceScriptData());

		//Create the .meta, to make faster the search in the map storing the uid.
		uint bytes = scriptRes->bytesToSerializeMeta();
		char* buffer = new char[bytes];
		char* cursor = buffer;
		scriptRes->SerializeToMeta(cursor);

		App->fs->Save("Assets/Scripts/" + scriptName + ".cs.meta", buffer, bytes);

		someScriptModified = true;

		delete[] buffer;
	}

	App->res->SetAsUsed(scriptRes->GetUuid());
	script->scriptResUUID = scriptRes->GetUuid();

	scripts.push_back(script);

	return script;
}

bool ScriptingModule::DestroyScript(ComponentScript* script)
{
	for (int i = 0; i < scripts.size(); ++i)
	{
		if (scripts[i] == script)
		{
			delete script;
			scripts.erase(scripts.begin() + i);
			return true;
		}
	}

	return false;
}

void ScriptingModule::ClearScriptComponent(ComponentScript* script)
{
	for (int i = 0; i < scripts.size(); ++i)
	{
		if (scripts[i] == script)
		{
			scripts.erase(scripts.begin() + i);
			break;
		}
	}
}

MonoObject* ScriptingModule::MonoObjectFrom(GameObject* gameObject)
{
	MonoObject* monoObject = gameObject->GetMonoObject();

	if (monoObject)
	{
		GameObject* storedGO = GameObjectFrom(monoObject);
		if(storedGO == gameObject)
			return monoObject;
	}

	MonoClass* gameObjectClass = mono_class_from_name(internalImage, "JellyBitEngine", "GameObject");
	monoObject = mono_object_new(domain, gameObjectClass);
	mono_runtime_object_init(monoObject);

	int address = (int)gameObject;
	mono_field_set_value(monoObject, mono_class_get_field_from_name(gameObjectClass, "cppAddress"), &address);

	uint32_t handleID = mono_gchandle_new(monoObject, true);

	gameObject->SetMonoObject(handleID);

	monoObjectHandles.push_back(handleID);

	return monoObject;
}

GameObject* ScriptingModule::GameObjectFrom(MonoObject* monoObject)
{
	int address;
	mono_field_get_value(monoObject, mono_class_get_field_from_name(mono_object_get_class(monoObject), "cppAddress"), &address);

	GameObject* gameObject = (GameObject*)address;

	MonoObject* storedMonoObj = gameObject->GetMonoObject();
	if (storedMonoObj == monoObject)
		return gameObject;
	else
		return nullptr;
	
	//We only can create MonoObjects though a GameObject*, not viceversa.
}

MonoObject* ScriptingModule::MonoComponentFrom(Component* component)
{
	MonoObject* monoComponent = nullptr;
	monoComponent = component->GetMonoComponent();
	if (monoComponent)
		return monoComponent;

	switch (component->GetType())
	{
		case ComponentTypes::CameraComponent:
		{
			monoComponent = mono_object_new(App->scripting->domain, mono_class_from_name(App->scripting->internalImage, "JellyBitEngine", "Camera"));			
			break;
		}
		case ComponentTypes::BoxColliderComponent:
		case ComponentTypes::CapsuleColliderComponent:
		case ComponentTypes::PlaneColliderComponent:
		case ComponentTypes::SphereColliderComponent:
		{
			monoComponent = mono_object_new(App->scripting->domain, mono_class_from_name(App->scripting->internalImage, "JellyBitEngine", "Collider"));
			break;
		}

		case ComponentTypes::NavAgentComponent:
		{
			monoComponent = mono_object_new(App->scripting->domain, mono_class_from_name(App->scripting->internalImage, "JellyBitEngine", "NavMeshAgent"));
			break;
		}

		case ComponentTypes::AnimationComponent:
		{
			monoComponent = mono_object_new(App->scripting->domain, mono_class_from_name(App->scripting->internalImage, "JellyBitEngine", "Animator"));
			break;
		}
		case ComponentTypes::EmitterComponent:
		{
			monoComponent = mono_object_new(App->scripting->domain, mono_class_from_name(App->scripting->internalImage, "JellyBitEngine", "ParticleEmitter"));
			break;
		}
		case ComponentTypes::RectTransformComponent:
		{
			monoComponent = mono_object_new(App->scripting->domain, mono_class_from_name(App->scripting->internalImage, "JellyBitEngine.UI", "RectTransform"));
			break;
		}
		case ComponentTypes::ButtonComponent:
		{
			monoComponent = mono_object_new(App->scripting->domain, mono_class_from_name(App->scripting->internalImage, "JellyBitEngine.UI", "Button"));
			break;
		}
		case ComponentTypes::ImageComponent:
		{
			monoComponent = mono_object_new(App->scripting->domain, mono_class_from_name(App->scripting->internalImage, "JellyBitEngine.UI", "Image"));
			break;
		}
		case ComponentTypes::RigidDynamicComponent:
		{
			monoComponent = mono_object_new(App->scripting->domain, mono_class_from_name(App->scripting->internalImage, "JellyBitEngine", "Rigidbody"));
			break;
		}
		case ComponentTypes::ProjectorComponent:
		{
			monoComponent = mono_object_new(App->scripting->domain, mono_class_from_name(App->scripting->internalImage, "JellyBitEngine", "Projector"));
			break;
		}
		case ComponentTypes::AudioSourceComponent:
		{
			monoComponent = mono_object_new(App->scripting->domain, mono_class_from_name(App->scripting->internalImage, "JellyBitEngine", "AudioSource"));
			break;
		}
		case ComponentTypes::AudioListenerComponent:
		{
			monoComponent = mono_object_new(App->scripting->domain, mono_class_from_name(App->scripting->internalImage, "JellyBitEngine", "AudioListener"));
			break;
		}
	}

	if (!monoComponent)
		return nullptr;

	mono_runtime_object_init(monoComponent);

	int gameObjectAddress = (int)component->GetParent();
	int componentAddress = (int)component;

	mono_field_set_value(monoComponent, mono_class_get_field_from_name(mono_object_get_class(monoComponent), "gameObjectAddress"), &gameObjectAddress);
	mono_field_set_value(monoComponent, mono_class_get_field_from_name(mono_object_get_class(monoComponent), "componentAddress"), &componentAddress);
	mono_field_set_value(monoComponent, mono_class_get_field_from_name(mono_object_get_class(monoComponent), "gameObject"), MonoObjectFrom(component->GetParent()));

	uint32_t handleID = mono_gchandle_new(monoComponent, true);
	component->SetMonoComponent(handleID);

	monoComponentHandles.push_back(handleID);

	return monoComponent;
}

Component* ScriptingModule::ComponentFrom(MonoObject* monoComponent)
{
	int componentAddress;
	mono_field_get_value(monoComponent, mono_class_get_field_from_name(mono_object_get_class(monoComponent), "componentAddress"), &componentAddress);	

	return (Component*)componentAddress;
}

bool ScriptingModule::alreadyCreated(std::string scriptName)
{
	clearSpaces(scriptName);

	for (int i = 0; i < scripts.size(); ++i)
	{
		if (scriptName == scripts[i]->scriptName)
			return true;
	}

	return false;
}

void ScriptingModule::CreateScriptingProject()
{
	if (App->fs->Exists("ScriptingProject.sln"))
		return;

	App->fs->CopyDirectoryAndContentsInto("Internal/ScriptingProject", "", false);
}

void ScriptingModule::ExecuteScriptingProject()
{
	//We need the path to the Visual Studio .exe and after that ask the program to start opening the right solution. No idea how to do that for now.
#if 0
	CreateScriptingProject();

	IncludecsFiles();

	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	bool ret = CreateProcess("C:\\Users\\Jony635\\Desktop\\Proyectos 3o\\GitHub\\Flan3DEngine\\Flan3DEngine\\Game\\ScriptingProject.sln", "C:\\Users\\Jony635\\Desktop\\Proyectos 3o\\GitHub\\Flan3DEngine\\Flan3DEngine\\Game\\ScriptingProject.sln", 0, 0, false, 0, 0, 0, &si, &pi) != 0;

	if (!ret)
	{
		LPSTR messageBuffer = nullptr;
		size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

		std::string message(messageBuffer, size);

		Debug.LogError("Could not open \"ScriptingProject.sln\". Error: %s", message.data());

		//Free the buffer.
		LocalFree(messageBuffer);
	}

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
#endif
}

void ScriptingModule::IncludeCSFiles()
{
	Directory scripts = App->fs->RecursiveGetFilesFromDir("Assets/Scripts");

	//Modify the project settings file, stored in a xml

	char* buffer;
	int size = App->fs->Load("Assembly-CSharp.csproj", &buffer);
	if (size <= 0)
		return;

	pugi::xml_document configFile;
	configFile.load_buffer(buffer, size);
	pugi::xml_node projectNode = configFile.child("Project");
	pugi::xml_node itemGroup = projectNode.child("ItemGroup").next_sibling();
	
	for (pugi::xml_node compile = itemGroup.child("Compile"); compile != nullptr;)
	{
		pugi::xml_node next = compile.next_sibling();

		//Do not delete the internal files
		if (std::string(compile.attribute("Include").as_string()).find("internal") != std::string::npos)
		{
			compile = next;
			continue;
		}
			
		itemGroup.remove_child(compile);
		compile = next;
	}

	IncludeCSFiles(itemGroup, scripts);

	std::ostringstream stream;
	configFile.save(stream, "\r\n");
	App->fs->Save("Assembly-CSharp.csproj", (char*)stream.str().data(), stream.str().size());	
}

void ScriptingModule::IncludeCSFiles(pugi::xml_node& nodeToAppend, const Directory& dir)
{
	for (int i = 0; i < dir.files.size(); ++i)
	{
		std::string extension;
		App->fs->GetExtension(dir.files[i].name.data(), extension);
		if (extension != ".cs")
			continue;

		nodeToAppend.append_child("Compile").append_attribute("Include").set_value(std::string("Assets\\Scripts\\" + dir.files[i].name).data());
	}

	for (int i = 0; i < dir.directories.size(); ++i)
	{
		IncludeCSFiles(nodeToAppend, dir.directories[i]);
	}
}

void ScriptingModule::CreateInternalCSProject()
{
	if (App->fs->Exists("JellyBitCS"))
		return;

	App->fs->CopyDirectoryAndContentsInto("Internal/JellyBitCS", "", true);
}

std::string ScriptingModule::getReferencePath() const
{
	std::string internalCommand = std::string("-r:") + std::string("\"") + App->fs->getAppPath() + std::string("JellyBitCS.dll\" " + std::string("-lib:\"") + App->fs->getAppPath() + "\\Library\\Scripts\" ");
	std::string referencesCommand;

	/*std::vector<Resource*> scripts = App->res->GetResourcesByType(ResourceTypes::ScriptResource);
	for (int i = 0; i < scripts.size(); ++i)
	{
		ResourceScript* script = (ResourceScript*)scripts[i];
		referencesCommand += std::string("-r:") + std::string("\"") + App->fs->getAppPath() + std::string("Library\\Scripts\\") + script->scriptName + ".dll" + std::string("\" ");
	}*/

	return internalCommand + referencesCommand;
}

std::string ScriptingModule::clearSpaces(std::string& scriptName)
{
	while (scriptName.find(" ") != std::string::npos)
	{
		scriptName = scriptName.replace(scriptName.find(" "), 1, "");
	}
	return scriptName;
}

void ScriptingModule::ReInstance()
{
	for (int i = 0; i < scripts.size(); ++i)
	{	
		scripts[i]->InstanceClass();
	}
}

void ScriptingModule::ClearMap()
{
	for (int i = 0; i < monoObjectHandles.size(); ++i)
	{
		mono_gchandle_free(monoObjectHandles[i]);
	}
	monoObjectHandles.clear();
}

Resource* ScriptingModule::ImportScriptResource(const char* file)
{
	//May be new file or generic file event

	std::string fileString = file;
	std::string metaFile = fileString + ".meta";

	std::string scriptName = fileString.substr(fileString.find_last_of("/") + 1);
	scriptName = scriptName.substr(0, scriptName.find_last_of("."));

	ResourceData data;
	data.name = scriptName;
	data.file = file;
	data.exportedFile = "";

	ResourceScript* scriptRes = nullptr;

	bool scriptModified = false;

	if (!App->fs->Exists(data.file + ".meta"))
	{
		scriptRes = (ResourceScript*)App->res->CreateResource(ResourceTypes::ScriptResource, data, &ResourceScriptData());

		//Create the .meta
		uint bytes = scriptRes->bytesToSerializeMeta();
		char* buffer = new char[bytes];
		char* cursor = buffer;
		scriptRes->SerializeToMeta(cursor);

		App->fs->Save(fileString + ".meta", buffer, bytes);

		delete[] buffer;
	}
	else
	{
		char* metaBuffer;
		uint size = App->fs->Load(metaFile, &metaBuffer);
		if (size > 0)
		{
			char* cursor = metaBuffer;
			int64_t lastSavedModTime;
			memcpy(&lastSavedModTime, cursor, sizeof(int64_t));
			cursor += sizeof(int64_t);
			cursor += sizeof(uint);
			uint uid;
			memcpy(&uid, cursor, sizeof(uint));

			int64_t lastModTime = App->fs->GetLastModificationTime(file);
			
			scriptModified = lastSavedModTime != lastModTime;

			if (scriptModified)
			{
				cursor = metaBuffer;
				memcpy(cursor, &lastModTime, sizeof(int64_t));
				App->fs->Save(metaFile, metaBuffer, size);

				someScriptModified = true;
			}

			scriptRes = (ResourceScript*)App->res->CreateResource(ResourceTypes::ScriptResource, data, &ResourceScriptData(), uid);

			cursor = metaBuffer;
			scriptRes->DeSerializeFromMeta(cursor);
			delete[] metaBuffer;
		}
	}
		
	return scriptRes;
}

void ScriptingModule::ScriptModified(const char* scriptPath)
{
	/*char metaFile[DEFAULT_BUF_SIZE];
	strcpy(metaFile, scriptPath);
	strcat(metaFile, ".meta");

	char* metaBuffer;
	uint size = App->fs->Load(metaFile, &metaBuffer);
	if (size < 0)
		return;

	char* cursor = metaBuffer;
	cursor += sizeof(int64_t) + sizeof(uint);
	uint UID;
	memcpy(&UID, cursor, sizeof(uint32_t));

	ResourceScript* scriptModified = (ResourceScript*)App->res->GetResource(UID);
	if (scriptModified->preCompileErrors())
		return;

	System_Event event;
	event.type = System_Event_Type::ScriptingDomainReloaded;
	App->PushSystemEvent(event);*/

	someScriptModified = true;
}

void ScriptingModule::RecompileScripts()
{
	std::string goRoot(R"(cd\ )");
	std::string goMonoBin(" cd \"" + App->fs->getAppPath() + "\\Mono\\bin\"");
	std::string compileCommand(" mcs -target:library ");

	/*std::string fileName = data.file.substr(data.file.find_last_of("/") + 1);
	std::string windowsFormattedPath = pathToWindowsNotation(data.file);*/

	std::string path = std::string("\"" + std::string(App->fs->getAppPath())) + std::string("\\Assets\\Scripts\\*.cs") + "\"";
	std::string redirectOutput(" 1> \"" + /*pathToWindowsNotation*/(App->fs->getAppPath()) + "LogError.txt\"" + std::string(" 2>&1"));
	std::string outputFile(" -out:..\\..\\Library\\Scripts\\Scripting.dll ");

	std::string error;
	if (!exec(std::string(goRoot + "&" + goMonoBin + "&" + compileCommand + path + " " + outputFile + App->scripting->getReferencePath() + redirectOutput).data(), error))
	{
		char* buffer;
		int size = App->fs->Load("LogError.txt", &buffer);
		if (size > 0)
		{
			std::string outPut(buffer);
			outPut.resize(size);

			CONSOLE_LOG(LogTypes::Error, "Error compiling Scripting assembly. Error: %s", outPut.data());

			delete[] buffer;
		}		
	}
	else
	{
		//Send the DomainReloaded event
		System_Event event;
		event.type = System_Event_Type::ScriptingDomainReloaded;
		App->PushSystemEvent(event);
	}
}

void ScriptingModule::GameObjectKilled(GameObject* killed)
{
	MonoObject* monoObject = killed->GetMonoObject();
	if (!monoObject)
		return;

	for (int i = 0; i < monoObjectHandles.size(); ++i)
	{	
		if (mono_gchandle_get_target(monoObjectHandles[i]) == monoObject)
		{
			MonoClass* monoObjectClass = mono_object_get_class(monoObject);

			MonoClassField* deletedField = mono_class_get_field_from_name(monoObjectClass, "destroyed");

			bool temp = true;
			mono_field_set_value(monoObject, deletedField, &temp);

			mono_gchandle_free(monoObjectHandles[i]);

			monoObjectHandles.erase(monoObjectHandles.begin() + i);
			break;
		}
	}
}

void ScriptingModule::FixedUpdate()
{
	for (int i = 0; i < scripts.size(); ++i)
	{
		scripts[i]->FixedUpdate();
	}
}

void ScriptingModule::UpdateMethods()
{
	for (int i = 0; i < scripts.size(); ++i)
	{
		scripts[i]->PreUpdate();
	}
	
	for (int i = 0; i < scripts.size(); ++i)
	{
		scripts[i]->Update();
	}

	for (int i = 0; i < scripts.size(); ++i)
	{
		scripts[i]->PostUpdate();
	}
}

void ScriptingModule::ExecuteCallbacks(GameObject* gameObject)
{
	for (int i = 0; i < gameObject->components.size(); ++i)
	{
		Component* comp = gameObject->components[i];
		if (comp->GetType() == ComponentTypes::ScriptComponent)
		{
			ComponentScript* script = (ComponentScript*)comp;

			script->OnEnableMethod();
			script->Awake();
			script->Start();
		}
	}

	for (int i = 0; i < gameObject->children.size(); ++i)
	{
		ExecuteCallbacks(gameObject->children[i]);
	}
}

void ScriptingModule::InitPlayerPrefs()
{
	if (App->fs->Exists("PrefDir/playerPrefs.jb"))
	{
		char* buffer;
		uint size = App->fs->Load("PrefDir/playerPrefs.jb", &buffer);
		if (size > 0)
		{
			playerPrefs = json_parse_string(buffer);
			playerPrefsOBJ = json_value_get_object(playerPrefs);
		}
	}
	else
	{
		playerPrefs = json_value_init_object();
		playerPrefsOBJ = json_value_get_object(playerPrefs);
	}
}

//-----------------------------

void DebugLogTranslator(MonoString* msg)
{
	MonoError error;
	char* string = mono_string_to_utf8_checked(msg, &error);

	if (!mono_error_ok(&error))
		return;

	CONSOLE_LOG(LogTypes::Normal, string);

	mono_free(string);
}

void DebugLogWarningTranslator(MonoString* msg)
{
	MonoError error;
	char* string = mono_string_to_utf8_checked(msg, &error);

	if (!mono_error_ok(&error))
		return;

	CONSOLE_LOG(LogTypes::Warning, string);

	mono_free(string);
}

void DebugLogErrorTranslator(MonoString* msg)
{
	MonoError error;
	char* string = mono_string_to_utf8_checked(msg, &error);

	if (!mono_error_ok(&error))
		return;

	CONSOLE_LOG(LogTypes::Error, string)

	mono_free(string);
}

void ClearConsole() 
{ 
#ifndef GAMEMODE
	App->gui->ClearConsole();
#endif
}

int32_t GetKeyStateCS(int32_t key)
{
	return App->input->GetKey(key);
}

int32_t GetMouseStateCS(int32_t button)
{
	return App->input->GetMouseButton(button);
}

MonoArray* GetMousePosCS()
{
	MonoArray* ret = mono_array_new(App->scripting->domain, mono_get_int32_class(), 2);
	int x = App->input->GetMouseX();
	int y = App->input->GetMouseY();
	mono_array_set(ret, float, 0, x);
	mono_array_set(ret, float, 1, y);

	return ret;
}

MonoArray* GetMouseDeltaPosCS()
{
	MonoArray* ret = mono_array_new(App->scripting->domain, mono_get_int32_class(), 2);
	mono_array_set(ret, float, 0, App->input->GetMouseXMotion());
	mono_array_set(ret, float, 1, App->input->GetMouseYMotion());

	return ret;
}

int GetWheelMovementCS()
{
	return App->input->GetMouseZ();
}

MonoObject* InstantiateGameObject(MonoObject* templateMO, MonoArray* position, MonoArray* rotation)
{
	if (!templateMO)
	{
		//Instantiate an empty GameObject and returns the MonoObject

		GameObject* instance = App->GOs->CreateGameObject("default", App->scene->root);

		MonoClass* gameObjectClass = mono_class_from_name(App->scripting->internalImage, "JellyBitEngine", "GameObject");
		MonoObject* monoInstance = mono_object_new(App->scripting->domain, gameObjectClass);
		mono_runtime_object_init(monoInstance);

		if (position)
		{
			math::float3 newPos{mono_array_get(position, float, 0), mono_array_get(position, float, 1), mono_array_get(position, float, 2)};
			instance->transform->SetPosition(newPos);
		}

		if (rotation)
		{
			math::Quat newRotation{ mono_array_get(position, float, 0), mono_array_get(position, float, 1), mono_array_get(position, float, 2), mono_array_get(position, float, 3)};
			instance->transform->SetRotation(newRotation);
		}

		uint32_t handleID = mono_gchandle_new(monoInstance, true);

		instance->SetMonoObject(handleID);

		int address = (int)instance;
		mono_field_set_value(monoInstance, mono_class_get_field_from_name(gameObjectClass, "cppAddress"), &instance);

		App->scripting->monoObjectHandles.push_back(handleID);

		return monoInstance;
	}

	else
	{
		//Search for the monoTemplate and his GameObject representation in the map, create 2 new copies,
		//add the GameObject to the Scene Hierarchy and returns the monoObject. Store this new Instantiated objects in the map.

		GameObject* templateGO = nullptr;

		for (int i = 0; i < App->scripting->monoObjectHandles.size(); ++i)
		{
			uint32_t handleID = App->scripting->monoObjectHandles[i];
			MonoObject* temp = mono_gchandle_get_target(handleID);

			if (temp == templateMO)
			{
				int address;
				mono_field_get_value(temp, mono_class_get_field_from_name(mono_object_get_class(temp), "cppAddress"), &address);
				templateGO = (GameObject*)address;
				break;
			}
		}

		if (!templateGO)
		{
			//The user may be trying to instantiate a GameObject created through script. 
			//This feature is not implemented for now.
			CONSOLE_LOG(LogTypes::Error,	"Missing GameObject/MonoObject pair when instantiating from a MonoObject template.\n"
											"Instantiating from a GameObject created through script is not supported for now.\n");
			return nullptr;
		}

		GameObject* newGameObject = App->GOs->Instanciate(templateGO, App->scene->root);
		MonoObject* moInstance = App->scripting->MonoObjectFrom(newGameObject);

		if (position)
		{
			math::float3 newPos{ mono_array_get(position, float, 0), mono_array_get(position, float, 1), mono_array_get(position, float, 2) };
			newGameObject->transform->SetPosition(newPos);
		}

		if (rotation)
		{
			math::Quat newRotation{ mono_array_get(position, float, 0), mono_array_get(position, float, 1), mono_array_get(position, float, 2), mono_array_get(position, float, 3) };
			newGameObject->transform->SetRotation(newRotation);
		}

		App->scripting->ExecuteCallbacks(newGameObject);

		return moInstance;
	}
}

void DestroyObj(MonoObject* obj)
{
	bool found = false;

	std::string className = mono_class_get_name(mono_object_get_class(obj));

	if (className == "GameObject")
	{
		for (int i = 0; i < App->scripting->monoObjectHandles.size(); ++i)
		{
			if (obj == mono_gchandle_get_target(App->scripting->monoObjectHandles[i]))
			{
				found = true;

				MonoClass* monoClass = mono_object_get_class(obj);
				MonoClassField* destroyed = mono_class_get_field_from_name(monoClass, "destroyed");
				mono_field_set_value(obj, destroyed, &found);

				int address;
				mono_field_get_value(obj, mono_class_get_field_from_name(monoClass, "cppAddress"), &address);

				GameObject* toDelete = (GameObject*)address;

				mono_gchandle_free(App->scripting->monoObjectHandles[i]);

				App->scripting->monoObjectHandles.erase(App->scripting->monoObjectHandles.begin() + i);

				//Destroy this GameObject
				App->GOs->DeleteGameObject(toDelete);				

				break;
			}
		}
	}
}

MonoArray* QuatMult(MonoArray* q1, MonoArray* q2)
{
	math::Quat _q1(mono_array_get(q1, float, 0), mono_array_get(q1, float, 1), mono_array_get(q1, float, 2), mono_array_get(q1, float, 3));
	math::Quat _q2(mono_array_get(q2, float, 0), mono_array_get(q2, float, 1), mono_array_get(q2, float, 2), mono_array_get(q2, float, 3));

	_q1.Normalize();
	_q2.Normalize();

	math::Quat result = _q1 * _q2;

	MonoArray* ret = mono_array_new(App->scripting->domain, mono_get_int32_class(), 4);
	mono_array_set(ret, float, 0, result.x);
	mono_array_set(ret, float, 1, result.y);
	mono_array_set(ret, float, 2, result.z);
	mono_array_set(ret, float, 3, result.w);

	return ret;
}

MonoArray* QuatVec3(MonoArray* q, MonoArray* vec)
{
	math::Quat _q(mono_array_get(q, float, 0), mono_array_get(q, float, 1), mono_array_get(q, float, 2), mono_array_get(q, float, 3));
	_q.Normalize();

	math::float3 _vec(mono_array_get(vec, float, 0), mono_array_get(vec, float, 1), mono_array_get(vec, float, 2));

	math::float3 res = _q * _vec;

	MonoArray* ret = mono_array_new(App->scripting->domain, mono_get_int32_class(), 3);
	mono_array_set(ret, float, 0, res.x);
	mono_array_set(ret, float, 1, res.y);
	mono_array_set(ret, float, 2, res.z);

	return ret;
}

MonoArray* ToEuler(MonoArray* quat)
{
	math::Quat _q(mono_array_get(quat, float, 0), mono_array_get(quat, float, 1), mono_array_get(quat, float, 2), mono_array_get(quat, float, 3));

	math::float3 axis;
	float angle;
	_q.ToAxisAngle(axis, angle);

	math::float3 euler = axis * math::RadToDeg(angle);

	MonoArray* ret = mono_array_new(App->scripting->domain, mono_get_int32_class(), 3);
	mono_array_set(ret, float, 0, euler.x);
	mono_array_set(ret, float, 1, euler.y);
	mono_array_set(ret, float, 2, euler.z);

	return ret;
}

MonoArray* RotateAxisAngle(MonoArray* axis, float deg)
{
	math::float3 _axis({ mono_array_get(axis, float, 0), mono_array_get(axis, float, 1), mono_array_get(axis, float, 2)});

	float rad = math::DegToRad(deg);

	math::Quat rotation = math::Quat::RotateAxisAngle(_axis, rad);

	MonoArray* ret = mono_array_new(App->scripting->domain, mono_get_int32_class(), 4);
	mono_array_set(ret, float, 0, rotation.x);
	mono_array_set(ret, float, 1, rotation.y);
	mono_array_set(ret, float, 2, rotation.z);
	mono_array_set(ret, float, 3, rotation.w);

	return ret;
}

MonoString* GetGOName(MonoObject* monoObject)
{
	int address;
	mono_field_get_value(monoObject, mono_class_get_field_from_name(mono_object_get_class(monoObject), "cppAddress"), &address);

	GameObject* gameObject = (GameObject*)address;

	if (!gameObject)
		return nullptr;

	return mono_string_new(App->scripting->domain, gameObject->GetName());
}

void SetGOName(MonoObject* monoObject, MonoString* monoString)
{
	int address;
	mono_field_get_value(monoObject, mono_class_get_field_from_name(mono_object_get_class(monoObject), "cppAddress"), &address);

	GameObject* gameObject = (GameObject*)address;

	if (!gameObject)
		return;

	char* newName = mono_string_to_utf8(monoString);
	gameObject->SetName(newName);
	mono_free(newName);
}

float GetDeltaTime()
{
	return App->timeManager->GetDt();
}

float GetRealDeltaTime()
{
	return App->timeManager->GetRealDt();
}

float GetFixedDeltaTime()
{
	return App->physics->GetFixedDT();
}

float GetTime()
{
	return App->timeManager->GetGameTime();
}

float GetRealTime()
{
	return App->timeManager->GetRealTime();
}

MonoArray* GetLocalPosition(MonoObject* monoObject)
{
	int address;
	mono_field_get_value(monoObject, mono_class_get_field_from_name(mono_object_get_class(monoObject), "cppAddress"), &address);

	GameObject* gameObject = (GameObject*)address;

	if (!gameObject)
		return nullptr;

	if (!gameObject->transform)
		return nullptr;

	MonoArray* ret = mono_array_new(App->scripting->domain, mono_get_int32_class(), 3);

	math::float3 pos = gameObject->transform->GetPosition();
	mono_array_set(ret, float, 0, pos.x);
	mono_array_set(ret, float, 1, pos.y);
	mono_array_set(ret, float, 2, pos.z);

	return ret;
}

void SetLocalPosition(MonoObject* monoObject, MonoArray* position)
{
	int address;
	mono_field_get_value(monoObject, mono_class_get_field_from_name(mono_object_get_class(monoObject), "cppAddress"), &address);

	GameObject* gameObject = (GameObject*)address;

	if (!gameObject)
		return;

	if (!gameObject->transform)
		return;

	math::float3 pos = math::float3::zero;
	pos.x = mono_array_get(position, float, 0);
	pos.y = mono_array_get(position, float, 1);
	pos.z = mono_array_get(position, float, 2);
	gameObject->transform->SetPosition(pos);
}

MonoArray* GetLocalRotation(MonoObject* monoObject)
{
	int address;
	mono_field_get_value(monoObject, mono_class_get_field_from_name(mono_object_get_class(monoObject), "cppAddress"), &address);

	GameObject* gameObject = (GameObject*)address;

	if (!gameObject)
		return nullptr;

	if (!gameObject->transform)
		return nullptr;

	MonoArray* ret = mono_array_new(App->scripting->domain, mono_get_int32_class(), 4);

	math::Quat rot = gameObject->transform->GetRotation();
	mono_array_set(ret, float, 0, rot.x);
	mono_array_set(ret, float, 1, rot.y);
	mono_array_set(ret, float, 2, rot.z);
	mono_array_set(ret, float, 3, rot.w);

	return ret;
}

void SetLocalRotation(MonoObject* monoObject, MonoArray* rotation)
{
	int address;
	mono_field_get_value(monoObject, mono_class_get_field_from_name(mono_object_get_class(monoObject), "cppAddress"), &address);

	GameObject* gameObject = (GameObject*)address;

	if (!gameObject)
		return;

	if (!gameObject->transform)
		return;

	math::Quat rot = math::Quat::identity;
	rot.x = mono_array_get(rotation, float, 0);
	rot.y = mono_array_get(rotation, float, 1);
	rot.z = mono_array_get(rotation, float, 2);
	rot.w = mono_array_get(rotation, float, 3);
	gameObject->transform->SetRotation(rot);
}

MonoArray* GetLocalScale(MonoObject* monoObject)
{
	int address;
	mono_field_get_value(monoObject, mono_class_get_field_from_name(mono_object_get_class(monoObject), "cppAddress"), &address);

	GameObject* gameObject = (GameObject*)address;

	if (!gameObject)
		return nullptr;

	if (!gameObject->transform)
		return nullptr;

	MonoArray* ret = mono_array_new(App->scripting->domain, mono_get_int32_class(), 3);


	math::float3 scale = gameObject->transform->GetScale();
	mono_array_set(ret, float, 0, scale.x);
	mono_array_set(ret, float, 1, scale.y);
	mono_array_set(ret, float, 2, scale.z);

	return ret;
}

void SetLocalScale(MonoObject* monoObject, MonoArray* scale)
{
	int address;
	mono_field_get_value(monoObject, mono_class_get_field_from_name(mono_object_get_class(monoObject), "cppAddress"), &address);

	GameObject* gameObject = (GameObject*)address;

	if (!gameObject)
		return;

	if (!gameObject->transform)
		return;


	math::float3 newScale = math::float3::zero;
	newScale.x = mono_array_get(scale, float, 0);
	newScale.y = mono_array_get(scale, float, 1);
	newScale.z = mono_array_get(scale, float, 2);
	gameObject->transform->SetScale(newScale);
}

MonoArray* GetGlobalPos(MonoObject* monoObject)
{
	GameObject* gameObject = nullptr;

	int address;
	mono_field_get_value(monoObject, mono_class_get_field_from_name(mono_object_get_class(monoObject), "cppAddress"), &address);

	gameObject = (GameObject*)address;

	if (!gameObject)
		return nullptr;

	if (!gameObject->transform)
		return nullptr;

	math::float3 position, scale;
	math::Quat rotation;

	math::float4x4 global = gameObject->transform->GetGlobalMatrix();
	global.Decompose(position, rotation, scale);

	MonoArray* ret = mono_array_new(App->scripting->domain, mono_get_int32_class(), 3);
	mono_array_set(ret, float, 0, position.x);
	mono_array_set(ret, float, 1, position.y);
	mono_array_set(ret, float, 2, position.z);

	return ret;
}

MonoArray* GetGlobalRotation(MonoObject* monoObject)
{
	GameObject* gameObject = nullptr;

	int address;
	mono_field_get_value(monoObject, mono_class_get_field_from_name(mono_object_get_class(monoObject), "cppAddress"), &address);

	gameObject = (GameObject*)address;

	if (!gameObject)
		return nullptr;

	if (!gameObject->transform)
		return nullptr;

	math::float3 position, scale;
	math::Quat rotation;

	math::float4x4 global = gameObject->transform->GetGlobalMatrix();
	global.Decompose(position, rotation, scale);

	MonoArray* ret = mono_array_new(App->scripting->domain, mono_get_int32_class(), 4);
	mono_array_set(ret, float, 0, rotation.x);
	mono_array_set(ret, float, 1, rotation.y);
	mono_array_set(ret, float, 2, rotation.z);
	mono_array_set(ret, float, 3, rotation.w);

	return ret;
}

MonoArray* GetGlobalScale(MonoObject* monoObject)
{
	GameObject* gameObject = nullptr;

	int address;
	mono_field_get_value(monoObject, mono_class_get_field_from_name(mono_object_get_class(monoObject), "cppAddress"), &address);

	gameObject = (GameObject*)address;

	if (!gameObject)
		return nullptr;

	if (!gameObject->transform)
		return nullptr;

	math::float3 position, scale;
	math::Quat rotation;

	math::float4x4 global = gameObject->transform->GetGlobalMatrix();
	global.Decompose(position, rotation, scale);

	MonoArray* ret = mono_array_new(App->scripting->domain, mono_get_int32_class(), 3);
	mono_array_set(ret, float, 0, scale.x);
	mono_array_set(ret, float, 1, scale.y);
	mono_array_set(ret, float, 2, scale.z);

	return ret;
}

void SetGlobalPos(MonoObject* monoObject, MonoArray* globalPos)
{
	math::float3 position, scale, newGlobalPos;
	math::Quat rotation;

	GameObject* gameObject = App->scripting->GameObjectFrom(monoObject);
	if (!gameObject || !gameObject->transform)
		return;

	math::float4x4 global = gameObject->transform->GetGlobalMatrix();
	global.Decompose(position, rotation, scale);

	newGlobalPos = {mono_array_get(globalPos, float, 0), mono_array_get(globalPos, float, 1), mono_array_get(globalPos, float, 2)};

	math::float4x4 newGlobal = math::float4x4::FromTRS(newGlobalPos, rotation, scale);
	gameObject->transform->SetMatrixFromGlobal(newGlobal);
}

void SetGlobalRot(MonoObject* monoObject, MonoArray* globalRot)
{
	math::float3 position, scale;
	math::Quat rotation, newGlobalRot;

	GameObject* gameObject = App->scripting->GameObjectFrom(monoObject);
	if (!gameObject || !gameObject->transform)
		return;

	math::float4x4 global = gameObject->transform->GetGlobalMatrix();
	global.Decompose(position, rotation, scale);

	newGlobalRot = { mono_array_get(globalRot, float, 0), mono_array_get(globalRot, float, 1), mono_array_get(globalRot, float, 2),  mono_array_get(globalRot, float, 3)};

	math::float4x4 newGlobal = math::float4x4::FromTRS(position, newGlobalRot, scale);
	gameObject->transform->SetMatrixFromGlobal(newGlobal);
}

void SetGlobalScale(MonoObject* monoObject, MonoArray* globalScale)
{
	math::float3 position, scale, newGlobalScale;
	math::Quat rotation;

	GameObject* gameObject = App->scripting->GameObjectFrom(monoObject);
	if (!gameObject || !gameObject->transform)
		return;

	math::float4x4 global = gameObject->transform->GetGlobalMatrix();
	global.Decompose(position, rotation, scale);

	newGlobalScale = { mono_array_get(globalScale, float, 0), mono_array_get(globalScale, float, 1), mono_array_get(globalScale, float, 2) };

	math::float4x4 newGlobal = math::float4x4::FromTRS(position, rotation, newGlobalScale);
	gameObject->transform->SetMatrixFromGlobal(newGlobal);
}

MonoObject* GetComponentByType(MonoObject* monoObject, MonoObject* type)
{
	if (!monoObject || !type)
		return nullptr;

	MonoObject* monoComp = nullptr;

	std::string className = mono_class_get_name(mono_object_get_class(type));

	if (className == "NavMeshAgent")
	{
		GameObject* gameObject = App->scripting->GameObjectFrom(monoObject);
		if (!gameObject)
			return nullptr;

		Component* comp = gameObject->GetComponent(ComponentTypes::NavAgentComponent);

		if (!comp)
			return nullptr;

		return App->scripting->MonoComponentFrom(comp);
	}
	else if (className == "Animator")
	{
		GameObject* gameObject = App->scripting->GameObjectFrom(monoObject);
		if (!gameObject)
			return nullptr;

		Component* comp = gameObject->GetComponent(ComponentTypes::AnimationComponent);

		if (!comp)
			return nullptr;

		return App->scripting->MonoComponentFrom(comp);
	}
	else if (className == "ParticleEmitter")
	{
		GameObject* gameObject = App->scripting->GameObjectFrom(monoObject);
		if (!gameObject)
			return nullptr;

		Component* comp = gameObject->GetComponent(ComponentTypes::EmitterComponent);

		if (!comp)
			return nullptr;

		return App->scripting->MonoComponentFrom(comp);
	}
	else if (className == "RectTransform")
	{
		GameObject* gameObject = App->scripting->GameObjectFrom(monoObject);
		if (!gameObject)
			return nullptr;

		Component* comp = gameObject->GetComponent(ComponentTypes::RectTransformComponent);

		if (!comp)
			return nullptr;

		return App->scripting->MonoComponentFrom(comp);
	}
	else if (className == "Button")
	{
		GameObject* gameObject = App->scripting->GameObjectFrom(monoObject);
		if (!gameObject)
			return nullptr;

		Component* comp = gameObject->GetComponent(ComponentTypes::ButtonComponent);

		if (!comp)
			return nullptr;

		return App->scripting->MonoComponentFrom(comp);
	}
	else if (className == "Image")
	{
		GameObject* gameObject = App->scripting->GameObjectFrom(monoObject);
		if (!gameObject)
			return nullptr;

		Component* comp = gameObject->GetComponent(ComponentTypes::ImageComponent);

		if (!comp)
			return nullptr;

		return App->scripting->MonoComponentFrom(comp);
	}
	else if (className == "Rigidbody")
	{
		GameObject* gameObject = App->scripting->GameObjectFrom(monoObject);
		if (!gameObject)
			return nullptr;

		Component* comp = gameObject->GetComponent(ComponentTypes::RigidDynamicComponent);

		if (!comp)
			return nullptr;

		return App->scripting->MonoComponentFrom(comp);
	}
	else if (className == "Projector")
	{
		GameObject* gameObject = App->scripting->GameObjectFrom(monoObject);
		if (!gameObject)
			return nullptr;

		Component* comp = gameObject->GetComponent(ComponentTypes::ProjectorComponent);

		if (!comp)
			return nullptr;

		return App->scripting->MonoComponentFrom(comp);
	}
	else if (className == "AudioSource")
	{
		GameObject* gameObject = App->scripting->GameObjectFrom(monoObject);
		if (!gameObject)
			return nullptr;

		Component* comp = gameObject->GetComponent(ComponentTypes::AudioSourceComponent);

		if (!comp)
			return nullptr;

		return App->scripting->MonoComponentFrom(comp);
	}
	else if (className == "AudioListener")
	{
		GameObject* gameObject = App->scripting->GameObjectFrom(monoObject);
		if (!gameObject)
			return nullptr;

		Component* comp = gameObject->GetComponent(ComponentTypes::AudioListenerComponent);

		if (!comp)
			return nullptr;

		return App->scripting->MonoComponentFrom(comp);
	}
	else
	{
		//Check if this monoObject is destroyed

		GameObject* gameObject = App->scripting->GameObjectFrom(monoObject);
		if (!gameObject)
			return nullptr;

		//Find a script named as this class

		for (int i = 0; i < gameObject->components.size(); ++i)
		{
			Component* comp = gameObject->components[i];
			if (comp->GetType() == ComponentTypes::ScriptComponent)
			{
				ComponentScript* script = (ComponentScript*)comp;
				if (script->scriptName == className)
				{
					return script->GetMonoComponent();
				}
			}
		}
	}

	return monoComp;
}

MonoObject* GetGameCamera()
{
	return App->scripting->MonoComponentFrom(App->renderer3D->GetCurrentCamera());
}

MonoObject* ScreenToRay(MonoArray* screenCoordinates, MonoObject* cameraComponent)
{
	math::float2 screenPoint{ mono_array_get(screenCoordinates, float, 0),mono_array_get(screenCoordinates, float, 1) };

	//Get the camera component attached

	int compAddress;
	mono_field_get_value(cameraComponent, mono_class_get_field_from_name(mono_object_get_class(cameraComponent) , "componentAddress"), &compAddress);

	ComponentCamera* camera = (ComponentCamera*)compAddress;
	if (!camera)
		return nullptr;

	//Get the MathGeoLib Ray
	math::Ray ray = camera->ScreenToRay(screenPoint);
	
	//Create the CSharp Ray object
	MonoClass* RayClass = mono_class_from_name(App->scripting->internalImage, "JellyBitEngine", "Ray");
	MonoObject* ret = mono_object_new(App->scripting->domain, RayClass);
	mono_runtime_object_init(ret);

	//SetUp the created Ray fields
	MonoClassField* positionField = mono_class_get_field_from_name(RayClass, "position");
	MonoClassField* directionField = mono_class_get_field_from_name(RayClass, "direction");
	
	MonoObject* positionObj; mono_field_get_value(ret, positionField, &positionObj);
	MonoObject* directionObj; mono_field_get_value(ret, directionField, &directionObj);

	mono_field_set_value(positionObj, mono_class_get_field_from_name(mono_object_get_class(positionObj), "_x"), &ray.pos.x);
	mono_field_set_value(positionObj, mono_class_get_field_from_name(mono_object_get_class(positionObj), "_y"), &ray.pos.y);
	mono_field_set_value(positionObj, mono_class_get_field_from_name(mono_object_get_class(positionObj), "_z"), &ray.pos.z);

	mono_field_set_value(directionObj, mono_class_get_field_from_name(mono_object_get_class(directionObj), "_x"), &ray.dir.x);
	mono_field_set_value(directionObj, mono_class_get_field_from_name(mono_object_get_class(directionObj), "_y"), &ray.dir.y);
	mono_field_set_value(directionObj, mono_class_get_field_from_name(mono_object_get_class(directionObj), "_z"), &ray.dir.z);

	return ret;
}

uint LayerToBit(MonoString* layerName)
{
	char* layerCName = mono_string_to_utf8(layerName);

	uint bits = 0; 
	int res = App->layers->NameToNumber(layerCName);
	res != -1 ? bits |= 1 << res : bits = 0;

	mono_free(layerCName);

	return bits;
}

bool Raycast(MonoArray* origin, MonoArray* direction, MonoObject** hitInfo, float maxDistance, uint filterMask, SceneQueryFlags sceneQueryFlags)
{
	math::float3 originCpp{mono_array_get(origin, float, 0), mono_array_get(origin, float, 1), mono_array_get(origin, float, 2)};
	math::float3 directionCpp{mono_array_get(direction, float, 0), mono_array_get(direction, float, 1), mono_array_get(direction, float, 2)};

	RaycastHit hitInfocpp;
	bool ret = App->physics->Raycast(originCpp, directionCpp, hitInfocpp, maxDistance, filterMask, sceneQueryFlags);

	if (ret)
	{
		//Create the HitInfo object
		MonoClass* raycastHitClass = mono_class_from_name(App->scripting->internalImage, "JellyBitEngine", "RaycastHit");
		*hitInfo = mono_object_new(App->scripting->domain, raycastHitClass);
		mono_runtime_object_init(*hitInfo);

		//Setup the gameObject and collider fields
		mono_field_set_value(*hitInfo, mono_class_get_field_from_name(raycastHitClass, "gameObject"), App->scripting->MonoObjectFrom(hitInfocpp.GetGameObject()));
		mono_field_set_value(*hitInfo, mono_class_get_field_from_name(raycastHitClass, "collider"), App->scripting->MonoComponentFrom((Component*)hitInfocpp.GetCollider()));

		//Setup the point field
		MonoClass* Vector3Class = mono_class_from_name(App->scripting->internalImage, "JellyBitEngine", "Vector3");
		MonoObject* pointObj = mono_object_new(App->scripting->domain, Vector3Class);
		mono_runtime_object_init(pointObj);

		math::float3 point = hitInfocpp.GetPoint();
		mono_field_set_value(pointObj, mono_class_get_field_from_name(Vector3Class, "_x"), &point.x);
		mono_field_set_value(pointObj, mono_class_get_field_from_name(Vector3Class, "_y"), &point.y);
		mono_field_set_value(pointObj, mono_class_get_field_from_name(Vector3Class, "_z"), &point.z);

		mono_field_set_value(*hitInfo, mono_class_get_field_from_name(raycastHitClass, "point"), pointObj);

		//Setup the normal field
		MonoObject* normalObj = mono_object_new(App->scripting->domain, Vector3Class);
		mono_runtime_object_init(normalObj);

		math::float3 normal = hitInfocpp.GetNormal();
		mono_field_set_value(normalObj, mono_class_get_field_from_name(Vector3Class, "_x"), &normal.x);
		mono_field_set_value(normalObj, mono_class_get_field_from_name(Vector3Class, "_y"), &normal.y);
		mono_field_set_value(normalObj, mono_class_get_field_from_name(Vector3Class, "_z"), &normal.z);

		mono_field_set_value(*hitInfo, mono_class_get_field_from_name(raycastHitClass, "normal"), normalObj);

		//Setup the texCoord field
		MonoClass* Vector2Class = mono_class_from_name(App->scripting->internalImage, "JellyBitEngine", "Vector2");
		MonoObject* texCoordObj = mono_object_new(App->scripting->domain, Vector2Class);
		mono_runtime_object_init(texCoordObj);

		math::float2 texCoord = hitInfocpp.GetTexCoord();
		mono_field_set_value(normalObj, mono_class_get_field_from_name(Vector2Class, "x"), &texCoord.x);
		mono_field_set_value(normalObj, mono_class_get_field_from_name(Vector2Class, "y"), &texCoord.y);

		mono_field_set_value(*hitInfo, mono_class_get_field_from_name(raycastHitClass, "texCoord"), texCoordObj);

		//Setup the distance and the faceIndex fields
		float distance = hitInfocpp.GetDistance();
		mono_field_set_value(*hitInfo, mono_class_get_field_from_name(raycastHitClass, "distance"), &distance);

		uint faceIndex = hitInfocpp.GetFaceIndex();
		mono_field_set_value(*hitInfo, mono_class_get_field_from_name(raycastHitClass, "faceIndex"), &faceIndex);
	}
	else
		*hitInfo = nullptr;

	return ret;
}

void SetDestination(MonoObject* navMeshAgent, MonoArray* newDestination)
{
	math::float3 newDestinationcpp(mono_array_get(newDestination, float, 0), mono_array_get(newDestination, float, 1), mono_array_get(newDestination, float, 2));
	int compAddress;
	mono_field_get_value(navMeshAgent, mono_class_get_field_from_name(mono_object_get_class(navMeshAgent), "componentAddress"), &compAddress);
	ComponentNavAgent* agent = (ComponentNavAgent*)compAddress;
	agent->SetDestination(newDestinationcpp.ptr());
}

float NavAgentGetRadius(MonoObject* compAgent)
{
	ComponentNavAgent* agent = (ComponentNavAgent*)App->scripting->ComponentFrom(compAgent);
	if (agent)
	{
		return agent->radius;
	}

	return 0.0f;
}

void NavAgentSetRadius(MonoObject* compAgent, float newRadius)
{
	ComponentNavAgent* agent = (ComponentNavAgent*)App->scripting->ComponentFrom(compAgent);
	if (agent)
	{
		agent->radius = newRadius;
		agent->UpdateParams();
	}
}

float NavAgentGetHeight(MonoObject* compAgent)
{
	ComponentNavAgent* agent = (ComponentNavAgent*)App->scripting->ComponentFrom(compAgent);
	if (agent)
	{
		return agent->height;
	}

	return 0.0f;
}

void NavAgentSetHeight(MonoObject* compAgent, float newHeight)
{
	ComponentNavAgent* agent = (ComponentNavAgent*)App->scripting->ComponentFrom(compAgent);
	if (agent)
	{
		agent->height = newHeight;
		agent->UpdateParams();
	}
}

float NavAgentGetMaxAcceleration(MonoObject* compAgent)
{
	ComponentNavAgent* agent = (ComponentNavAgent*)App->scripting->ComponentFrom(compAgent);
	if (agent)
	{
		return agent->maxAcceleration;
	}

	return 0.0f;
}

void NavAgentSetMaxAcceleration(MonoObject* compAgent, float newMaxAcceleration)
{
	ComponentNavAgent* agent = (ComponentNavAgent*)App->scripting->ComponentFrom(compAgent);
	if (agent)
	{
		agent->maxAcceleration = newMaxAcceleration;
		agent->UpdateParams();
	}
}

float NavAgentGetMaxSpeed(MonoObject* compAgent)
{
	ComponentNavAgent* agent = (ComponentNavAgent*)App->scripting->ComponentFrom(compAgent);
	if (agent)
	{
		return agent->maxSpeed;
	}

	return 0.0f;
}

void NavAgentSetMaxSpeed(MonoObject* compAgent, float newMaxSpeed)
{
	ComponentNavAgent* agent = (ComponentNavAgent*)App->scripting->ComponentFrom(compAgent);
	if (agent)
	{
		agent->maxSpeed = newMaxSpeed;
		agent->UpdateParams();
	}
}

float NavAgentGetSeparationWeight(MonoObject* compAgent)
{
	ComponentNavAgent* agent = (ComponentNavAgent*)App->scripting->ComponentFrom(compAgent);
	if (agent)
	{
		return agent->separationWeight;
	}

	return 0.0f;
}

void NavAgentSetSeparationWeight(MonoObject* compAgent, float newSeparationWeight)
{
	ComponentNavAgent* agent = (ComponentNavAgent*)App->scripting->ComponentFrom(compAgent);
	if (agent)
	{
		agent->separationWeight = newSeparationWeight;
		agent->UpdateParams();
	}
}

bool NavAgentIsWalking(MonoObject* compAgent)
{
	ComponentNavAgent* agent = (ComponentNavAgent*)App->scripting->ComponentFrom(compAgent);
	if (agent)
		return agent->IsWalking();

	return false;
}

void NavAgentRequestMoveVelocity(MonoObject* compAgent, MonoArray* direction)
{
	ComponentNavAgent* agent = (ComponentNavAgent*)App->scripting->ComponentFrom(compAgent);
	if (agent)
	{
		math::float3 dirCPP(mono_array_get(direction, float, 0), mono_array_get(direction, float, 1), mono_array_get(direction, float, 2));
		agent->RequestMoveVelocity(dirCPP.ptr());
	}		
}

void NavAgentResetMoveTarget(MonoObject* compAgent)
{
	ComponentNavAgent* agent = (ComponentNavAgent*)App->scripting->ComponentFrom(compAgent);
	if (agent)
	{
		agent->ResetMoveTarget();
	}
}

uint NavAgentGetParams(MonoObject* compAgent)
{
	ComponentNavAgent* agent = (ComponentNavAgent*)App->scripting->ComponentFrom(compAgent);
	if (agent)
	{
		return agent->params;
	}
	return 0;
}

void NavAgentSetParams(MonoObject* compAgent, uint params)
{
	ComponentNavAgent* agent = (ComponentNavAgent*)App->scripting->ComponentFrom(compAgent);
	if (agent)
	{
		agent->params = params;
		agent->UpdateParams();
	}
}

bool OverlapSphere(float radius, MonoArray* center, MonoArray** overlapHit, uint filterMask, SceneQueryFlags sceneQueryFlags)
{
	math::float3 centercpp(mono_array_get(center, float, 0), mono_array_get(center, float, 1), mono_array_get(center, float, 2));

	std::vector<OverlapHit> hits;
	if (App->physics->OverlapSphere(radius, centercpp, hits, filterMask, sceneQueryFlags))
	{
		uint hitsCount = hits.size();

		MonoClass* overlapClass = mono_class_from_name(App->scripting->internalImage, "JellyBitEngine", "OverlapHit");

		*overlapHit = mono_array_new(App->scripting->domain, overlapClass, hitsCount);

		for (int i = 0; i < hitsCount; ++i)
		{
			MonoObject* hitCSharp = mono_object_new(App->scripting->domain, overlapClass);
			mono_runtime_object_init(hitCSharp);

			OverlapHit hitCpp = hits[i];
			mono_field_set_value(hitCSharp, mono_class_get_field_from_name(overlapClass, "gameObject"), App->scripting->MonoObjectFrom(hitCpp.GetGameObject()));
			mono_field_set_value(hitCSharp, mono_class_get_field_from_name(overlapClass, "collider"), App->scripting->MonoComponentFrom((Component*)hitCpp.GetCollider()));

			uint faceIndex = hitCpp.GetFaceIndex();
			mono_field_set_value(hitCSharp, mono_class_get_field_from_name(overlapClass, "faceIndex"), &faceIndex);

			mono_array_setref(*overlapHit, i, hitCSharp);
		}

		return true;
	}
	else
		return false;
}

void SetCompActive(MonoObject* monoComponent, bool active)
{
	Component* component = App->scripting->ComponentFrom(monoComponent);
	component->IsActive() != active ? component->ToggleIsActive() : void();
}

void SetGameObjectActive(MonoObject* monoObject, bool active)
{
	GameObject* gameObject = App->scripting->GameObjectFrom(monoObject);
	gameObject->IsActive() != active ? gameObject->ToggleIsActive() : void();
}

bool GetGameObjectActive(MonoObject* monoObject, bool active)
{
	GameObject* gameObject = App->scripting->GameObjectFrom(monoObject);
	return gameObject->IsActive();
}

bool PlayAnimation(MonoObject* animatorComp, MonoString* animUUID)
{
	char* anim = mono_string_to_utf8(animUUID);

	ComponentAnimation* animator = (ComponentAnimation*)App->scripting->ComponentFrom(animatorComp);
	bool ret = animator->PlayAnimation(anim);
	
	mono_free(anim);

	return ret;
}

void ParticleEmitterPlay(MonoObject* particleComp)
{
	ComponentEmitter* emitter = (ComponentEmitter*)App->scripting->ComponentFrom(particleComp);
	if (emitter)
	{
		emitter->StartEmitter();
	}
}

void ParticleEmitterStop(MonoObject* particleComp)
{
	ComponentEmitter* emitter = (ComponentEmitter*)App->scripting->ComponentFrom(particleComp);
	if (emitter)
	{
		emitter->ClearEmitter();
	}
}

bool UIHovered()
{
	return App->ui->IsUIHovered();
}

MonoArray* RectTransform_GetRect(MonoObject* rectComp)
{
	MonoArray* ret = mono_array_new(App->scripting->domain, mono_get_uint32_class(), 4);

	ComponentRectTransform* rectCpp = (ComponentRectTransform*)App->scripting->ComponentFrom(rectComp);
	if (!rectCpp)
		return nullptr;

	uint* rectVector = rectCpp->GetRect();
	mono_array_set(ret, uint, 0, rectVector[0]);
	mono_array_set(ret, uint, 1, rectVector[1]);
	mono_array_set(ret, uint, 2, rectVector[2]);
	mono_array_set(ret, uint, 3, rectVector[3]);

	return ret;
}

void RectTransform_SetRect(MonoObject* rectComp, MonoArray* newRect)
{
	ComponentRectTransform* rectCpp = (ComponentRectTransform*)App->scripting->ComponentFrom(rectComp);
	if (!rectCpp)
		return;

	uint rectVector[4];
	rectVector[0] = mono_array_get(newRect, uint, 0);
	rectVector[1] = mono_array_get(newRect, uint, 1);
	rectVector[2] = mono_array_get(newRect, uint, 2);
	rectVector[3] = mono_array_get(newRect, uint, 3);
	
	rectCpp->SetRect(rectVector[0], rectVector[1], rectVector[2], rectVector[3]);
}

void ButtonSetKey(MonoObject* buttonComp, uint key)
{
	ComponentButton* button = (ComponentButton*)App->scripting->ComponentFrom(buttonComp);
	if (button)
	{
		button->SetNewKey(key);
	}
}

int ButtonGetState(MonoObject* buttonComp)
{
	ComponentButton* button = (ComponentButton*)App->scripting->ComponentFrom(buttonComp);
	if (button)
	{
		return button->GetState();
	}
}

void PlayerPrefsSave()
{
	uint size = json_serialization_size(App->scripting->playerPrefs);
	char* buffer = new char[size];
	JSON_Status status = json_serialize_to_buffer(App->scripting->playerPrefs, buffer, size);

	std::string writeDir = App->fs->GetWritePath();

	bool success = App->fs->SetWriteDir(App->fs->GetPrefDir());
	App->fs->Save("playerPrefs.jb", buffer, size);
	App->fs->SetWriteDir(writeDir);
}

void PlayerPrefsSetNumber(MonoString* key, double value)
{
	char* keyCpp = mono_string_to_utf8(key);
	json_object_set_number(App->scripting->playerPrefsOBJ, keyCpp, value);
	mono_free(keyCpp);
}

double PlayerPrefsGetNumber(MonoString* key)
{
	char* keyCpp = mono_string_to_utf8(key);
	double value = json_object_get_number(App->scripting->playerPrefsOBJ, keyCpp);
	mono_free(keyCpp);

	return value;
}

void PlayerPrefsSetString(MonoString* key, MonoString* string)
{
	char* keyCpp = mono_string_to_utf8(key);
	char* stringCpp = mono_string_to_utf8(string);

	json_object_set_string(App->scripting->playerPrefsOBJ, keyCpp, stringCpp);

	mono_free(keyCpp);
	mono_free(stringCpp);
}

MonoString* PlayerPrefsGetString(MonoString* key)
{
	char* keyCpp = mono_string_to_utf8(key);
	char* stringCpp = (char*)json_object_get_string(App->scripting->playerPrefsOBJ, keyCpp);
	
	if (!stringCpp)
		return nullptr;

	MonoString* string = mono_string_new(App->scripting->domain, stringCpp);
	
	mono_free(keyCpp);

	return string;
}

void PlayerPrefsSetBoolean(MonoString* key, bool boolean)
{
	char* keyCpp = mono_string_to_utf8(key);
	json_object_set_boolean(App->scripting->playerPrefsOBJ, keyCpp, boolean);
	mono_free(keyCpp);
}

bool PlayerPrefsGetBoolean(MonoString* key)
{
	char* keyCpp = mono_string_to_utf8(key);
	bool ret = json_object_get_boolean(App->scripting->playerPrefsOBJ, keyCpp);
	mono_free(keyCpp);

	return ret;
}

bool PlayerPrefsHasKey(MonoString* key)
{
	char* keyCpp = mono_string_to_utf8(key);

	bool ret = json_object_has_value(App->scripting->playerPrefsOBJ, keyCpp);

	mono_free(keyCpp);

	return ret;
}

void PlayerPrefsDeleteKey(MonoString* key)
{
	char* keyCpp = mono_string_to_utf8(key);

	json_object_remove(App->scripting->playerPrefsOBJ, keyCpp);

	mono_free(keyCpp);
}

void PlayerPrefsDeleteAll()
{
	json_object_clear(App->scripting->playerPrefsOBJ);
}

void SMLoadScene(MonoString* sceneName)
{
	char* sceneNameCPP = mono_string_to_utf8(sceneName);

	System_Event newEvent;
	newEvent.sceneEvent.type = System_Event_Type::LoadScene;
	memcpy(newEvent.sceneEvent.nameScene, sceneNameCPP, sizeof(char) * DEFAULT_BUF_SIZE);
	App->PushSystemEvent(newEvent);

	mono_free(sceneNameCPP);
}

MonoString* AudioSourceGetAudio(MonoObject* monoComp)
{
	ComponentAudioSource* source = (ComponentAudioSource*)App->scripting->ComponentFrom(monoComp);
	if (!source)
		return nullptr;

	std::string audio = source->GetAudioToPlay();
	return mono_string_new(App->scripting->domain, audio.c_str());
}

void AudioSourceSetAudio(MonoObject* monoComp, MonoString* newAudio)
{
	ComponentAudioSource* source = (ComponentAudioSource*)App->scripting->ComponentFrom(monoComp);
	if (!source)
		return;

	char* audio = mono_string_to_utf8(newAudio);

	source->SetAudio(audio);

	mono_free(audio);
}

bool AudioSourceGetMuted(MonoObject* monoComp)
{
	ComponentAudioSource* source = (ComponentAudioSource*)App->scripting->ComponentFrom(monoComp);
	if (!source)
		return false;

	return source->isMuted();
}

void AudioSourceSetMuted(MonoObject* monoComp, bool muted)
{
	ComponentAudioSource* source = (ComponentAudioSource*)App->scripting->ComponentFrom(monoComp);
	if (!source)
		return;

	source->SetMuted(muted);
}

bool AudioSourceGetBypassEffects(MonoObject* monoComp)
{
	ComponentAudioSource* source = (ComponentAudioSource*)App->scripting->ComponentFrom(monoComp);
	if (!source)
		return false;

	return source->GetBypassEffects();
}

void AudioSourceSetBypassEffects(MonoObject* monoComp, bool value)
{
	ComponentAudioSource* source = (ComponentAudioSource*)App->scripting->ComponentFrom(monoComp);
	if (!source)
		return;

	source->SetBypassEffects(value);
}

bool AudioSourceGetPlayOnAwake(MonoObject* monoComp)
{
	ComponentAudioSource* source = (ComponentAudioSource*)App->scripting->ComponentFrom(monoComp);
	if (!source)
		return false;

	return source->GetPlayOnAwake();
}

void AudioSourceSetPlayOnAwake(MonoObject* monoComp, bool value)
{
	ComponentAudioSource* source = (ComponentAudioSource*)App->scripting->ComponentFrom(monoComp);
	if (!source)
		return;

	source->SetPlayOnAwake(value);
}

bool AudioSourceGetLoop(MonoObject* monoComp)
{
	ComponentAudioSource* source = (ComponentAudioSource*)App->scripting->ComponentFrom(monoComp);
	if (!source)
		return false;

	return source->isInLoop();
}

void AudioSourceSetLoop(MonoObject* monoComp, bool value)
{
	ComponentAudioSource* source = (ComponentAudioSource*)App->scripting->ComponentFrom(monoComp);
	if (!source)
		return;

	source->SetLoop(value);
}

int AudioSourceGetPriority(MonoObject* monoComp)
{
	ComponentAudioSource* source = (ComponentAudioSource*)App->scripting->ComponentFrom(monoComp);
	if (!source)
		return -1;

	return source->GetPriority();
}

void AudioSourceSetPriority(MonoObject* monoComp, int value)
{
	ComponentAudioSource* source = (ComponentAudioSource*)App->scripting->ComponentFrom(monoComp);
	if (!source)
		return;

	source->SetPriority(value);
}

float AudioSourceGetVolume(MonoObject* monoComp)
{
	ComponentAudioSource* source = (ComponentAudioSource*)App->scripting->ComponentFrom(monoComp);
	if (!source)
		return -1.0f;

	return source->GetVolume();
}

void AudioSourceSetVolume(MonoObject* monoComp, float value)
{
	ComponentAudioSource* source = (ComponentAudioSource*)App->scripting->ComponentFrom(monoComp);
	if (!source)
		return;

	source->SetVolume(value);
}

bool AudioSourceGetMono(MonoObject* monoComp)
{
	ComponentAudioSource* source = (ComponentAudioSource*)App->scripting->ComponentFrom(monoComp);
	if (!source)
		return false;

	return source->isMono();
}

void AudioSourceSetMono(MonoObject* monoComp, bool value)
{
	ComponentAudioSource* source = (ComponentAudioSource*)App->scripting->ComponentFrom(monoComp);
	if (!source)
		return;

	source->SetMono(value);
}

float AudioSourceGetPitch(MonoObject* monoComp)
{
	ComponentAudioSource* source = (ComponentAudioSource*)App->scripting->ComponentFrom(monoComp);
	if (!source)
		return -1.0f;

	return source->GetPitch();
}

void AudioSourceSetPitch(MonoObject* monoComp, float value)
{
	ComponentAudioSource* source = (ComponentAudioSource*)App->scripting->ComponentFrom(monoComp);
	if (!source)
		return;

	source->SetPitch(value);
}

float AudioSourceGetStereoPanL(MonoObject* monoComp)
{
	ComponentAudioSource* source = (ComponentAudioSource*)App->scripting->ComponentFrom(monoComp);
	if (!source)
		return -1.0f;

	return source->GetStereoPanLeft();
}

void AudioSourceSetStereoPanL(MonoObject* monoComp, float value)
{
	ComponentAudioSource* source = (ComponentAudioSource*)App->scripting->ComponentFrom(monoComp);
	if (!source)
		return;

	source->SetStereoPanLeft(value);
}

float AudioSourceGetStereoPanR(MonoObject* monoComp)
{
	ComponentAudioSource* source = (ComponentAudioSource*)App->scripting->ComponentFrom(monoComp);
	if (!source)
		return -1.0f;

	return source->GetStereoPanRight();
}

void AudioSourceSetStereoPanR(MonoObject* monoComp, float value)
{
	ComponentAudioSource* source = (ComponentAudioSource*)App->scripting->ComponentFrom(monoComp);
	if (!source)
		return;

	source->SetStereoPanRight(value);
}

float AudioSourceGetMinDistance(MonoObject* monoComp)
{
	ComponentAudioSource* source = (ComponentAudioSource*)App->scripting->ComponentFrom(monoComp);
	if (!source)
		return -1.0f;

	return source->GetMinDistance();
}

void AudioSourceSetMinDistance(MonoObject* monoComp, float value)
{
	ComponentAudioSource* source = (ComponentAudioSource*)App->scripting->ComponentFrom(monoComp);
	if (!source)
		return;

	source->SetMinDistance(value);
}

float AudioSourceGetMaxDistance(MonoObject* monoComp)
{
	ComponentAudioSource* source = (ComponentAudioSource*)App->scripting->ComponentFrom(monoComp);
	if (!source)
		return -1.0f;

	return source->GetMaxDistance();
}

void AudioSourceSetMaxDistance(MonoObject* monoComp, float value)
{
	ComponentAudioSource* source = (ComponentAudioSource*)App->scripting->ComponentFrom(monoComp);
	if (!source)
		return;

	source->SetMaxDistance(value);
}

int AudioSourceGetState(MonoObject* monoComp)
{
	ComponentAudioSource* source = (ComponentAudioSource*)App->scripting->ComponentFrom(monoComp);
	if (!source)
		return -1;

	return source->GetState();
}

void AudioSourceSetState(MonoObject* monoComp, int value)
{
	ComponentAudioSource* source = (ComponentAudioSource*)App->scripting->ComponentFrom(monoComp);
	if (!source)
		return;

	source->SetState(value);
}

void AudioSourcePlayAudio(MonoObject* monoComp)
{
	ComponentAudioSource* source = (ComponentAudioSource*)App->scripting->ComponentFrom(monoComp);
	if (!source)
		return;

	source->PlayAudio();
}

void AudioSourcePauseAudio(MonoObject* monoComp)
{
	ComponentAudioSource* source = (ComponentAudioSource*)App->scripting->ComponentFrom(monoComp);
	if (!source)
		return;

	source->PauseAudio();
}

void AudioSourceResumeAudio(MonoObject* monoComp)
{
	ComponentAudioSource* source = (ComponentAudioSource*)App->scripting->ComponentFrom(monoComp);
	if (!source)
		return;

	source->ResumeAudio();
}

void AudioSourceStopAudio(MonoObject* monoComp)
{
	ComponentAudioSource* source = (ComponentAudioSource*)App->scripting->ComponentFrom(monoComp);
	if (!source)
		return;

	source->StopAudio();
}

void RigidbodyAddForce(MonoObject* monoComp, MonoArray* force, int mode)
{
	ComponentRigidDynamic* rigidbody = (ComponentRigidDynamic*)App->scripting->ComponentFrom(monoComp);
	if (!rigidbody)
		return;

	math::float3 forceCpp(mono_array_get(force, float, 0), mono_array_get(force, float, 1), mono_array_get(force, float, 2));

	rigidbody->AddForce(forceCpp, (physx::PxForceMode::Enum) mode);
}

void RigidbodyClearForce(MonoObject* monoComp)
{
	ComponentRigidDynamic* rigidbody = (ComponentRigidDynamic*)App->scripting->ComponentFrom(monoComp);
	if (!rigidbody)
		return;

	rigidbody->ClearForce();
}

void RigidbodyAddTorque(MonoObject* monoComp, MonoArray* torque, int mode)
{
	ComponentRigidDynamic* rigidbody = (ComponentRigidDynamic*)App->scripting->ComponentFrom(monoComp);
	if (!rigidbody)
		return;

	math::float3 torqueCpp(mono_array_get(torque, float, 0), mono_array_get(torque, float, 1), mono_array_get(torque, float, 2));

	rigidbody->AddTorque(torqueCpp, (physx::PxForceMode::Enum) mode);
}

void RigidbodyClearTorque(MonoObject* monoComp)
{
	ComponentRigidDynamic* rigidbody = (ComponentRigidDynamic*)App->scripting->ComponentFrom(monoComp);
	if (!rigidbody)
		return;

	rigidbody->ClearTorque();
}

//-----------------------------------------------------------------------------------------------------------------------------

void ScriptingModule::CreateDomain()
{
	static bool firstDomain = true;

	MonoDomain* nextDom = mono_domain_create_appdomain("The reloaded domain", NULL);
	if (!nextDom)
		return;

	if (!mono_domain_set(nextDom, false))
		return;
	
	//Make sure we do not delete the main domain
	if (!firstDomain)
	{
		mono_domain_unload(domain);		
	}

	domain = nextDom;

	char* buffer;
	int size = App->fs->Load("JellyBitCS.dll", &buffer);
	if(size <= 0)
		return;

	//Loading assemblies from data instead of from file
	MonoImageOpenStatus status = MONO_IMAGE_ERROR_ERRNO;
	internalImage = mono_image_open_from_data(buffer, size, 1, &status);
	internalAssembly = mono_assembly_load_from(internalImage, "InternalAssembly", &status);

	delete[] buffer;

	//SetUp Internal Calls
	mono_add_internal_call("JellyBitEngine.Debug::Log", (const void*)&DebugLogTranslator);
	mono_add_internal_call("JellyBitEngine.Debug::LogWarning", (const void*)&DebugLogWarningTranslator);
	mono_add_internal_call("JellyBitEngine.Debug::LogError", (const void*)&DebugLogErrorTranslator);
	mono_add_internal_call("JellyBitEngine.Debug::ClearConsole", (const void*)&ClearConsole);
	mono_add_internal_call("JellyBitEngine.GameObject::_Instantiate", (const void*)&InstantiateGameObject);
	mono_add_internal_call("JellyBitEngine.Input::GetKeyState", (const void*)&GetKeyStateCS);
	mono_add_internal_call("JellyBitEngine.Input::GetMouseButtonState", (const void*)&GetMouseStateCS);
	mono_add_internal_call("JellyBitEngine.Input::GetMousePos", (const void*)&GetMousePosCS);
	mono_add_internal_call("JellyBitEngine.Input::GetWheelMovement", (const void*)&GetWheelMovementCS);
	mono_add_internal_call("JellyBitEngine.Input::GetMouseDeltaPos", (const void*)&GetMouseDeltaPosCS);
	mono_add_internal_call("JellyBitEngine.Object::Destroy", (const void*)&DestroyObj);
	mono_add_internal_call("JellyBitEngine.Quaternion::quatMult", (const void*)&QuatMult);
	mono_add_internal_call("JellyBitEngine.Quaternion::quatVec3", (const void*)&QuatVec3);
	mono_add_internal_call("JellyBitEngine.Quaternion::toEuler", (const void*)&ToEuler);
	mono_add_internal_call("JellyBitEngine.Quaternion::RotateAxisAngle", (const void*)&RotateAxisAngle);
	mono_add_internal_call("JellyBitEngine.GameObject::getName", (const void*)&GetGOName);
	mono_add_internal_call("JellyBitEngine.GameObject::setName", (const void*)&SetGOName);
	mono_add_internal_call("JellyBitEngine.Time::getDeltaTime", (const void*)&GetDeltaTime);
	mono_add_internal_call("JellyBitEngine.Time::getRealDeltaTime", (const void*)&GetRealDeltaTime);
	mono_add_internal_call("JellyBitEngine.Time::getTime", (const void*)&GetTime);
	mono_add_internal_call("JellyBitEngine.Time::getRealTime", (const void*)&GetRealTime);
	mono_add_internal_call("JellyBitEngine.Time::getFixedDT", (const void*)&GetFixedDeltaTime);
	mono_add_internal_call("JellyBitEngine.Transform::getLocalPosition", (const void*)&GetLocalPosition);
	mono_add_internal_call("JellyBitEngine.Transform::setLocalPosition", (const void*)&SetLocalPosition);
	mono_add_internal_call("JellyBitEngine.Transform::getLocalRotation", (const void*)&GetLocalRotation);
	mono_add_internal_call("JellyBitEngine.Transform::setLocalRotation", (const void*)&SetLocalRotation);
	mono_add_internal_call("JellyBitEngine.Transform::getLocalScale", (const void*)&GetLocalScale);
	mono_add_internal_call("JellyBitEngine.Transform::setLocalScale", (const void*)&SetLocalScale);
	mono_add_internal_call("JellyBitEngine.Transform::getGlobalPos", (const void*)&GetGlobalPos);
	mono_add_internal_call("JellyBitEngine.Transform::getGlobalRotation", (const void*)&GetGlobalRotation);
	mono_add_internal_call("JellyBitEngine.Transform::getGlobalScale", (const void*)&GetGlobalScale);
	mono_add_internal_call("JellyBitEngine.Transform::setGlobalPos", (const void*)&SetGlobalPos);
	mono_add_internal_call("JellyBitEngine.Transform::setGlobalRotation", (const void*)&SetGlobalRot);
	mono_add_internal_call("JellyBitEngine.Transform::setGlobalScale", (const void*)&SetGlobalScale);
	mono_add_internal_call("JellyBitEngine.GameObject::GetComponentByType", (const void*)&GetComponentByType);
	mono_add_internal_call("JellyBitEngine.Camera::getMainCamera", (const void*)&GetGameCamera);
	mono_add_internal_call("JellyBitEngine.Physics::_ScreenToRay", (const void*)&ScreenToRay);
	mono_add_internal_call("JellyBitEngine.LayerMask::GetMaskBit", (const void*)&LayerToBit);
	mono_add_internal_call("JellyBitEngine.Physics::_Raycast", (const void*)&Raycast);
	mono_add_internal_call("JellyBitEngine.NavMeshAgent::_SetDestination", (const void*)&SetDestination);
	mono_add_internal_call("JellyBitEngine.Physics::_OverlapSphere", (const void*)&OverlapSphere);
	mono_add_internal_call("JellyBitEngine.Component::SetActive", (const void*)&SetCompActive);
	mono_add_internal_call("JellyBitEngine.Animator::PlayAnimation", (const void*)&PlayAnimation);
	mono_add_internal_call("JellyBitEngine.ParticleEmitter::Play", (const void*)&ParticleEmitterPlay);
	mono_add_internal_call("JellyBitEngine.ParticleEmitter::Stop", (const void*)&ParticleEmitterStop);
	mono_add_internal_call("JellyBitEngine.UI.UI::UIHovered", (const void*)&UIHovered);
	mono_add_internal_call("JellyBitEngine.UI.RectTransform::GetRect", (const void*)&RectTransform_GetRect);
	mono_add_internal_call("JellyBitEngine.UI.RectTransform::SetRect", (const void*)&RectTransform_SetRect);
	mono_add_internal_call("JellyBitEngine.UI.Button::SetKey", (const void*)&ButtonSetKey);
	mono_add_internal_call("JellyBitEngine.UI.Button::GetState", (const void*)&ButtonGetState);
	mono_add_internal_call("JellyBitEngine.GameObject::GetActive", (const void*)&GetGameObjectActive);
	mono_add_internal_call("JellyBitEngine.GameObject::SetActive", (const void*)&SetGameObjectActive);
	mono_add_internal_call("JellyBitEngine.PlayerPrefs::Save", (const void*)&PlayerPrefsSave);
	mono_add_internal_call("JellyBitEngine.PlayerPrefs::GetNumber", (const void*)&PlayerPrefsGetNumber);
	mono_add_internal_call("JellyBitEngine.PlayerPrefs::SetNumber", (const void*)&PlayerPrefsSetNumber);
	mono_add_internal_call("JellyBitEngine.PlayerPrefs::GetString", (const void*)&PlayerPrefsGetString);
	mono_add_internal_call("JellyBitEngine.PlayerPrefs::SetString", (const void*)&PlayerPrefsSetString);
	mono_add_internal_call("JellyBitEngine.PlayerPrefs::GetBoolean", (const void*)&PlayerPrefsGetBoolean);
	mono_add_internal_call("JellyBitEngine.PlayerPrefs::SetBoolean", (const void*)&PlayerPrefsSetBoolean);
	mono_add_internal_call("JellyBitEngine.PlayerPrefs::HasKey", (const void*)&PlayerPrefsHasKey);
	mono_add_internal_call("JellyBitEngine.PlayerPrefs::DeleteKey", (const void*)&PlayerPrefsDeleteKey);
	mono_add_internal_call("JellyBitEngine.PlayerPrefs::DeleteAll", (const void*)&PlayerPrefsDeleteAll);
	mono_add_internal_call("JellyBitEngine.SceneManager.SceneManager::LoadScene", (const void*)&SMLoadScene);
	mono_add_internal_call("JellyBitEngine.NavMeshAgent::GetRadius", (const void*)&NavAgentGetRadius);
	mono_add_internal_call("JellyBitEngine.NavMeshAgent::SetRadius", (const void*)&NavAgentSetRadius);
	mono_add_internal_call("JellyBitEngine.NavMeshAgent::GetHeight", (const void*)&NavAgentGetHeight);
	mono_add_internal_call("JellyBitEngine.NavMeshAgent::SetHeight", (const void*)&NavAgentSetHeight);
	mono_add_internal_call("JellyBitEngine.NavMeshAgent::GetMaxAcceleration", (const void*)&NavAgentGetMaxAcceleration);
	mono_add_internal_call("JellyBitEngine.NavMeshAgent::SetMaxAcceleration", (const void*)&NavAgentSetMaxAcceleration);
	mono_add_internal_call("JellyBitEngine.NavMeshAgent::GetMaxSpeed", (const void*)&NavAgentGetMaxSpeed);
	mono_add_internal_call("JellyBitEngine.NavMeshAgent::SetMaxSpeed", (const void*)&NavAgentSetMaxSpeed);
	mono_add_internal_call("JellyBitEngine.NavMeshAgent::GetSeparationWeight", (const void*)&NavAgentGetSeparationWeight);
	mono_add_internal_call("JellyBitEngine.NavMeshAgent::SetSeparationWeight", (const void*)&NavAgentSetSeparationWeight);
	mono_add_internal_call("JellyBitEngine.NavMeshAgent::isWalking", (const void*)&NavAgentIsWalking);
	mono_add_internal_call("JellyBitEngine.NavMeshAgent::_RequestMoveVelocity", (const void*)&NavAgentRequestMoveVelocity);
	mono_add_internal_call("JellyBitEngine.NavMeshAgent::ResetMoveTarget", (const void*)&NavAgentResetMoveTarget);
	mono_add_internal_call("JellyBitEngine.NavMeshAgent::GetParams", (const void*)&NavAgentGetParams);
	mono_add_internal_call("JellyBitEngine.NavMeshAgent::SetParams", (const void*)&NavAgentSetParams);
	mono_add_internal_call("JellyBitEngine.AudioSource::GetAudio", (const void*)&AudioSourceGetAudio);
	mono_add_internal_call("JellyBitEngine.AudioSource::SetAudio", (const void*)&AudioSourceSetAudio);
	mono_add_internal_call("JellyBitEngine.AudioSource::GetMuted", (const void*)&AudioSourceGetMuted);
	mono_add_internal_call("JellyBitEngine.AudioSource::SetMuted", (const void*)&AudioSourceSetMuted);
	mono_add_internal_call("JellyBitEngine.AudioSource::GetBypassEffects", (const void*)&AudioSourceGetBypassEffects);
	mono_add_internal_call("JellyBitEngine.AudioSource::SetBypassEffects", (const void*)&AudioSourceSetBypassEffects);
	mono_add_internal_call("JellyBitEngine.AudioSource::GetPlayOnAwake", (const void*)&AudioSourceGetPlayOnAwake);
	mono_add_internal_call("JellyBitEngine.AudioSource::SetPlayOnAwake", (const void*)&AudioSourceSetPlayOnAwake);
	mono_add_internal_call("JellyBitEngine.AudioSource::GetLoop", (const void*)&AudioSourceGetLoop);
	mono_add_internal_call("JellyBitEngine.AudioSource::SetLoop", (const void*)&AudioSourceSetLoop);
	mono_add_internal_call("JellyBitEngine.AudioSource::GetPriority", (const void*)&AudioSourceGetPriority);
	mono_add_internal_call("JellyBitEngine.AudioSource::SetPriority", (const void*)&AudioSourceSetPriority);
	mono_add_internal_call("JellyBitEngine.AudioSource::GetVolume", (const void*)&AudioSourceGetVolume);
	mono_add_internal_call("JellyBitEngine.AudioSource::SetVolume", (const void*)&AudioSourceSetVolume);
	mono_add_internal_call("JellyBitEngine.AudioSource::GetMono", (const void*)&AudioSourceGetMono);
	mono_add_internal_call("JellyBitEngine.AudioSource::SetMono", (const void*)&AudioSourceSetMono);
	mono_add_internal_call("JellyBitEngine.AudioSource::GetPitch", (const void*)&AudioSourceGetPitch);
	mono_add_internal_call("JellyBitEngine.AudioSource::SetPitch", (const void*)&AudioSourceSetPitch);
	mono_add_internal_call("JellyBitEngine.AudioSource::GetStereoPanL", (const void*)&AudioSourceGetStereoPanL);
	mono_add_internal_call("JellyBitEngine.AudioSource::SetStereoPanL", (const void*)&AudioSourceSetStereoPanL);
	mono_add_internal_call("JellyBitEngine.AudioSource::GetStereoPanR", (const void*)&AudioSourceGetStereoPanR);
	mono_add_internal_call("JellyBitEngine.AudioSource::SetStereoPanR", (const void*)&AudioSourceSetStereoPanR);
	mono_add_internal_call("JellyBitEngine.AudioSource::GetMinDistance", (const void*)&AudioSourceGetMinDistance);
	mono_add_internal_call("JellyBitEngine.AudioSource::SetMinDistance", (const void*)&AudioSourceSetMinDistance);
	mono_add_internal_call("JellyBitEngine.AudioSource::GetMaxDistance", (const void*)&AudioSourceGetMaxDistance);
	mono_add_internal_call("JellyBitEngine.AudioSource::SetMaxDistance", (const void*)&AudioSourceSetMaxDistance);
	mono_add_internal_call("JellyBitEngine.AudioSource::GetState", (const void*)&AudioSourceGetState);
	mono_add_internal_call("JellyBitEngine.AudioSource::SetState", (const void*)&AudioSourceSetState);
	mono_add_internal_call("JellyBitEngine.AudioSource::PlayAudio", (const void*)&AudioSourcePlayAudio);
	mono_add_internal_call("JellyBitEngine.AudioSource::PauseAudio", (const void*)&AudioSourcePauseAudio);
	mono_add_internal_call("JellyBitEngine.AudioSource::ResumeAudio", (const void*)&AudioSourceResumeAudio);
	mono_add_internal_call("JellyBitEngine.AudioSource::StopAudio", (const void*)&AudioSourceStopAudio);
	mono_add_internal_call("JellyBitEngine.Rigidbody::_AddForce", (const void*)&RigidbodyAddForce);
	mono_add_internal_call("JellyBitEngine.Rigidbody::_AddTorque", (const void*)&RigidbodyAddTorque);
	mono_add_internal_call("JellyBitEngine.Rigidbody::ClearForce", (const void*)&RigidbodyClearForce);
	mono_add_internal_call("JellyBitEngine.Rigidbody::ClearTorque", (const void*)&RigidbodyClearTorque);

	ClearMap();

	firstDomain = false;
}

void ScriptingModule::UpdateScriptingReferences()
{
	char* buffer;
	int size = App->fs->Load("Library/Scripts/Scripting.dll", &buffer);
	if (size <= 0)
		return;

	//Loading assemblies from data instead of from file
	MonoImageOpenStatus status = MONO_IMAGE_ERROR_ERRNO;
	scriptsImage = mono_image_open_from_data(buffer, size, 1, &status);
	scriptsAssembly = mono_assembly_load_from(scriptsImage, "ScriptingAssembly", &status);

	delete[] buffer;
}
