#include "Component.h"
#include "GameObject.h"

#include "SimulationEvents.h"

#include <mono/metadata/object.h>

class ResourceScript;

class ComponentScript : public Component
{
public:
	ComponentScript(std::string scriptName, GameObject* gameObject = nullptr);
	ComponentScript(ComponentScript& copy, GameObject* gameObject, bool includeComponents = true);
	virtual ~ComponentScript();

	//NOTE: If you override this method, make sure to call the base class method. 
	//(Component::OnSystemEvent(event); at start)
	void OnSystemEvent(System_Event event);

	void Awake();
	void Start();
	void PreUpdate();
	void Update();
	void PostUpdate();
	void OnEnableMethod();
	void OnDisableMethod();
	void OnStop();
	void FixedUpdate();
	void OnCollisionEnter(Collision& collision);
	void OnCollisionStay(Collision& collision);
	void OnCollisionExit(Collision& collision);
	void OnTriggerEnter(Collision& collision);
	void OnTriggerStay(Collision& collision);
	void OnTriggerExit(Collision& collision);

	void OnEnable() override;
	void OnDisable() override;

	void OnUniqueEditor() override;
	
	uint GetInternalSerializationBytes();
	uint GetPublicVarsSerializationBytes() const;
	uint GetPublicVarsSerializationBytesFromBuffer(char* buffer) const;

	void SavePublicVars(char*& cursor) const;
	void LoadPublicVars(char*& cursor);

	void OnInternalSave(char*& cursor);
	void OnInternalLoad(char*& cursor);

public:
	void InstanceClass();
	void InstanceClass(MonoObject* classInstance);

public:
	bool awaked = false;
	std::string scriptName;

	uint scriptResUUID = 0;

	uint tempBufferBytes = 0u;
	char* tempBuffer = nullptr;

	MonoObject* classInstance = nullptr;
};