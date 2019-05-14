#ifndef __SCRIPTINGMODULE_H__
#define __SCRIPTINGMODULE_H__

#include "Module.h"

#include <string>
#include <vector>
#include <map>

#include "pugui/pugixml.hpp"

#include <mono/metadata/object.h>

class ComponentScript;
class ResourceScript;

struct Directory;

bool exec(const char* cmd, std::string& error = std::string());

class ScriptingModule : public Module
{
	friend MonoObject* InstantiateGameObject(MonoObject* templateMO, MonoArray* position, MonoArray* rotation);
	friend MonoObject* GetComponentByType(MonoObject* monoObject, MonoReflectionType* type);

	friend void PlayerPrefsSave();
	friend void PlayerPrefsSetNumber(MonoString* key, double value);
	friend double PlayerPrefsGetNumber(MonoString* key);
	friend void PlayerPrefsSetString(MonoString* key, MonoString* string);
	friend MonoString* PlayerPrefsGetString(MonoString* key);
	friend void PlayerPrefsSetBoolean(MonoString* key, bool boolean);
	friend bool PlayerPrefsGetBoolean(MonoString* key);
	friend bool PlayerPrefsHasKey(MonoString* key);
	friend void PlayerPrefsDeleteKey(MonoString* key);
	friend void PlayerPrefsDeleteAll();

public:
	ScriptingModule(bool start_enabled = true) : Module(start_enabled) { name = "ScriptingModule"; }
	~ScriptingModule() {}

	bool Init(JSON_Object* data) override;
	bool Start();
	update_status PreUpdate();
	update_status Update();
	update_status PostUpdate();
	bool CleanUp();

	void OnSystemEvent(System_Event event) override;

	void Play();

public:
	ComponentScript* CreateScriptComponent(std::string scriptName, ResourceScript* scriptRes);
	bool DestroyScript(ComponentScript* script);
	inline void AddScriptComponent(ComponentScript* script){scripts.push_back(script);}
	void ClearScriptComponent(ComponentScript* script);

	MonoObject* MonoObjectFrom(GameObject* gameObject, bool create = true);
	GameObject* GameObjectFrom(MonoObject* monoObject);

	MonoObject* MonoComponentFrom(Component* component, bool create = true);
	Component* ComponentFrom(MonoObject* monoComponent);

	void MarkAsDestroyed(GameObject* toDestroy);
	void MarkAsDestroyed(Component* toDestroy);

	bool alreadyCreated(std::string scriptName);

	void CreateScriptingProject();
	void ExecuteScriptingProject();
	void IncludeCSFiles();
	void IncludeCSFiles(pugi::xml_node& nodeToAppend, const Directory& dir);

	void CreateInternalCSProject();

	std::string getReferencePath()const;
	
	std::string clearSpaces(std::string& scriptName = std::string());

	void CreateDomain();
	void UpdateScriptingReferences();

	void ReInstance();

	Resource* ImportScriptResource(const char* file);
	void ScriptModified(const char* scriptPath);
	void RecompileScripts();

	void GameObjectKilled(GameObject* killed);
	void ComponentKilled(Component* killed);

	void FixedUpdate();
	void OnDrawGizmos();
	void OnDrawGizmosSelected();

	void TemporalSave();
	void TemporalLoad();

private:
	void UpdateMethods();
	void ExecuteCallbacks(GameObject* gameObject);
	void InitPlayerPrefs();

public:
	_MonoDomain*			runtimeDomain		= nullptr;
	_MonoDomain*			domain				= nullptr;
	_MonoAssembly*			internalAssembly	= nullptr;
	_MonoImage*				internalImage		= nullptr;

	_MonoAssembly*			scriptsAssembly		= nullptr;
	_MonoImage*				scriptsImage		= nullptr;

private:

	bool someScriptModified = false;
	bool engineOpened = true;
	std::vector<ComponentScript*> scripts;

	JSON_Value* playerPrefs = nullptr;
	JSON_Object* playerPrefsOBJ = nullptr;
	std::string playerPrefsPath;
};

#endif