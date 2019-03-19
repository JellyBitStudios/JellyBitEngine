#include "Application.h"
#include "ModuleInput.h"
#include "ModuleResourceManager.h"
#include "ScriptingModule.h"
#include "ModuleLayers.h"
#include "ModuleGOs.h"

#include "ComponentScript.h"
#include "ResourceScript.h"
#include "ResourcePrefab.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imgui_stl.h"

#include <mono/metadata/assembly.h>
#include <mono/jit/jit.h>
#include <mono/metadata/mono-config.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/attrdefs.h>

ComponentScript::ComponentScript(std::string scriptName, GameObject* gameObject) : scriptName(scriptName), Component(gameObject, ComponentTypes::ScriptComponent)
{
	UUID = App->GenerateRandomNumber();
}

ComponentScript::ComponentScript(ComponentScript& copy, GameObject* parent, bool includeComponents) : scriptName(copy.scriptName), Component(parent, ComponentTypes::ScriptComponent)
{
	UUID = App->GenerateRandomNumber();
	this->scriptResUUID = copy.scriptResUUID;
	App->res->SetAsUsed(scriptResUUID);

	MonoObject* original = copy.GetMonoComponent();
	MonoObject* newInstance = mono_object_clone(original); //In theory it copies all the public variables too

	InstanceClass(newInstance);

	if(includeComponents)
		App->scripting->AddScriptComponent(this);
}

ComponentScript::~ComponentScript()
{
	if (scriptResUUID != 0)
	{
		ResourceScript* scriptRes = (ResourceScript*)App->res->GetResource(scriptResUUID);
		if (scriptRes)
			App->res->SetAsUnused(scriptRes->GetUuid());

		scriptResUUID = 0;
	}

	if (monoCompHandle != 0)
	{
		mono_gchandle_free(monoCompHandle);
		monoCompHandle = 0;
	}

	App->scripting->ClearScriptComponent(this);
}

void ComponentScript::OnSystemEvent(System_Event event)
{
	Component::OnSystemEvent(event);

	switch (event.type)
	{
		case System_Event_Type::LoadFinished:
		{
			LoadPublicVars(tempBuffer);

			delete[] tempBuffer;
			tempBuffer = nullptr;
			tempBufferBytes = 0u;

			break;
		}
		case System_Event_Type::GameObjectDestroyed:
		{
			MonoObject* monoInstance = GetMonoComponent();

			if (!monoInstance)
				return;

			MonoClass* monoClass = mono_object_get_class(monoInstance);

			void* it = NULL;
			MonoClassField* field = NULL;

			do
			{
				field = mono_class_get_fields(monoClass, &it);

				if (field)
				{
					MonoType* type = mono_field_get_type(field);
					char* typeName = mono_type_get_name(type);

					if (strcmp(typeName, "JellyBitEngine.GameObject") == 0)
					{
						MonoObject* storedMO;
						mono_field_get_value(monoInstance, field, &storedMO);

						if (!storedMO)
						{
							continue;
						}
								
						int address;
						mono_field_get_value(storedMO, mono_class_get_field_from_name(mono_object_get_class(storedMO), "cppAddress"), &address);

						GameObject* storedGO = (GameObject*)address;

						if (storedGO == event.goEvent.gameObject)
						{
							//Our referenced GO is being destroyed!
							mono_field_set_value(monoInstance, field, NULL);
						}
					}
				}
				
			} while (field != NULL);
		}
	}
}

void ComponentScript::Awake()
{
	ResourceScript* scriptRes = (ResourceScript*)App->res->GetResource(scriptResUUID);
	if (scriptRes && scriptRes->awakeMethod && !awaked)
	{
		MonoObject* exc = nullptr;
		if (IsTreeActive())
		{
			awaked = true;
			mono_runtime_invoke(scriptRes->awakeMethod, GetMonoComponent(), NULL, &exc);
			if (exc)
			{
				System_Event event;
				event.type = System_Event_Type::Pause;
				App->PushSystemEvent(event);
				App->Pause();

				MonoString* exceptionMessage = mono_object_to_string(exc, NULL);
				char* toLogMessage = mono_string_to_utf8(exceptionMessage);
				CONSOLE_LOG(LogTypes::Error, toLogMessage);
				mono_free(toLogMessage);
			}
		}
	}
}

void ComponentScript::Start()
{
	ResourceScript* scriptRes = (ResourceScript*)App->res->GetResource(scriptResUUID);
	if (scriptRes && scriptRes->startMethod)
	{
		MonoObject* exc = nullptr;
		if (IsTreeActive())
		{
			mono_runtime_invoke(scriptRes->startMethod, GetMonoComponent(), NULL, &exc);
			if (exc)
			{
				System_Event event;
				event.type = System_Event_Type::Pause;
				App->PushSystemEvent(event);
				App->Pause();

				MonoString* exceptionMessage = mono_object_to_string(exc, NULL);
				char* toLogMessage = mono_string_to_utf8(exceptionMessage);
				CONSOLE_LOG(LogTypes::Error, toLogMessage);
				mono_free(toLogMessage);
			}
		}
	}
}

void ComponentScript::PreUpdate()
{
	ResourceScript* scriptRes = (ResourceScript*)App->res->GetResource(scriptResUUID);
	if (scriptRes && scriptRes->preUpdateMethod)
	{
		MonoObject* exc = nullptr;
		if (IsTreeActive())
		{
			mono_runtime_invoke(scriptRes->preUpdateMethod, GetMonoComponent(), NULL, &exc);
			if (exc)
			{
				System_Event event;
				event.type = System_Event_Type::Pause;
				App->PushSystemEvent(event);
				App->Pause();

				MonoString* exceptionMessage = mono_object_to_string(exc, NULL);
				char* toLogMessage = mono_string_to_utf8(exceptionMessage);
				CONSOLE_LOG(LogTypes::Error, toLogMessage);
				mono_free(toLogMessage);
			}
		}
	}
}

void ComponentScript::Update()
{
	ResourceScript* scriptRes = (ResourceScript*)App->res->GetResource(scriptResUUID);
	if (scriptRes && scriptRes->updateMethod)
	{
		MonoObject* exc = nullptr;
		if (IsTreeActive())
		{
			mono_runtime_invoke(scriptRes->updateMethod, GetMonoComponent(), NULL, &exc);
			if (exc)
			{
				System_Event event;
				event.type = System_Event_Type::Pause;
				App->PushSystemEvent(event);
				App->Pause();

				MonoString* exceptionMessage = mono_object_to_string(exc, NULL);
				char* toLogMessage = mono_string_to_utf8(exceptionMessage);
				CONSOLE_LOG(LogTypes::Error, toLogMessage);
				mono_free(toLogMessage);
			}
		}
	}
}

void ComponentScript::PostUpdate()
{
	ResourceScript* scriptRes = (ResourceScript*)App->res->GetResource(scriptResUUID);
	if (scriptRes && scriptRes->postUpdateMethod)
	{
		MonoObject* exc = nullptr;
		if (IsTreeActive())
		{
			mono_runtime_invoke(scriptRes->postUpdateMethod, GetMonoComponent(), NULL, &exc);
			if (exc)
			{
				System_Event event;
				event.type = System_Event_Type::Pause;
				App->PushSystemEvent(event);
				App->Pause();

				MonoString* exceptionMessage = mono_object_to_string(exc, NULL);
				char* toLogMessage = mono_string_to_utf8(exceptionMessage);
				CONSOLE_LOG(LogTypes::Error, toLogMessage);
				mono_free(toLogMessage);
			}
		}
	}
}

void ComponentScript::OnEnableMethod()
{
	ResourceScript* scriptRes = (ResourceScript*)App->res->GetResource(scriptResUUID);
	if (scriptRes && scriptRes->enableMethod)
	{
		MonoObject* exc = nullptr;
		if (IsTreeActive())
		{
			mono_runtime_invoke(scriptRes->enableMethod, GetMonoComponent(), NULL, &exc);
			if (exc)
			{
				System_Event event;
				event.type = System_Event_Type::Pause;
				App->PushSystemEvent(event);
				App->Pause();

				MonoString* exceptionMessage = mono_object_to_string(exc, NULL);
				char* toLogMessage = mono_string_to_utf8(exceptionMessage);
				CONSOLE_LOG(LogTypes::Error, toLogMessage);
				mono_free(toLogMessage);
			}
		}
	}
}

void ComponentScript::OnDisableMethod()
{
	ResourceScript* scriptRes = (ResourceScript*)App->res->GetResource(scriptResUUID);
	if (scriptRes && scriptRes->disableMethod)
	{
		MonoObject* exc = nullptr;
		mono_runtime_invoke(scriptRes->disableMethod, GetMonoComponent(), NULL, &exc);
		if (exc)
		{
			System_Event event;
			event.type = System_Event_Type::Pause;
			App->PushSystemEvent(event);
			App->Pause();

			MonoString* exceptionMessage = mono_object_to_string(exc, NULL);
			char* toLogMessage = mono_string_to_utf8(exceptionMessage);
			CONSOLE_LOG(LogTypes::Error, toLogMessage);
			mono_free(toLogMessage);
		}
	}
}

void ComponentScript::OnStop()
{
	ResourceScript* scriptRes = (ResourceScript*)App->res->GetResource(scriptResUUID);
	if (scriptRes && scriptRes->stopMethod)
	{
		MonoObject* exc = nullptr;
		if (IsTreeActive())
		{
			awaked = false;
			mono_runtime_invoke(scriptRes->stopMethod, GetMonoComponent(), NULL, &exc);
			if (exc)
			{
				System_Event event;
				event.type = System_Event_Type::Pause;
				App->PushSystemEvent(event);
				App->Pause();

				MonoString* exceptionMessage = mono_object_to_string(exc, NULL);
				char* toLogMessage = mono_string_to_utf8(exceptionMessage);
				CONSOLE_LOG(LogTypes::Error, toLogMessage);
				mono_free(toLogMessage);
			}
		}
	}
}

void ComponentScript::FixedUpdate()
{
	ResourceScript* scriptRes = (ResourceScript*)App->res->GetResource(scriptResUUID);
	if (scriptRes && scriptRes->fixedUpdateMethod)
	{
		MonoObject* exc = nullptr;
		if (IsTreeActive())
		{
			mono_runtime_invoke(scriptRes->fixedUpdateMethod, GetMonoComponent(), NULL, &exc);
			if (exc)
			{
				System_Event event;
				event.type = System_Event_Type::Pause;
				App->PushSystemEvent(event);
				App->Pause();

				MonoString* exceptionMessage = mono_object_to_string(exc, NULL);
				char* toLogMessage = mono_string_to_utf8(exceptionMessage);
				CONSOLE_LOG(LogTypes::Error, toLogMessage);
				mono_free(toLogMessage);
			}
		}
	}
}

void ComponentScript::OnCollisionEnter(Collision& collision)
{
	ResourceScript* scriptRes = (ResourceScript*)App->res->GetResource(scriptResUUID);
	if (scriptRes && scriptRes->OnCollisionEnterMethod)
	{
		MonoObject* exc = nullptr;
		if (IsTreeActive())
		{
			//Set the collision object
			MonoClass* collisionClass = mono_class_from_name(App->scripting->internalImage, "JellyBitEngine", "Collision");
			MonoObject* collisionOBJ = mono_object_new(App->scripting->domain, collisionClass);
			mono_runtime_object_init(collisionOBJ);

			//Set the gameObject
			mono_field_set_value(collisionOBJ, mono_class_get_field_from_name(collisionClass, "gameObject"), App->scripting->MonoObjectFrom(collision.GetGameObject()));

			//Set the collider
			mono_field_set_value(collisionOBJ, mono_class_get_field_from_name(collisionClass, "collider"), App->scripting->MonoComponentFrom((Component*)collision.GetCollider()));
			
			//Set the impulse
			MonoClass* vector3Class = mono_class_from_name(App->scripting->internalImage, "JellyBitEngine", "Vector3");
			MonoObject* impulseOBJ = mono_object_new(App->scripting->domain, vector3Class);
			mono_runtime_object_init(impulseOBJ);

			math::float3 impulse = collision.GetImpulse();
			mono_field_set_value(impulseOBJ, mono_class_get_field_from_name(vector3Class, "_x"), &impulse.x);
			mono_field_set_value(impulseOBJ, mono_class_get_field_from_name(vector3Class, "_y"), &impulse.y);
			mono_field_set_value(impulseOBJ, mono_class_get_field_from_name(vector3Class, "_z"), &impulse.z);

			mono_field_set_value(collisionOBJ, mono_class_get_field_from_name(collisionClass, "impulse"), impulseOBJ);

			//Set the relative velocity
			MonoObject* relVelocityOBJ = mono_object_new(App->scripting->domain, vector3Class);
			mono_runtime_object_init(relVelocityOBJ);

			math::float3 relativeVelocity = collision.GetRelativeVelocity();
			mono_field_set_value(relVelocityOBJ, mono_class_get_field_from_name(vector3Class, "_x"), &relativeVelocity.x);
			mono_field_set_value(relVelocityOBJ, mono_class_get_field_from_name(vector3Class, "_y"), &relativeVelocity.y);
			mono_field_set_value(relVelocityOBJ, mono_class_get_field_from_name(vector3Class, "_z"), &relativeVelocity.z);

			mono_field_set_value(collisionOBJ, mono_class_get_field_from_name(collisionClass, "relativeVelocity"), &relVelocityOBJ);

			//Set the contacts
			std::vector<ContactPoint> contacts = collision.GetContactPoints();
			float numContacts = contacts.size();
			mono_field_set_value(collisionOBJ, mono_class_get_field_from_name(collisionClass, "contactCount"), &numContacts);

			MonoClass* ContactPointClass = mono_class_from_name(App->scripting->internalImage, "JellyBitEngine", "ContactPoint");
			MonoArray* contactArray = mono_array_new(App->scripting->domain, ContactPointClass, numContacts);

			for (int i = 0; i < numContacts; ++i)
			{
				ContactPoint contact = contacts[i];
				MonoObject* contactOBJ = mono_object_new(App->scripting->domain, ContactPointClass);
				mono_runtime_object_init(contactOBJ);

				//Set normal
				math::float3 normal = contact.GetNormal();
				MonoObject* normalOBJ = mono_object_new(App->scripting->domain, vector3Class);
				mono_runtime_object_init(normalOBJ);
				mono_field_set_value(normalOBJ, mono_class_get_field_from_name(vector3Class, "_x"), &normal.x);
				mono_field_set_value(normalOBJ, mono_class_get_field_from_name(vector3Class, "_y"), &normal.y);
				mono_field_set_value(normalOBJ, mono_class_get_field_from_name(vector3Class, "_z"), &normal.z);
				
				mono_field_set_value(contactOBJ, mono_class_get_field_from_name(ContactPointClass, "normal"), normalOBJ);

				//Set other collider
				mono_field_set_value(contactOBJ, mono_class_get_field_from_name(ContactPointClass, "otherCollider"), App->scripting->MonoComponentFrom((Component*)contact.GetOtherCollider()));

				//Set point
				math::float3 point = contact.GetPoint();
				MonoObject* pointOBJ = mono_object_new(App->scripting->domain, vector3Class);
				mono_runtime_object_init(pointOBJ);
				mono_field_set_value(pointOBJ, mono_class_get_field_from_name(vector3Class, "_x"), &point.x);
				mono_field_set_value(pointOBJ, mono_class_get_field_from_name(vector3Class, "_y"), &point.y);
				mono_field_set_value(pointOBJ, mono_class_get_field_from_name(vector3Class, "_z"), &point.z);

				mono_field_set_value(contactOBJ, mono_class_get_field_from_name(ContactPointClass, "point"), pointOBJ);

				//Set separation
				float separation = contact.GetSeparation();
				mono_field_set_value(contactOBJ, mono_class_get_field_from_name(ContactPointClass, "separation"), &separation);

				//Set this collider
				mono_field_set_value(contactOBJ, mono_class_get_field_from_name(ContactPointClass, "thisCollider"), App->scripting->MonoComponentFrom((Component*)contact.GetThisCollider()));

				mono_array_setref(contactArray, i, contactOBJ);
			}

			mono_field_set_value(collisionOBJ, mono_class_get_field_from_name(collisionClass, "contacts"), contactArray);

			void* params[1];
			params[0] = collisionOBJ;

			mono_runtime_invoke(scriptRes->OnCollisionEnterMethod, GetMonoComponent(), params, &exc);
			if (exc)
			{
				System_Event event;
				event.type = System_Event_Type::Pause;
				App->PushSystemEvent(event);
				App->Pause();

				MonoString* exceptionMessage = mono_object_to_string(exc, NULL);
				char* toLogMessage = mono_string_to_utf8(exceptionMessage);
				CONSOLE_LOG(LogTypes::Error, toLogMessage);
				mono_free(toLogMessage);
			}
		}
	}
}

void ComponentScript::OnCollisionStay(Collision& collision)
{
	ResourceScript* scriptRes = (ResourceScript*)App->res->GetResource(scriptResUUID);
	if (scriptRes && scriptRes->OnCollisionStayMethod)
	{
		MonoObject* exc = nullptr;
		if (IsTreeActive())
		{
			//Set the collision object
			MonoClass* collisionClass = mono_class_from_name(App->scripting->internalImage, "JellyBitEngine", "Collision");
			MonoObject* collisionOBJ = mono_object_new(App->scripting->domain, collisionClass);
			mono_runtime_object_init(collisionOBJ);

			//Set the gameObject
			mono_field_set_value(collisionOBJ, mono_class_get_field_from_name(collisionClass, "gameObject"), App->scripting->MonoObjectFrom(collision.GetGameObject()));

			//Set the collider
			mono_field_set_value(collisionOBJ, mono_class_get_field_from_name(collisionClass, "collider"), App->scripting->MonoComponentFrom((Component*)collision.GetCollider()));

			//Set the impulse
			MonoClass* vector3Class = mono_class_from_name(App->scripting->internalImage, "JellyBitEngine", "Vector3");
			MonoObject* impulseOBJ = mono_object_new(App->scripting->domain, vector3Class);
			mono_runtime_object_init(impulseOBJ);

			math::float3 impulse = collision.GetImpulse();
			mono_field_set_value(impulseOBJ, mono_class_get_field_from_name(vector3Class, "_x"), &impulse.x);
			mono_field_set_value(impulseOBJ, mono_class_get_field_from_name(vector3Class, "_y"), &impulse.y);
			mono_field_set_value(impulseOBJ, mono_class_get_field_from_name(vector3Class, "_z"), &impulse.z);

			mono_field_set_value(collisionOBJ, mono_class_get_field_from_name(collisionClass, "impulse"), impulseOBJ);

			//TODO: RELATIVE VELOCITY?

			//Set the contacts
			std::vector<ContactPoint> contacts = collision.GetContactPoints();
			float numContacts = contacts.size();
			mono_field_set_value(collisionOBJ, mono_class_get_field_from_name(collisionClass, "contactCount"), &numContacts);

			MonoClass* ContactPointClass = mono_class_from_name(App->scripting->internalImage, "JellyBitEngine", "ContactPoint");
			MonoArray* contactArray = mono_array_new(App->scripting->domain, ContactPointClass, numContacts);

			for (int i = 0; i < numContacts; ++i)
			{
				ContactPoint contact = contacts[i];
				MonoObject* contactOBJ = mono_object_new(App->scripting->domain, ContactPointClass);
				mono_runtime_object_init(contactOBJ);

				//Set normal
				math::float3 normal = contact.GetNormal();
				MonoObject* normalOBJ = mono_object_new(App->scripting->domain, vector3Class);
				mono_runtime_object_init(normalOBJ);
				mono_field_set_value(normalOBJ, mono_class_get_field_from_name(vector3Class, "_x"), &normal.x);
				mono_field_set_value(normalOBJ, mono_class_get_field_from_name(vector3Class, "_y"), &normal.y);
				mono_field_set_value(normalOBJ, mono_class_get_field_from_name(vector3Class, "_z"), &normal.z);

				mono_field_set_value(contactOBJ, mono_class_get_field_from_name(ContactPointClass, "normal"), normalOBJ);

				//Set other collider
				mono_field_set_value(contactOBJ, mono_class_get_field_from_name(ContactPointClass, "otherCollider"), App->scripting->MonoComponentFrom((Component*)collision.GetCollider()));

				//Set point
				math::float3 point = contact.GetPoint();
				MonoObject* pointOBJ = mono_object_new(App->scripting->domain, vector3Class);
				mono_runtime_object_init(pointOBJ);
				mono_field_set_value(pointOBJ, mono_class_get_field_from_name(vector3Class, "_x"), &point.x);
				mono_field_set_value(pointOBJ, mono_class_get_field_from_name(vector3Class, "_y"), &point.y);
				mono_field_set_value(pointOBJ, mono_class_get_field_from_name(vector3Class, "_z"), &point.z);

				mono_field_set_value(contactOBJ, mono_class_get_field_from_name(ContactPointClass, "point"), pointOBJ);

				//Set separation
				float separation = contact.GetSeparation();
				mono_field_set_value(contactOBJ, mono_class_get_field_from_name(ContactPointClass, "separation"), &separation);

				//TODO: SET THIS COLLIDER
				//mono_field_set_value(contactOBJ, mono_class_get_field_from_name(ContactPointClass, "thisCollider"), App->scripting->MonoComponentFrom((Component*)collision.GetThiCollider()));

				mono_array_setref(contactArray, i, contactOBJ);
			}

			mono_field_set_value(collisionOBJ, mono_class_get_field_from_name(collisionClass, "contacts"), contactArray);

			void* params[1];
			params[0] = collisionOBJ;

			mono_runtime_invoke(scriptRes->OnCollisionStayMethod, GetMonoComponent(), NULL, &exc);
			if (exc)
			{
				System_Event event;
				event.type = System_Event_Type::Pause;
				App->PushSystemEvent(event);
				App->Pause();

				MonoString* exceptionMessage = mono_object_to_string(exc, NULL);
				char* toLogMessage = mono_string_to_utf8(exceptionMessage);
				CONSOLE_LOG(LogTypes::Error, toLogMessage);
				mono_free(toLogMessage);
			}
		}
	}
}

void ComponentScript::OnCollisionExit(Collision& collision)
{
	ResourceScript* scriptRes = (ResourceScript*)App->res->GetResource(scriptResUUID);
	if (scriptRes && scriptRes->OnCollisionExitMethod)
	{
		MonoObject* exc = nullptr;
		if (IsTreeActive())
		{
			//Set the collision object
			MonoClass* collisionClass = mono_class_from_name(App->scripting->internalImage, "JellyBitEngine", "Collision");
			MonoObject* collisionOBJ = mono_object_new(App->scripting->domain, collisionClass);
			mono_runtime_object_init(collisionOBJ);

			//Set the gameObject
			mono_field_set_value(collisionOBJ, mono_class_get_field_from_name(collisionClass, "gameObject"), App->scripting->MonoObjectFrom(collision.GetGameObject()));

			//Set the collider
			mono_field_set_value(collisionOBJ, mono_class_get_field_from_name(collisionClass, "collider"), App->scripting->MonoComponentFrom((Component*)collision.GetCollider()));

			//Set the impulse
			MonoClass* vector3Class = mono_class_from_name(App->scripting->internalImage, "JellyBitEngine", "Vector3");
			MonoObject* impulseOBJ = mono_object_new(App->scripting->domain, vector3Class);
			mono_runtime_object_init(impulseOBJ);

			math::float3 impulse = collision.GetImpulse();
			mono_field_set_value(impulseOBJ, mono_class_get_field_from_name(vector3Class, "_x"), &impulse.x);
			mono_field_set_value(impulseOBJ, mono_class_get_field_from_name(vector3Class, "_y"), &impulse.y);
			mono_field_set_value(impulseOBJ, mono_class_get_field_from_name(vector3Class, "_z"), &impulse.z);

			mono_field_set_value(collisionOBJ, mono_class_get_field_from_name(collisionClass, "impulse"), impulseOBJ);

			//TODO: RELATIVE VELOCITY?

			//Set the contacts
			std::vector<ContactPoint> contacts = collision.GetContactPoints();
			float numContacts = contacts.size();
			mono_field_set_value(collisionOBJ, mono_class_get_field_from_name(collisionClass, "contactCount"), &numContacts);

			MonoClass* ContactPointClass = mono_class_from_name(App->scripting->internalImage, "JellyBitEngine", "ContactPoint");
			MonoArray* contactArray = mono_array_new(App->scripting->domain, ContactPointClass, numContacts);

			for (int i = 0; i < numContacts; ++i)
			{
				ContactPoint contact = contacts[i];
				MonoObject* contactOBJ = mono_object_new(App->scripting->domain, ContactPointClass);
				mono_runtime_object_init(contactOBJ);

				//Set normal
				math::float3 normal = contact.GetNormal();
				MonoObject* normalOBJ = mono_object_new(App->scripting->domain, vector3Class);
				mono_runtime_object_init(normalOBJ);
				mono_field_set_value(normalOBJ, mono_class_get_field_from_name(vector3Class, "_x"), &normal.x);
				mono_field_set_value(normalOBJ, mono_class_get_field_from_name(vector3Class, "_y"), &normal.y);
				mono_field_set_value(normalOBJ, mono_class_get_field_from_name(vector3Class, "_z"), &normal.z);

				mono_field_set_value(contactOBJ, mono_class_get_field_from_name(ContactPointClass, "normal"), normalOBJ);

				//Set other collider
				mono_field_set_value(contactOBJ, mono_class_get_field_from_name(ContactPointClass, "otherCollider"), App->scripting->MonoComponentFrom((Component*)collision.GetCollider()));

				//Set point
				math::float3 point = contact.GetPoint();
				MonoObject* pointOBJ = mono_object_new(App->scripting->domain, vector3Class);
				mono_runtime_object_init(pointOBJ);
				mono_field_set_value(pointOBJ, mono_class_get_field_from_name(vector3Class, "_x"), &point.x);
				mono_field_set_value(pointOBJ, mono_class_get_field_from_name(vector3Class, "_y"), &point.y);
				mono_field_set_value(pointOBJ, mono_class_get_field_from_name(vector3Class, "_z"), &point.z);

				mono_field_set_value(contactOBJ, mono_class_get_field_from_name(ContactPointClass, "point"), pointOBJ);

				//Set separation
				float separation = contact.GetSeparation();
				mono_field_set_value(contactOBJ, mono_class_get_field_from_name(ContactPointClass, "separation"), &separation);

				//TODO: SET THIS COLLIDER
				//mono_field_set_value(contactOBJ, mono_class_get_field_from_name(ContactPointClass, "thisCollider"), App->scripting->MonoComponentFrom((Component*)collision.GetThiCollider()));

				mono_array_setref(contactArray, i, contactOBJ);
			}

			mono_field_set_value(collisionOBJ, mono_class_get_field_from_name(collisionClass, "contacts"), contactArray);

			void* params[1];
			params[0] = collisionOBJ;

			mono_runtime_invoke(scriptRes->OnCollisionExitMethod, GetMonoComponent(), NULL, &exc);
			if (exc)
			{
				System_Event event;
				event.type = System_Event_Type::Pause;
				App->PushSystemEvent(event);
				App->Pause();

				MonoString* exceptionMessage = mono_object_to_string(exc, NULL);
				char* toLogMessage = mono_string_to_utf8(exceptionMessage);
				CONSOLE_LOG(LogTypes::Error, toLogMessage);
				mono_free(toLogMessage);
			}
		}
	}
}

void ComponentScript::OnTriggerEnter(Collision& collision)
{
	ResourceScript* scriptRes = (ResourceScript*)App->res->GetResource(scriptResUUID);
	if (scriptRes && scriptRes->OnTriggerEnterMethod)
	{
		MonoObject* exc = nullptr;
		if (IsTreeActive())
		{
			MonoObject* collider = App->scripting->MonoComponentFrom((Component*)collision.GetCollider());

			void* args[1];
			args[0] = collider;

			mono_runtime_invoke(scriptRes->OnTriggerEnterMethod, GetMonoComponent(), args, &exc);
			if (exc)
			{
				System_Event event;
				event.type = System_Event_Type::Pause;
				App->PushSystemEvent(event);
				App->Pause();

				MonoString* exceptionMessage = mono_object_to_string(exc, NULL);
				char* toLogMessage = mono_string_to_utf8(exceptionMessage);
				CONSOLE_LOG(LogTypes::Error, toLogMessage);
				mono_free(toLogMessage);
			}
		}
	}
}

void ComponentScript::OnTriggerStay(Collision& collision)
{
	ResourceScript* scriptRes = (ResourceScript*)App->res->GetResource(scriptResUUID);
	if (scriptRes && scriptRes->OnTriggerStayMethod)
	{
		MonoObject* exc = nullptr;
		if (IsTreeActive())
		{
			MonoObject* collider = App->scripting->MonoComponentFrom((Component*)collision.GetCollider());

			void* args[1];
			args[0] = collider;

			mono_runtime_invoke(scriptRes->OnTriggerStayMethod, GetMonoComponent(), args, &exc);
			if (exc)
			{
				System_Event event;
				event.type = System_Event_Type::Pause;
				App->PushSystemEvent(event);
				App->Pause();

				MonoString* exceptionMessage = mono_object_to_string(exc, NULL);
				char* toLogMessage = mono_string_to_utf8(exceptionMessage);
				CONSOLE_LOG(LogTypes::Error, toLogMessage);
				mono_free(toLogMessage);
			}
		}
	}
}

void ComponentScript::OnTriggerExit(Collision& collision)
{
	ResourceScript* scriptRes = (ResourceScript*)App->res->GetResource(scriptResUUID);
	if (scriptRes && scriptRes->OnTriggerExitMethod)
	{
		MonoObject* exc = nullptr;
		if (IsTreeActive())
		{
			MonoObject* collider = App->scripting->MonoComponentFrom((Component*)collision.GetCollider());

			void* args[1];
			args[0] = collider;

			mono_runtime_invoke(scriptRes->OnTriggerExitMethod, GetMonoComponent(), args, &exc);
			if (exc)
			{
				System_Event event;
				event.type = System_Event_Type::Pause;
				App->PushSystemEvent(event);
				App->Pause();

				MonoString* exceptionMessage = mono_object_to_string(exc, NULL);
				char* toLogMessage = mono_string_to_utf8(exceptionMessage);
				CONSOLE_LOG(LogTypes::Error, toLogMessage);
				mono_free(toLogMessage);
			}
		}
	}
}

void ComponentScript::OnEnable()
{
	if (App->GetEngineState() == engine_states::ENGINE_PLAY)
		OnEnableMethod();
}

void ComponentScript::OnDisable()
{
	if (App->GetEngineState() == engine_states::ENGINE_PLAY)
		OnDisableMethod();
}

void ComponentScript::OnUniqueEditor()
{
#ifndef GAMEMODE

	//TODO: RECEIVE THOSE EVENTS THOUGH THE PARENT ONEDITOR()

	/*if (ImGui::Checkbox(("###ACTIVE_SCRIPT" + std::to_string(UUID)).data(), &isActive))
	{
	if (isActive)
	{
	if (App->GetEngineState() == engine_states::ENGINE_PLAY)
	this->OnEnableMethod();
	}
	else
	{
	if (App->GetEngineState() == engine_states::ENGINE_PLAY)
	this->OnDisableMethod();
	}
	}
	ImGui::SameLine();*/

	float PosX = ImGui::GetCursorPosX();
	bool opened = ImGui::CollapsingHeader(std::string("##Script" + std::to_string(UUID)).data()); ImGui::SameLine();

	if (ImGui::IsItemClicked(1))
	{
		ImGui::OpenPopup(std::string("##Script" + std::to_string(UUID)).data());
	}

	ImGui::SetNextWindowSize({ 150, 45 });

	ImGuiWindowFlags wflags = 0;
	wflags |= ImGuiWindowFlags_::ImGuiWindowFlags_NoScrollbar;

	bool deleted = false;

	if (ImGui::BeginPopup(std::string("##Script" + std::to_string(UUID)).data(), wflags))
	{
		if (ImGui::MenuItem("Delete Component"))
		{
			System_Event event;
			event.compEvent.type = System_Event_Type::ComponentDestroyed;
			event.compEvent.component = this;
			App->PushSystemEvent(event);

			parent->EraseComponent(this);
			deleted = true;
			App->scripting->DestroyScript(this);
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	if (deleted)
		return;

	ImGuiDragDropFlags flags = 0;
	flags |= ImGuiDragDropFlags_::ImGuiDragDropFlags_SourceNoHoldToOpenOthers;
	if (ImGui::BeginDragDropSource(flags))
	{
		ImGui::BeginTooltip();
		ImGui::Text(std::string(scriptName + " (Script)").data());
		ImGui::EndTooltip();
		ComponentScript* thisOne = (ComponentScript*)this;
		ImGui::SetDragDropPayload("DraggingComponents", &thisOne, sizeof(ComponentScript));
		ImGui::EndDragDropSource();
	}

	ImGui::SetCursorPosX(PosX + 20);
	ImGui::Text(std::string(scriptName + " (Script)").data());
	if (opened)
	{
		ImGui::NewLine();
		ImGui::Text(".cs File:  "); ImGui::SameLine();

		ImVec2 drawingPos = ImGui::GetCursorScreenPos();
		drawingPos = { drawingPos.x - 10, drawingPos.y };
		ImGui::SetCursorScreenPos(drawingPos);

		uint buttonWidth = 2 * ImGui::GetWindowWidth() / 3;
		ImGui::ButtonEx("##csFile", { (float)buttonWidth, 15 }, ImGuiButtonFlags_::ImGuiButtonFlags_Disabled);

		if (ImGui::IsItemHovered())
		{
			ImDrawList* drawList = ImGui::GetWindowDrawList();
			drawList->AddRectFilled(drawingPos, { drawingPos.x + buttonWidth, drawingPos.y + 15 }, ImGui::GetColorU32(ImGuiCol_::ImGuiCol_ButtonHovered));
		}

		if (ImGui::IsItemClicked(0))
		{
			ImDrawList* drawList = ImGui::GetWindowDrawList();
			drawList->AddRectFilled(drawingPos, { drawingPos.x + buttonWidth, drawingPos.y + 15 }, ImGui::GetColorU32(ImGuiCol_::ImGuiCol_ButtonActive));
		}

		ResourceScript* scriptRes = (ResourceScript*)App->res->GetResource(scriptResUUID);
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::Text("\"%s\"\n\nThe .cs file attached to this script component.\nDo not move the script for now!", scriptRes ? scriptRes->GetFile() : "null");
			ImGui::EndTooltip();
		}

		ImGui::SetCursorScreenPos({ drawingPos.x + 7, drawingPos.y });

		//Calculate the text fitting the button rect
		std::string originalText = scriptRes ? scriptRes->GetFile() : "";
		std::string clampedText;

		ImVec2 textSize = ImGui::CalcTextSize(originalText.data());

		if (textSize.x > buttonWidth - 5)
		{
			uint maxTextLenght = originalText.length() * (buttonWidth - 5) / textSize.x;
			clampedText = originalText.substr(0, maxTextLenght - 5);
			clampedText.append("(...)");
		}
		else
			clampedText = originalText;

		ImGui::Text(clampedText.data());

		ImGui::NewLine();


		if (!GetMonoComponent())
		{
			ImGui::TextColored({ .5,0,0,1 }, "SCRIPT WITH ERRORS, CHECK IT");
			return;
		}

		//Script fields
		void* iterator = 0;
		MonoClassField* field = mono_class_get_fields(mono_object_get_class(GetMonoComponent()), &iterator);

		while (field != nullptr)
		{
			uint32_t flags = mono_field_get_flags(field);
			if (flags & MONO_FIELD_ATTR_PUBLIC && !(flags & MONO_FIELD_ATTR_STATIC))
			{
				//This field is public and not static.
				//Show the field, check the type and adapt the gui to it.

				MonoType* type = mono_field_get_type(field);

				std::string typeName = mono_type_full_name(type);

				std::string fieldName = mono_field_get_name(field);

				if (typeName == "bool")
				{
					bool varState; mono_field_get_value(GetMonoComponent(), field, &varState);

					ImVec2 cursorPos = ImGui::GetCursorScreenPos();
					ImGui::SetCursorScreenPos({ cursorPos.x, cursorPos.y + 5 });

					ImGui::Text(fieldName.data()); ImGui::SameLine();

					cursorPos = ImGui::GetCursorScreenPos();
					ImGui::SetCursorScreenPos({ cursorPos.x, cursorPos.y - 5 });

					if (ImGui::Checkbox(("##" + fieldName).data(), &varState))
					{
						mono_field_set_value(GetMonoComponent(), field, &varState);
					}
				}
				else if (typeName == "single") //this is a float, idk
				{
					float varState; mono_field_get_value(GetMonoComponent(), field, &varState);

					ImVec2 cursorPos = ImGui::GetCursorScreenPos();
					ImGui::SetCursorScreenPos({ cursorPos.x, cursorPos.y + 5 });

					ImGui::Text(fieldName.data()); ImGui::SameLine();

					cursorPos = ImGui::GetCursorScreenPos();
					ImGui::SetCursorScreenPos({ cursorPos.x, cursorPos.y - 5 });

					if (ImGui::InputFloat(("##" + fieldName).data(), &varState))
					{
						mono_field_set_value(GetMonoComponent(), field, &varState);
					}
				}
				else if (typeName == "double")
				{
					double varState;
					mono_field_get_value(GetMonoComponent(), field, &varState);
					ImVec2 cursorPos = ImGui::GetCursorScreenPos();
					ImGui::SetCursorScreenPos({ cursorPos.x, cursorPos.y + 5 });

					ImGui::Text(fieldName.data()); ImGui::SameLine();

					cursorPos = ImGui::GetCursorScreenPos();
					ImGui::SetCursorScreenPos({ cursorPos.x, cursorPos.y - 5 });

					if (ImGui::InputDouble(("##" + fieldName).data(), &varState))
					{
						mono_field_set_value(GetMonoComponent(), field, &varState);
					}
				}
				else if (typeName == "System.Decimal")
				{
					//We cant convert System.Decimal, since we do not have this decimal precision in any C++ type.
				}
				else if (typeName == "sbyte")
				{
					int8_t varState;
					mono_field_get_value(GetMonoComponent(), field, &varState);

					ImVec2 cursorPos = ImGui::GetCursorScreenPos();
					ImGui::SetCursorScreenPos({ cursorPos.x, cursorPos.y + 5 });

					ImGui::Text(fieldName.data()); ImGui::SameLine();

					cursorPos = ImGui::GetCursorScreenPos();
					ImGui::SetCursorScreenPos({ cursorPos.x, cursorPos.y - 5 });

					int varState_int = (int)varState;
					if (ImGui::InputScalar(("##" + fieldName).data(), ImGuiDataType_U32, &varState_int))
					{
						varState = varState_int;
						mono_field_set_value(GetMonoComponent(), field, &varState);
					}
				}
				else if (typeName == "byte")
				{
					uint8_t varState;
					mono_field_get_value(GetMonoComponent(), field, &varState);

					ImVec2 cursorPos = ImGui::GetCursorScreenPos();
					ImGui::SetCursorScreenPos({ cursorPos.x, cursorPos.y + 5 });

					ImGui::Text(fieldName.data()); ImGui::SameLine();

					cursorPos = ImGui::GetCursorScreenPos();
					ImGui::SetCursorScreenPos({ cursorPos.x, cursorPos.y - 5 });

					int varState_int = (int)varState;
					if (ImGui::InputScalar(("##" + fieldName).data(), ImGuiDataType_U32, &varState_int))
					{
						varState = varState_int;
						mono_field_set_value(GetMonoComponent(), field, &varState);
					}
				}
				else if (typeName == "int16")
				{
					int16_t varState;
					mono_field_get_value(GetMonoComponent(), field, &varState);

					ImVec2 cursorPos = ImGui::GetCursorScreenPos();
					ImGui::SetCursorScreenPos({ cursorPos.x, cursorPos.y + 5 });

					ImGui::Text(fieldName.data()); ImGui::SameLine();

					cursorPos = ImGui::GetCursorScreenPos();
					ImGui::SetCursorScreenPos({ cursorPos.x, cursorPos.y - 5 });

					int varState_int = (int)varState;
					if (ImGui::InputScalar(("##" + fieldName).data(), ImGuiDataType_S32, &varState_int))
					{
						varState = varState_int;
						mono_field_set_value(GetMonoComponent(), field, &varState);
					}
				}
				else if (typeName == "uint16")
				{
					uint16_t varState;
					mono_field_get_value(GetMonoComponent(), field, &varState);

					ImVec2 cursorPos = ImGui::GetCursorScreenPos();
					ImGui::SetCursorScreenPos({ cursorPos.x, cursorPos.y + 5 });

					ImGui::Text(fieldName.data()); ImGui::SameLine();

					cursorPos = ImGui::GetCursorScreenPos();
					ImGui::SetCursorScreenPos({ cursorPos.x, cursorPos.y - 5 });

					int varState_int = (int)varState;
					if (ImGui::InputScalar(("##" + fieldName).data(), ImGuiDataType_U32, &varState_int))
					{
						varState = varState_int;
						mono_field_set_value(GetMonoComponent(), field, &varState);
					}
				}
				else if (typeName == "int")
				{
					int32_t varState;
					mono_field_get_value(GetMonoComponent(), field, &varState);

					ImVec2 cursorPos = ImGui::GetCursorScreenPos();
					ImGui::SetCursorScreenPos({ cursorPos.x, cursorPos.y + 5 });

					ImGui::Text(fieldName.data()); ImGui::SameLine();

					cursorPos = ImGui::GetCursorScreenPos();
					ImGui::SetCursorScreenPos({ cursorPos.x, cursorPos.y - 5 });

					if (ImGui::InputScalar(("##" + fieldName).data(), ImGuiDataType_S32, &varState))
					{
						mono_field_set_value(GetMonoComponent(), field, &varState);
					}
				}
				else if (typeName == "uint")
				{
					uint32_t varState;
					mono_field_get_value(GetMonoComponent(), field, &varState);

					ImVec2 cursorPos = ImGui::GetCursorScreenPos();
					ImGui::SetCursorScreenPos({ cursorPos.x, cursorPos.y + 5 });

					ImGui::Text(fieldName.data()); ImGui::SameLine();

					cursorPos = ImGui::GetCursorScreenPos();
					ImGui::SetCursorScreenPos({ cursorPos.x, cursorPos.y - 5 });

					if (ImGui::InputScalar(("##" + fieldName).data(), ImGuiDataType_U32, &varState))
					{
						mono_field_set_value(GetMonoComponent(), field, &varState);
					}
				}
				else if (typeName == "long")
				{
					int64_t varState;
					mono_field_get_value(GetMonoComponent(), field, &varState);

					ImVec2 cursorPos = ImGui::GetCursorScreenPos();
					ImGui::SetCursorScreenPos({ cursorPos.x, cursorPos.y + 5 });

					ImGui::Text(fieldName.data()); ImGui::SameLine();

					cursorPos = ImGui::GetCursorScreenPos();
					ImGui::SetCursorScreenPos({ cursorPos.x, cursorPos.y - 5 });

					if (ImGui::InputScalar(("##" + fieldName).data(), ImGuiDataType_S64, &varState))
					{
						mono_field_set_value(GetMonoComponent(), field, &varState);
					}
				}
				else if (typeName == "ulong")
				{
					uint64_t varState;
					mono_field_get_value(GetMonoComponent(), field, &varState);

					ImVec2 cursorPos = ImGui::GetCursorScreenPos();
					ImGui::SetCursorScreenPos({ cursorPos.x, cursorPos.y + 5 });

					ImGui::Text(fieldName.data()); ImGui::SameLine();

					cursorPos = ImGui::GetCursorScreenPos();
					ImGui::SetCursorScreenPos({ cursorPos.x, cursorPos.y - 5 });

					if (ImGui::InputScalar(("##" + fieldName).data(), ImGuiDataType_U64, &varState))
					{
						mono_field_set_value(GetMonoComponent(), field, &varState);
					}
				}
				else if (typeName == "char")
				{
					int temp;
					mono_field_get_value(GetMonoComponent(), field, &temp);

					char varState = (char)temp;

					std::string stringToModify = std::string(1, varState);
					if (ImGui::InputText(mono_field_get_name(field), &stringToModify))
					{
						MonoString* newString = mono_string_new(App->scripting->domain, stringToModify.data());
						temp = (int)stringToModify[0];
						mono_field_set_value(GetMonoComponent(), field, &temp);
					}
				}
				else if (typeName == "string")
				{
					MonoString* varState;
					mono_field_get_value(GetMonoComponent(), field, &varState);

					char* convertedString = mono_string_to_utf8(varState);

					std::string stringToModify = convertedString;

					ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_FrameBg, { 0.26f, 0.59f, 0.98f, 0.5f });

					if (ImGui::InputText(mono_field_get_name(field), &stringToModify))
					{
						MonoString* newString = mono_string_new(App->scripting->domain, stringToModify.data());
						mono_field_set_value(GetMonoComponent(), field, newString);
					}

					ImGui::PopStyleColor();

					mono_free(convertedString);
				}
				else if (typeName == "JellyBitEngine.GameObject")
				{
					uint buttonWidth = 0.65 * ImGui::GetWindowWidth();

					float varState; mono_field_get_value(GetMonoComponent(), field, &varState);

					ImVec2 cursorPos = ImGui::GetCursorScreenPos();
					ImGui::SetCursorScreenPos({ cursorPos.x, cursorPos.y + 5 });

					ImGui::Text(fieldName.data()); ImGui::SameLine();

					cursorPos = ImGui::GetCursorScreenPos();
					ImGui::SetCursorScreenPos({ cursorPos.x, cursorPos.y - 5 });

					cursorPos = { cursorPos.x, cursorPos.y - 5 };

					ImGui::ButtonEx(("##" + fieldName).data(), { (float)buttonWidth, 20 }, ImGuiButtonFlags_::ImGuiButtonFlags_Disabled);

					//Case 1: Dragging Real GameObjects
					if (ImGui::BeginDragDropTarget())
					{
						const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GAMEOBJECTS_HIERARCHY", ImGuiDragDropFlags_AcceptBeforeDelivery | ImGuiDragDropFlags_AcceptNoDrawDefaultRect);
						if (payload)
						{
							GameObject* go = *(GameObject**)payload->Data;

							if (ImGui::IsMouseReleased(0))
							{
								MonoObject* monoObject = App->scripting->MonoObjectFrom(go);
								mono_field_set_value(GetMonoComponent(), field, monoObject);
							}
						}
						ImGui::EndDragDropTarget();
					}

					//Case 2: Dragging Prefabs
					if (ImGui::BeginDragDropTarget())
					{
						const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("PREFAB_RESOURCE", ImGuiDragDropFlags_AcceptBeforeDelivery | ImGuiDragDropFlags_AcceptNoDrawDefaultRect);
						if (payload)
						{
							ResourcePrefab* prefab = *(ResourcePrefab**)payload->Data;

							if (ImGui::IsMouseReleased(0))
							{
								MonoObject* oldObject;
								mono_field_get_value(GetMonoComponent(), field, &oldObject);

								if (oldObject != nullptr)
								{
									GameObject* oldGameObject = App->scripting->GameObjectFrom(oldObject);
									if (oldGameObject)
									{
										if (oldGameObject->prefab)
										{
											App->res->SetAsUnused(oldGameObject->prefab->GetUuid());
										}
									}
								}

								App->res->SetAsUsed(prefab->GetUuid());

								MonoObject* monoObject = App->scripting->MonoObjectFrom(prefab->GetRoot());
								mono_field_set_value(GetMonoComponent(), field, monoObject);
							}
						}
						ImGui::EndDragDropTarget();
					}

					if (ImGui::IsItemClicked(0))
					{
						ImDrawList* drawList = ImGui::GetWindowDrawList();
						drawList->AddRectFilled(cursorPos, { cursorPos.x + buttonWidth, cursorPos.y + 20 }, ImGui::GetColorU32(ImGuiCol_::ImGuiCol_ButtonActive));
					}
					else if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
					{
						ImDrawList* drawList = ImGui::GetWindowDrawList();
						drawList->AddRectFilled(cursorPos, { cursorPos.x + buttonWidth, cursorPos.y + 20 }, ImGui::GetColorU32(ImGuiCol_::ImGuiCol_ButtonHovered));

						if (App->input->GetKey(SDL_SCANCODE_BACKSPACE) == KEY_DOWN)
						{
							MonoObject* oldObject;
							mono_field_get_value(GetMonoComponent(), field, &oldObject);

							if (oldObject != nullptr)
							{
								GameObject* oldGameObject = App->scripting->GameObjectFrom(oldObject);
								if (oldGameObject)
								{
									if (oldGameObject->prefab)
									{
										App->res->SetAsUnused(oldGameObject->prefab->GetUuid());
									}
								}
							}

							mono_field_set_value(GetMonoComponent(), field, NULL);
						}
					}

					//Button text
					MonoObject* monoObject;
					mono_field_get_value(GetMonoComponent(), field, &monoObject);

					std::string text;

					if (monoObject)
					{
						bool destroyed;
						mono_field_get_value(monoObject, mono_class_get_field_from_name(mono_object_get_class(monoObject), "destroyed"), &destroyed);

						if (!destroyed)
						{
							GameObject* gameObject = App->scripting->GameObjectFrom(monoObject);

							if (gameObject->prefab)
								text = gameObject->GetName() + std::string(" (Prefab)");
							else
								text = gameObject->GetName() + std::string(" (GameObject)");
						}
						else
						{
							mono_field_set_value(GetMonoComponent(), field, NULL);
						}
					}
					else
					{
						text = "NULL (GameObject)";
					}

					ImGui::SetCursorScreenPos({ cursorPos.x + 5, cursorPos.y + 3 });

					ImGui::Text(text.data());

					cursorPos = ImGui::GetCursorScreenPos();
					ImGui::SetCursorScreenPos({ cursorPos.x, cursorPos.y + 4 });
				}
				else if (typeName == "JellyBitEngine.Transform")
				{
					uint buttonWidth = 0.65 * ImGui::GetWindowWidth();

					float varState; mono_field_get_value(GetMonoComponent(), field, &varState);

					ImVec2 cursorPos = ImGui::GetCursorScreenPos();
					ImGui::SetCursorScreenPos({ cursorPos.x, cursorPos.y + 5 });

					ImGui::Text(fieldName.data()); ImGui::SameLine();

					cursorPos = ImGui::GetCursorScreenPos();
					ImGui::SetCursorScreenPos({ cursorPos.x, cursorPos.y - 5 });

					cursorPos = { cursorPos.x, cursorPos.y - 5 };

					ImGui::ButtonEx(("##" + fieldName).data(), { (float)buttonWidth, 20 }, ImGuiButtonFlags_::ImGuiButtonFlags_Disabled);

					if (ImGui::BeginDragDropTarget())
					{
						const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GAMEOBJECTS_HIERARCHY", ImGuiDragDropFlags_AcceptBeforeDelivery | ImGuiDragDropFlags_AcceptNoDrawDefaultRect);
						if (payload)
						{
							GameObject* go = *(GameObject**)payload->Data;

							if (ImGui::IsMouseReleased(0))
							{
								MonoObject* monoObject = App->scripting->MonoObjectFrom(go);

								MonoObject* monoTransform;
								mono_field_get_value(monoObject, mono_class_get_field_from_name(mono_object_get_class(monoObject), "transform"), &monoTransform);

								mono_field_set_value(GetMonoComponent(), field, monoTransform);
							}
						}
						ImGui::EndDragDropTarget();
					}

					if (ImGui::IsItemClicked(0))
					{
						ImDrawList* drawList = ImGui::GetWindowDrawList();
						drawList->AddRectFilled(cursorPos, { cursorPos.x + buttonWidth, cursorPos.y + 20 }, ImGui::GetColorU32(ImGuiCol_::ImGuiCol_ButtonActive));
					}
					else if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
					{
						ImDrawList* drawList = ImGui::GetWindowDrawList();
						drawList->AddRectFilled(cursorPos, { cursorPos.x + buttonWidth, cursorPos.y + 20 }, ImGui::GetColorU32(ImGuiCol_::ImGuiCol_ButtonHovered));

						if (App->input->GetKey(SDL_SCANCODE_BACKSPACE) == KEY_DOWN)
						{
							mono_field_set_value(GetMonoComponent(), field, NULL);
						}
					}

					//Button text
					MonoObject* monoTransform;
					mono_field_get_value(GetMonoComponent(), field, &monoTransform);

					MonoObject* monoObject;
					if (monoTransform)
						mono_field_get_value(monoTransform, mono_class_get_field_from_name(mono_object_get_class(monoTransform), "gameObject"), &monoObject);

					std::string text;

					if (monoTransform)
					{
						bool destroyed;
						mono_field_get_value(monoObject, mono_class_get_field_from_name(mono_object_get_class(monoObject), "destroyed"), &destroyed);

						if (!destroyed)
						{
							GameObject* gameObject = App->scripting->GameObjectFrom(monoObject);
							text = gameObject->GetName() + std::string(" (Transform)");
						}
						else
						{
							mono_field_set_value(GetMonoComponent(), field, NULL);
						}
					}
					else
					{
						text = "NULL (Transform)";
					}

					ImGui::SetCursorScreenPos({ cursorPos.x + 5, cursorPos.y + 3 });

					ImGui::Text(text.data());

					cursorPos = ImGui::GetCursorScreenPos();
					ImGui::SetCursorScreenPos({ cursorPos.x, cursorPos.y + 4 });
				}
				else if (typeName == "JellyBitEngine.LayerMask")
				{
					MonoObject* layerMask;
					mono_field_get_value(GetMonoComponent(), field, &layerMask);

					if (layerMask)
					{
						uint32_t bits;
						mono_field_get_value(layerMask, mono_class_get_field_from_name(mono_object_get_class(layerMask), "masks"), &bits);

						std::string enabled;
						uint totalLayers = 0u;
						uint amountEnabled = 0u;

						for (uint i = 0; i < MAX_NUM_LAYERS; ++i)
						{
							const char* layerName = App->layers->NumberToName(i);
							if (strcmp(layerName, "") == 0)
								continue;

							totalLayers++;
							enabled = (bits >> i) & 1U == 1 ? layerName : enabled;
							amountEnabled += (bits >> i) & 1U == 1 ? 1 : 0;
						}

						const char* title = amountEnabled == 0 ? "None" : amountEnabled == 1 ? enabled.data() : totalLayers == amountEnabled ? "Everything" : "Multiple Selected";

						ImGui::PushItemWidth(150.0f);
						if (ImGui::BeginCombo((fieldName + "##" + std::to_string(UUID)).data(), title))
						{
							for (uint i = 0; i < MAX_NUM_LAYERS; ++i)
							{
								const char* layerName = App->layers->NumberToName(i);
								if (strcmp(layerName, "") == 0)
									continue;

								if (ImGui::Selectable(layerName, (bits >> i) & 1U == 1 ? true : false, ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups))
								{
									bits ^= 1UL << i;
									mono_field_set_value(layerMask, mono_class_get_field_from_name(mono_object_get_class(layerMask), "masks"), &bits);
								}
							}
							ImGui::EndCombo();
						}
						ImGui::PopItemWidth();
					}
				}
				else if (mono_class_is_enum(mono_type_get_class(type)))
				{
					//A public enum

					//Get the saved value, as int and as string
					MonoClass* enumClass = mono_get_enum_class();

					int32_t enumValue;
					mono_field_get_value(GetMonoComponent(), field, &enumValue);

					MonoObject* enumOBJ = mono_field_get_value_object(App->scripting->domain, field, GetMonoComponent());

					MonoMethodDesc* ToStringDesc = mono_method_desc_new("Enum::ToString", false);
					MonoMethod* ToStringMethod = mono_method_desc_search_in_class(ToStringDesc, enumClass);
					mono_method_desc_free(ToStringDesc);

					MonoObject* ret = mono_runtime_invoke(ToStringMethod, enumOBJ, NULL, NULL);
					MonoString* monoString = mono_object_to_string(ret, NULL);

					char* string = mono_string_to_utf8(monoString);

					//Create the combo
					if (ImGui::BeginCombo((fieldName + std::string("##") + std::to_string(UUID)).data(), string))
					{
						//ImGui::TextColored({ 1,0,0,1 }, "IN PROGRESS");

						//Get the number of values this enum has

						MonoMethodDesc* GetValuesDesc = mono_method_desc_new("Enum::GetValues", false);
						MonoMethod* GetValuesMethod = mono_method_desc_search_in_class(GetValuesDesc, enumClass);
						mono_method_desc_free(GetValuesDesc);

						MonoMethodDesc* ParseDesc = mono_method_desc_new("Enum::Parse", false);
						MonoMethod* ParseMethod = mono_method_desc_search_in_class(ParseDesc, enumClass);
						mono_method_desc_free(ParseDesc);

						MonoMethodDesc* GetTypeDesc = mono_method_desc_new("object::GetType", false);
						MonoMethod* GetTypeMethod = mono_method_desc_search_in_class(GetTypeDesc, mono_get_object_class());
						mono_method_desc_free(GetTypeDesc);

						MonoObject* enumType = mono_runtime_invoke(GetTypeMethod, enumOBJ, NULL, NULL);

						void* params[1];
						params[0] = enumType;

						MonoArray* values = (MonoArray*)mono_runtime_invoke(GetValuesMethod, NULL, params, NULL);

						uint numValues = mono_array_length(values);

						for (int i = 0; i < numValues; ++i)
						{
							void* nameParams[2];
							nameParams[0] = enumType;
							nameParams[1] = mono_string_new(App->scripting->domain, std::to_string(i).data());

							MonoObject* stringOBJ = mono_runtime_invoke(ParseMethod, NULL, nameParams, NULL);

							MonoString* stringCS = mono_object_to_string(stringOBJ, NULL);

							char* stringcpp = mono_string_to_utf8(stringCS);

							if (ImGui::Selectable(stringcpp))
							{
								mono_field_set_value(GetMonoComponent(), field, &i);
							}

							mono_free(stringcpp);
						}

						ImGui::EndCombo();
					}

					mono_free(string);
				}
			}

			field = mono_class_get_fields(mono_object_get_class(GetMonoComponent()), (void**)&iterator);
		}
		ImGui::NewLine();
	}
#endif
}

uint ComponentScript::GetInternalSerializationBytes()
{
	//My resource uuid + public vars
	return sizeof(uint32_t) + GetPublicVarsSerializationBytes();
}

void ComponentScript::OnInternalSave(char*& cursor)
{
	uint bytes = sizeof(uint32_t);

	uint32_t resUID = scriptResUUID;
	memcpy(cursor, &resUID, bytes);
	cursor += bytes;

	SavePublicVars(cursor);
}

void ComponentScript::OnInternalLoad(char*& cursor)
{
	uint bytes = sizeof(uint32_t);

	memcpy(&scriptResUUID, cursor, bytes);
	cursor += bytes;

	ResourceScript* scriptRes = (ResourceScript*)App->res->GetResource(scriptResUUID);
	if (scriptRes)
	{
		scriptName = scriptRes->scriptName;
		App->res->SetAsUsed(scriptRes->GetUuid());
	}
	else
		CONSOLE_LOG(LogTypes::Error, "A ComponentScript lost his ResourceScript reference!");

	InstanceClass();

	tempBufferBytes = ComponentScript::GetPublicVarsSerializationBytesFromBuffer(cursor);
	tempBuffer = new char[tempBufferBytes];
	memcpy(tempBuffer, cursor, tempBufferBytes);

	cursor += tempBufferBytes;
}

uint ComponentScript::GetPublicVarsSerializationBytes() const
{
	uint bytes = sizeof(uint);

	void* iterator = 0;

	if (!GetMonoComponent())
		return 0;

	MonoClassField* field = mono_class_get_fields(mono_object_get_class(GetMonoComponent()), &iterator);

	while (field != nullptr)
	{
		uint32_t flags = mono_field_get_flags(field);
		if (flags & MONO_FIELD_ATTR_PUBLIC && !(flags & MONO_FIELD_ATTR_STATIC))
		{
			MonoType* type = mono_field_get_type(field);
			std::string typeName = mono_type_full_name(type);
			std::string fieldName = mono_field_get_name(field);

			VarType varType;

			if (typeName == "bool")
			{
				varType = VarType::BOOL;

				uint nameLenght = fieldName.length();
				bytes += (sizeof(varType) + sizeof(uint) + nameLenght + sizeof(bool));
			}
			else if (typeName == "single")
			{
				varType = VarType::FLOAT;

				uint nameLenght = fieldName.length();
				bytes += (sizeof(varType) + sizeof(uint) + nameLenght + sizeof(float));
			}
			else if (typeName == "double")
			{
				varType = VarType::DOUBLE;

				uint nameLenght = fieldName.length();
				bytes += (sizeof(varType) + sizeof(uint) + nameLenght + sizeof(double));
			}
			else if (typeName == "sbyte")
			{
				varType = VarType::INT8;

				uint nameLenght = fieldName.length();
				bytes += (sizeof(varType) + sizeof(uint) + nameLenght + sizeof(int8_t));
			}
			else if (typeName == "byte")
			{
				varType = VarType::UINT8;

				uint nameLenght = fieldName.length();
				bytes += (sizeof(varType) + sizeof(uint) + nameLenght + sizeof(uint8_t));
			}
			else if (typeName == "int16")
			{
				varType = VarType::INT16;

				uint nameLenght = fieldName.length();
				bytes += (sizeof(varType) + sizeof(uint) + nameLenght + sizeof(int16_t));
			}
			else if (typeName == "uint16")
			{
				varType = VarType::UINT16;

				uint nameLenght = fieldName.length();
				bytes += (sizeof(varType) + sizeof(uint) + nameLenght + sizeof(uint16_t));
			}
			else if (typeName == "int")
			{
				varType = VarType::INT;

				uint nameLenght = fieldName.length();
				bytes += (sizeof(varType) + sizeof(uint) + nameLenght + sizeof(int));
			}
			else if (typeName == "uint")
			{
				varType = VarType::UINT;

				uint nameLenght = fieldName.length();
				bytes += (sizeof(varType) + sizeof(uint) + nameLenght + sizeof(uint));
			}
			else if (typeName == "long")
			{
				varType = VarType::INT64;

				uint nameLenght = fieldName.length();
				bytes += (sizeof(varType) + sizeof(uint) + nameLenght + sizeof(long));
			}
			else if (typeName == "ulong")
			{
				varType = VarType::UINT64;

				uint nameLenght = fieldName.length();
				bytes += (sizeof(varType) + sizeof(uint) + nameLenght + sizeof(uint64_t));
			}
			else if (typeName == "char")
			{
				varType = VarType::CHAR;

				uint nameLenght = fieldName.length();
				bytes += (sizeof(varType) + sizeof(uint) + nameLenght + sizeof(char));
			}
			else if (typeName == "string")
			{
				varType = VarType::STRING;

				uint nameLenght = fieldName.length();

				MonoString* varState; mono_field_get_value(GetMonoComponent(), field, &varState);
				char* cString = mono_string_to_utf8(varState);

				std::string defString(cString);
				uint stringLenght = defString.size();

				bytes += (sizeof(varType) + sizeof(uint) + nameLenght + sizeof(uint) + stringLenght);

				mono_free(cString);
			}
			else if (typeName == "JellyBitEngine.GameObject")
			{
				varType = VarType::GAMEOBJECT;

				uint nameLenght = fieldName.length();
				bytes += (sizeof(varType) + sizeof(uint) + nameLenght + sizeof(uint32_t));
			}
			else if (typeName == "JellyBitEngine.Transform")
			{
				varType = VarType::TRANSFORM;

				uint nameLenght = fieldName.length();
				bytes += (sizeof(varType) + sizeof(uint) + nameLenght + sizeof(uint32_t));
			}
			else if (typeName == "JellyBitEngine.LayerMask")
			{
				varType = VarType::LAYERMASK;

				uint nameLenght = fieldName.length();
				bytes += (sizeof(varType) + sizeof(uint) + nameLenght + sizeof(uint32_t));
			}
			else if (mono_class_is_enum(mono_type_get_class(type)))
			{
				varType = VarType::ENUM;

				uint nameLenght = fieldName.length();
				bytes += (sizeof(varType) + sizeof(uint) + nameLenght + sizeof(uint32_t));
			}
		}
		field = mono_class_get_fields(mono_object_get_class(GetMonoComponent()), (void**)&iterator);
	}
	return bytes;
}

uint ComponentScript::GetPublicVarsSerializationBytesFromBuffer(char* buffer) const
{
	char* cursor = buffer;
	uint totalSize = 0;

	uint numVars = 0;
	uint bytes = sizeof(uint);
	memcpy(&numVars, cursor, bytes);
	totalSize += bytes;
	cursor += bytes;

	for (int i = 0; i < numVars; i++)
	{
		//Load type
		VarType varType;
		uint bytes = sizeof(VarType);
		memcpy(&varType, cursor, bytes);
		totalSize += bytes;
		cursor += bytes;

		//Load lenght + string
		bytes = sizeof(uint);
		uint nameLenght;
		memcpy(&nameLenght, cursor, bytes);
		totalSize += bytes;
		cursor += bytes;

		bytes = nameLenght;
		std::string varName;
		varName.resize(nameLenght);
		memcpy((void*)varName.c_str(), cursor, bytes);
		totalSize += bytes;
		varName.resize(nameLenght);
		cursor += bytes;

		//Load data
		switch (varType)
		{
		case VarType::BOOL:
		{
			bytes = sizeof(bool);
			bool var;
			memcpy(&var, cursor, bytes);
			totalSize += bytes;
			cursor += bytes;

			break;
		}
		case VarType::FLOAT:
		{
			bytes = sizeof(float);
			float var;
			memcpy(&var, cursor, bytes);
			totalSize += bytes;
			cursor += bytes;
			void* iterator = 0;
	
			break;
		}
		case VarType::DOUBLE:
		{
			bytes = sizeof(double);
			bool var;
			memcpy(&var, cursor, bytes);
			totalSize += bytes;
			cursor += bytes;
			
			break;
		}
		case VarType::INT8:
		{
			bytes = sizeof(signed char);
			bool var;
			memcpy(&var, cursor, bytes);
			totalSize += bytes;
			cursor += bytes;
			
			break;
		}
		case VarType::UINT8:
		{
			bytes = sizeof(unsigned char);
			bool var;
			memcpy(&var, cursor, bytes);
			totalSize += bytes;
			cursor += bytes;
		
			break;
		}
		case VarType::INT16:
		{
			bytes = sizeof(short);
			bool var;
			memcpy(&var, cursor, bytes);
			totalSize += bytes;
			cursor += bytes;
			
			break;
		}
		case VarType::UINT16:
		{
			bytes = sizeof(unsigned short);
			bool var;
			memcpy(&var, cursor, bytes);
			totalSize += bytes;
			cursor += bytes;
			
			break;
		}
		case VarType::INT:
		{
			bytes = sizeof(int);
			int32_t var;
			memcpy(&var, cursor, bytes);
			totalSize += bytes;
			cursor += bytes;
			
			break;
		}
		case VarType::UINT:
		{
			bytes = sizeof(uint);
			bool var;
			memcpy(&var, cursor, bytes);
			totalSize += bytes;
			cursor += bytes;
			
			break;
		}
		case VarType::INT64:
		{
			bytes = sizeof(long long);
			bool var;
			memcpy(&var, cursor, bytes);
			totalSize += bytes;
			cursor += bytes;
			
			break;
		}
		case VarType::UINT64:
		{
			bytes = sizeof(unsigned long long);
			bool var;
			memcpy(&var, cursor, bytes);
			totalSize += bytes;
			cursor += bytes;
			
			break;
		}
		case VarType::CHAR:
		{
			bytes = sizeof(char);
			bool var;
			memcpy(&var, cursor, bytes);
			totalSize += bytes;
			cursor += bytes;
			
			break;
		}
		case VarType::STRING:
		{
			bytes = sizeof(uint);
			uint stringLength;
			memcpy(&stringLength, cursor, bytes);
			totalSize += bytes;
			cursor += bytes;

			bytes = stringLength;
			std::string string;
			string.resize(stringLength);
			memcpy((void*)string.c_str(), cursor, bytes);
			totalSize += bytes;
			string.resize(stringLength);
			cursor += bytes;

			break;
		}
		case VarType::GAMEOBJECT:
		{
			bytes = sizeof(uint32_t);
			uint32_t uid;
			memcpy(&uid, cursor, bytes);
			totalSize += bytes;
			cursor += bytes;
		
			break;
		}
		case VarType::TRANSFORM:
		{
			bytes = sizeof(uint32_t);
			uint32_t uid;
			memcpy(&uid, cursor, bytes);
			totalSize += bytes;
			cursor += bytes;

			break;
		}
		case VarType::LAYERMASK:
		{
			//DeSerialize the var value
			bytes = sizeof(uint32_t);
			uint32_t var;
			memcpy(&var, cursor, bytes);
			totalSize += bytes;
			cursor += bytes;

			break;
		}
		case VarType::ENUM:
		{
			//DeSerialize the var value
			bytes = sizeof(uint32_t);
			uint32_t var;
			memcpy(&var, cursor, bytes);
			totalSize += bytes;
			cursor += bytes;
		}
		default:
			break;
		}
	}
	
	return totalSize;
}

void ComponentScript::SavePublicVars(char*& cursor) const
{
	uint numVars = 0;

	void* iterator = 0;

	if (!GetMonoComponent())
		return;

	MonoClassField* field = mono_class_get_fields(mono_object_get_class(GetMonoComponent()), &iterator);
	while (field != nullptr)
	{
		uint32_t flags = mono_field_get_flags(field);
		if (flags & MONO_FIELD_ATTR_PUBLIC && !(flags & MONO_FIELD_ATTR_STATIC))
		{
			MonoType* type = mono_field_get_type(field);
			std::string typeName = mono_type_full_name(type);

			if(	typeName == "bool"							||
				typeName == "single"						||
				typeName == "double"						||
				typeName == "sbyte"							||
				typeName == "byte"							||
				typeName == "int16"							||
				typeName == "uint16"						||
				typeName == "int"							||
				typeName == "uint"							||
				typeName == "long"							||
				typeName == "ulong"							||
				typeName == "char"							||
				typeName == "string"						||
				typeName == "JellyBitEngine.GameObject"		||
				typeName == "JellyBitEngine.Transform"		||
				typeName == "JellyBitEngine.LayerMask"		||
				mono_class_is_enum(mono_type_get_class(type)))

			//Only count the serializable ones
			numVars++;
		}
		field = mono_class_get_fields(mono_object_get_class(GetMonoComponent()), (void**)&iterator);
	}

	uint bytes = sizeof(uint);
	memcpy(cursor, &numVars, bytes);
	cursor += bytes;

	iterator = 0;
	field = mono_class_get_fields(mono_object_get_class(GetMonoComponent()), &iterator);
	while (field != nullptr)
	{
		uint32_t flags = mono_field_get_flags(field);
		if (flags & MONO_FIELD_ATTR_PUBLIC && !(flags & MONO_FIELD_ATTR_STATIC))
		{
			MonoType* type = mono_field_get_type(field);
			std::string typeName = mono_type_full_name(type);
			std::string fieldName = mono_field_get_name(field);

			VarType varType;

			if (typeName == "bool")
			{
				varType = VarType::BOOL;

				//Serialize the varType
				bytes = sizeof(varType);
				memcpy(cursor, &varType, bytes);
				cursor += bytes;

				//Serialize the varName (lenght + string)

				bytes = sizeof(uint);
				uint nameLenght = fieldName.length();
				memcpy(cursor, &nameLenght, bytes);
				cursor += bytes;

				bytes = nameLenght;
				memcpy(cursor, fieldName.c_str(), bytes);
				cursor += bytes;

				//Serialize the var value
				bytes = sizeof(bool);
				bool varState; mono_field_get_value(GetMonoComponent(), field, &varState);
				memcpy(cursor, &varState, bytes);
				cursor += bytes;
			}
			else if (typeName == "single")
			{
				varType = VarType::FLOAT;

				//Serialize the varType
				bytes = sizeof(varType);
				memcpy(cursor, &varType, bytes);
				cursor += bytes;

				//Serialize the varName (lenght + string)

				bytes = sizeof(uint);
				uint nameLenght = fieldName.length();
				memcpy(cursor, &nameLenght, bytes);
				cursor += bytes;

				bytes = nameLenght;
				memcpy(cursor, fieldName.c_str(), bytes);
				cursor += bytes;

				//Serialize the var value
				bytes = sizeof(float);
				float varState; mono_field_get_value(GetMonoComponent(), field, &varState);
				memcpy(cursor, &varState, bytes);
				cursor += bytes;
			}
			else if (typeName == "double")
			{
				varType = VarType::DOUBLE;

				//Serialize the varType
				bytes = sizeof(varType);
				memcpy(cursor, &varType, bytes);
				cursor += bytes;

				//Serialize the varName (lenght + string)

				bytes = sizeof(uint);
				uint nameLenght = fieldName.length();
				memcpy(cursor, &nameLenght, bytes);
				cursor += bytes;

				bytes = nameLenght;
				memcpy(cursor, fieldName.c_str(), bytes);
				cursor += bytes;

				//Serialize the var value
				bytes = sizeof(double);
				double varState; mono_field_get_value(GetMonoComponent(), field, &varState);
				memcpy(cursor, &varState, bytes);
				cursor += bytes;
			}
			else if (typeName == "sbyte")
			{
				varType = VarType::INT8;

				//Serialize the varType
				bytes = sizeof(varType);
				memcpy(cursor, &varType, bytes);
				cursor += bytes;

				//Serialize the varName (lenght + string)

				bytes = sizeof(uint);
				uint nameLenght = fieldName.length();
				memcpy(cursor, &nameLenght, bytes);
				cursor += bytes;

				bytes = nameLenght;
				memcpy(cursor, fieldName.c_str(), bytes);
				cursor += bytes;

				//Serialize the var value
				bytes = sizeof(int8_t);
				int8_t varState; mono_field_get_value(GetMonoComponent(), field, &varState);
				memcpy(cursor, &varState, bytes);
				cursor += bytes;
			}
			else if (typeName == "byte")
			{
				varType = VarType::UINT8;

				//Serialize the varType
				bytes = sizeof(varType);
				memcpy(cursor, &varType, bytes);
				cursor += bytes;

				//Serialize the varName (lenght + string)

				bytes = sizeof(uint);
				uint nameLenght = fieldName.length();
				memcpy(cursor, &nameLenght, bytes);
				cursor += bytes;

				bytes = nameLenght;
				memcpy(cursor, fieldName.c_str(), bytes);
				cursor += bytes;

				//Serialize the var value
				bytes = sizeof(uint8_t);
				uint8_t varState; mono_field_get_value(GetMonoComponent(), field, &varState);
				memcpy(cursor, &varState, bytes);
				cursor += bytes;
			}
			else if (typeName == "int16")
			{
				varType = VarType::INT16;

				//Serialize the varType
				bytes = sizeof(varType);
				memcpy(cursor, &varType, bytes);
				cursor += bytes;

				//Serialize the varName (lenght + string)

				bytes = sizeof(uint);
				uint nameLenght = fieldName.length();
				memcpy(cursor, &nameLenght, bytes);
				cursor += bytes;

				bytes = nameLenght;
				memcpy(cursor, fieldName.c_str(), bytes);
				cursor += bytes;

				//Serialize the var value
				bytes = sizeof(int16_t);
				int16_t varState; mono_field_get_value(GetMonoComponent(), field, &varState);
				memcpy(cursor, &varState, bytes);
				cursor += bytes;
			}
			else if (typeName == "uint16")
			{
				varType = VarType::UINT16;

				//Serialize the varType
				bytes = sizeof(varType);
				memcpy(cursor, &varType, bytes);
				cursor += bytes;

				//Serialize the varName (lenght + string)

				bytes = sizeof(uint);
				uint nameLenght = fieldName.length();
				memcpy(cursor, &nameLenght, bytes);
				cursor += bytes;

				bytes = nameLenght;
				memcpy(cursor, fieldName.c_str(), bytes);
				cursor += bytes;

				//Serialize the var value
				bytes = sizeof(uint16_t);
				uint16_t varState; mono_field_get_value(GetMonoComponent(), field, &varState);
				memcpy(cursor, &varState, bytes);
				cursor += bytes;
			}
			else if (typeName == "int")
			{
				varType = VarType::INT;

				//Serialize the varType
				bytes = sizeof(varType);
				memcpy(cursor, &varType, bytes);
				cursor += bytes;

				//Serialize the varName (lenght + string)

				bytes = sizeof(uint);
				uint nameLenght = fieldName.length();
				memcpy(cursor, &nameLenght, bytes);
				cursor += bytes;

				bytes = nameLenght;
				memcpy(cursor, fieldName.c_str(), bytes);
				cursor += bytes;

				//Serialize the var value
				bytes = sizeof(int32_t);
				int32_t varState; mono_field_get_value(GetMonoComponent(), field, &varState);
				memcpy(cursor, &varState, bytes);
				cursor += bytes;
			}
			else if (typeName == "uint")
			{
				varType = VarType::UINT;

				//Serialize the varType
				bytes = sizeof(varType);
				memcpy(cursor, &varType, bytes);
				cursor += bytes;

				//Serialize the varName (lenght + string)

				bytes = sizeof(uint);
				uint nameLenght = fieldName.length();
				memcpy(cursor, &nameLenght, bytes);
				cursor += bytes;

				bytes = nameLenght;
				memcpy(cursor, fieldName.c_str(), bytes);
				cursor += bytes;

				//Serialize the var value
				bytes = sizeof(uint32_t);
				uint32_t varState; mono_field_get_value(GetMonoComponent(), field, &varState);
				memcpy(cursor, &varState, bytes);
				cursor += bytes;
			}
			else if (typeName == "long")
			{
				varType = VarType::INT64;

				//Serialize the varType
				bytes = sizeof(varType);
				memcpy(cursor, &varType, bytes);
				cursor += bytes;

				//Serialize the varName (lenght + string)

				bytes = sizeof(uint);
				uint nameLenght = fieldName.length();
				memcpy(cursor, &nameLenght, bytes);
				cursor += bytes;

				bytes = nameLenght;
				memcpy(cursor, fieldName.c_str(), bytes);
				cursor += bytes;

				//Serialize the var value
				bytes = sizeof(int64_t);
				int64_t varState; mono_field_get_value(GetMonoComponent(), field, &varState);
				memcpy(cursor, &varState, bytes);
				cursor += bytes;
			}
			else if (typeName == "ulong")
			{
				varType = VarType::UINT64;

				//Serialize the varType
				bytes = sizeof(varType);
				memcpy(cursor, &varType, bytes);
				cursor += bytes;

				//Serialize the varName (lenght + string)

				bytes = sizeof(uint);
				uint nameLenght = fieldName.length();
				memcpy(cursor, &nameLenght, bytes);
				cursor += bytes;

				bytes = nameLenght;
				memcpy(cursor, fieldName.c_str(), bytes);
				cursor += bytes;

				//Serialize the var value
				bytes = sizeof(uint64_t);
				uint64_t varState; mono_field_get_value(GetMonoComponent(), field, &varState);
				memcpy(cursor, &varState, bytes);
				cursor += bytes;
			}
			else if (typeName == "char")
			{
				varType = VarType::CHAR;

				//Serialize the varType
				bytes = sizeof(varType);
				memcpy(cursor, &varType, bytes);
				cursor += bytes;

				//Serialize the varName (lenght + string)

				bytes = sizeof(uint);
				uint nameLenght = fieldName.length();
				memcpy(cursor, &nameLenght, bytes);
				cursor += bytes;

				bytes = nameLenght;
				memcpy(cursor, fieldName.c_str(), bytes);
				cursor += bytes;

				//Serialize the var value
				bytes = sizeof(char);
				char varState; mono_field_get_value(GetMonoComponent(), field, &varState);
				memcpy(cursor, &varState, bytes);
				cursor += bytes;
			}
			else if (typeName == "string")
			{
				varType = VarType::STRING;

				//Serialize the varType
				bytes = sizeof(varType);
				memcpy(cursor, &varType, bytes);
				cursor += bytes;

				//Serialize the varName (lenght + string)

				bytes = sizeof(uint);
				uint nameLenght = fieldName.length();
				memcpy(cursor, &nameLenght, bytes);
				cursor += bytes;

				bytes = nameLenght;
				memcpy(cursor, fieldName.c_str(), bytes);
				cursor += bytes;

				//Serialize the var value				
				MonoString* varState; mono_field_get_value(GetMonoComponent(), field, &varState);
				char* cString = mono_string_to_utf8(varState);

				std::string defString(cString);

				bytes = sizeof(uint);

				uint stringLenght = defString.size();
				memcpy(cursor, &stringLenght, bytes);
				cursor += bytes;

				bytes = stringLenght;
				memcpy(cursor, defString.c_str(), bytes);
				cursor += bytes;

				mono_free(cString);
			}
			else if (typeName == "JellyBitEngine.GameObject")
			{
				varType = VarType::GAMEOBJECT;

				MonoObject* monoObject; mono_field_get_value(GetMonoComponent(), field, &monoObject);

				GameObject* serializableGO = monoObject ? App->scripting->GameObjectFrom(monoObject) : nullptr;

				//Serialize the varType
				bytes = sizeof(varType);
				memcpy(cursor, &varType, bytes);
				cursor += bytes;

				//Serialize the varName (lenght + string)

				bytes = sizeof(uint);
				uint nameLenght = fieldName.length();
				memcpy(cursor, &nameLenght, bytes);
				cursor += bytes;

				bytes = nameLenght;
				memcpy(cursor, fieldName.c_str(), bytes);
				cursor += bytes;

				//Here save the UID of the gameObject you have referenced
				uint32_t uid = serializableGO ? serializableGO->GetUUID() : 0;
				bytes = sizeof(uint32_t);
				memcpy(cursor, &uid, bytes);
				cursor += bytes;

			}
			else if (typeName == "JellyBitEngine.Transform")
			{
				varType = VarType::TRANSFORM;

				MonoObject* transformObj; mono_field_get_value(GetMonoComponent(), field, &transformObj);

				MonoObject* monoObject;

				transformObj ? mono_field_get_value(transformObj, mono_class_get_field_from_name(mono_object_get_class(transformObj), "gameObject"), &monoObject) : monoObject = nullptr;

				GameObject* serializableGO = monoObject ? App->scripting->GameObjectFrom(monoObject) : nullptr;

				//Serialize the varType
				bytes = sizeof(varType);
				memcpy(cursor, &varType, bytes);
				cursor += bytes;

				//Serialize the varName (lenght + string)

				bytes = sizeof(uint);
				uint nameLenght = fieldName.length();
				memcpy(cursor, &nameLenght, bytes);
				cursor += bytes;

				bytes = nameLenght;
				memcpy(cursor, fieldName.c_str(), bytes);
				cursor += bytes;

				//Here save the UID of the transform->gameObject you have referenced
				uint32_t uid = serializableGO ? serializableGO->GetUUID() : 0;
				bytes = sizeof(uint32_t);
				memcpy(cursor, &uid, bytes);
				cursor += bytes;
			}
			else if (typeName == "JellyBitEngine.LayerMask")
			{
				varType = VarType::LAYERMASK;

				//Serialize the varType
				bytes = sizeof(varType);
				memcpy(cursor, &varType, bytes);
				cursor += bytes;

				//Serialize the varName (length + string)

				bytes = sizeof(uint);
				uint nameLenght = fieldName.length();
				memcpy(cursor, &nameLenght, bytes);
				cursor += bytes;

				bytes = nameLenght;
				memcpy(cursor, fieldName.c_str(), bytes);
				cursor += bytes;

				//Serialize the var value

				MonoObject* layerMask;
				mono_field_get_value(GetMonoComponent(), field, &layerMask);

				uint32_t varState = 0;
				mono_field_get_value(layerMask, mono_class_get_field_from_name(mono_object_get_class(layerMask), "masks"), &varState);

				bytes = sizeof(uint32_t);
				memcpy(cursor, &varState, bytes);
				cursor += bytes;
			}
			else if (mono_class_is_enum(mono_type_get_class(type)))
			{
				varType = VarType::ENUM;

				//Serialize the varType
				bytes = sizeof(varType);
				memcpy(cursor, &varType, bytes);
				cursor += bytes;

				//Serialize the varName (length + string)

				bytes = sizeof(uint);
				uint nameLenght = fieldName.length();
				memcpy(cursor, &nameLenght, bytes);
				cursor += bytes;

				bytes = nameLenght;
				memcpy(cursor, fieldName.c_str(), bytes);
				cursor += bytes;

				//Serialize the var value

				int32_t value;
				mono_field_get_value(GetMonoComponent(), field, &value);

				bytes = sizeof(uint32_t);
				memcpy(cursor, &value, bytes);
				cursor += bytes;
			}
		}
		field = mono_class_get_fields(mono_object_get_class(GetMonoComponent()), (void**)&iterator);
	}
}

void ComponentScript::LoadPublicVars(char*& buffer)
{
	char* cursor = buffer;

	if (!GetMonoComponent())
		return;

	uint numVars = 0;
	uint bytes = sizeof(uint);
	memcpy(&numVars, cursor, bytes);
	cursor += bytes;

	for (int i = 0; i < numVars; i++)
	{
		//Load type
		VarType varType;
		uint bytes = sizeof(VarType);
		memcpy(&varType, cursor, bytes);
		cursor += bytes;

		//Load lenght + string
		bytes = sizeof(uint);
		uint nameLenght;
		memcpy(&nameLenght, cursor, bytes);
		cursor += bytes;

		bytes = nameLenght;
		std::string varName;
		varName.resize(nameLenght);
		memcpy((void*)varName.c_str(), cursor, bytes);
		varName.resize(nameLenght);
		cursor += bytes;

		//Load data
		switch (varType)
		{
		case VarType::BOOL:
		{
			bytes = sizeof(bool);
			bool var;
			memcpy(&var, cursor, bytes);
			cursor += bytes;

			void* iterator = 0;
			MonoClassField* field = mono_class_get_fields(mono_object_get_class(GetMonoComponent()), &iterator);

			while (field != nullptr)
			{
				uint32_t flags = mono_field_get_flags(field);
				if (flags & MONO_FIELD_ATTR_PUBLIC && !(flags & MONO_FIELD_ATTR_STATIC))
				{
					MonoType* type = mono_field_get_type(field);
					std::string typeName = mono_type_full_name(type);
					std::string fieldName = mono_field_get_name(field);

					if (typeName == "bool" && fieldName == varName)
					{
						mono_field_set_value(GetMonoComponent(), field, &var);
						break;
					}
				}
				field = mono_class_get_fields(mono_object_get_class(GetMonoComponent()), (void**)&iterator);
			}
			break;
		}
		case VarType::FLOAT:
		{
			bytes = sizeof(float);
			float var;
			memcpy(&var, cursor, bytes);
			cursor += bytes;
			void* iterator = 0;
			MonoClassField* field = mono_class_get_fields(mono_object_get_class(GetMonoComponent()), &iterator);

			while (field != nullptr)
			{
				uint32_t flags = mono_field_get_flags(field);
				if (flags & MONO_FIELD_ATTR_PUBLIC && !(flags & MONO_FIELD_ATTR_STATIC))
				{
					MonoType* type = mono_field_get_type(field);
					std::string typeName = mono_type_full_name(type);
					std::string fieldName = mono_field_get_name(field);

					if (typeName == "single" && fieldName == varName)
					{
						mono_field_set_value(GetMonoComponent(), field, &var);
						break;
					}
				}
				field = mono_class_get_fields(mono_object_get_class(GetMonoComponent()), (void**)&iterator);
			}
			break;
		}
		case VarType::DOUBLE:
		{
			bytes = sizeof(double);
			double var;
			memcpy(&var, cursor, bytes);
			cursor += bytes;
			void* iterator = 0;
			MonoClassField* field = mono_class_get_fields(mono_object_get_class(GetMonoComponent()), &iterator);

			while (field != nullptr)
			{
				uint32_t flags = mono_field_get_flags(field);
				if (flags & MONO_FIELD_ATTR_PUBLIC && !(flags & MONO_FIELD_ATTR_STATIC))
				{
					MonoType* type = mono_field_get_type(field);
					std::string typeName = mono_type_full_name(type);
					std::string fieldName = mono_field_get_name(field);

					if (typeName == "double" && fieldName == varName)
					{
						mono_field_set_value(GetMonoComponent(), field, &var);
						break;
					}
				}
				field = mono_class_get_fields(mono_object_get_class(GetMonoComponent()), (void**)&iterator);
			}
			break;
		}
		case VarType::INT8:
		{
			bytes = sizeof(int8_t);
			int8_t var;
			memcpy(&var, cursor, bytes);
			cursor += bytes;
			void* iterator = 0;
			MonoClassField* field = mono_class_get_fields(mono_object_get_class(GetMonoComponent()), &iterator);

			while (field != nullptr)
			{
				uint32_t flags = mono_field_get_flags(field);
				if (flags & MONO_FIELD_ATTR_PUBLIC && !(flags & MONO_FIELD_ATTR_STATIC))
				{
					MonoType* type = mono_field_get_type(field);
					std::string typeName = mono_type_full_name(type);
					std::string fieldName = mono_field_get_name(field);

					if (typeName == "sbyte" && fieldName == varName)
					{
						mono_field_set_value(GetMonoComponent(), field, &var);
						break;
					}
				}
				field = mono_class_get_fields(mono_object_get_class(GetMonoComponent()), (void**)&iterator);
			}
			break;
		}
		case VarType::UINT8:
		{
			bytes = sizeof(uint8_t);
			uint8_t var;
			memcpy(&var, cursor, bytes);
			cursor += bytes;
			void* iterator = 0;
			MonoClassField* field = mono_class_get_fields(mono_object_get_class(GetMonoComponent()), &iterator);

			while (field != nullptr)
			{
				uint32_t flags = mono_field_get_flags(field);
				if (flags & MONO_FIELD_ATTR_PUBLIC && !(flags & MONO_FIELD_ATTR_STATIC))
				{
					MonoType* type = mono_field_get_type(field);
					std::string typeName = mono_type_full_name(type);
					std::string fieldName = mono_field_get_name(field);

					if (typeName == "byte" && fieldName == varName)
					{
						mono_field_set_value(GetMonoComponent(), field, &var);
						break;
					}
				}
				field = mono_class_get_fields(mono_object_get_class(GetMonoComponent()), (void**)&iterator);
			}
			break;
		}
		case VarType::INT16:
		{
			bytes = sizeof(int16_t);
			int16_t var;
			memcpy(&var, cursor, bytes);
			cursor += bytes;
			void* iterator = 0;
			MonoClassField* field = mono_class_get_fields(mono_object_get_class(GetMonoComponent()), &iterator);

			while (field != nullptr)
			{
				uint32_t flags = mono_field_get_flags(field);
				if (flags & MONO_FIELD_ATTR_PUBLIC && !(flags & MONO_FIELD_ATTR_STATIC))
				{
					MonoType* type = mono_field_get_type(field);
					std::string typeName = mono_type_full_name(type);
					std::string fieldName = mono_field_get_name(field);

					if (typeName == "int16" && fieldName == varName)
					{
						mono_field_set_value(GetMonoComponent(), field, &var);
						break;
					}
				}
				field = mono_class_get_fields(mono_object_get_class(GetMonoComponent()), (void**)&iterator);
			}
			break;
		}
		case VarType::UINT16:
		{
			bytes = sizeof(uint16_t);
			uint16_t var;
			memcpy(&var, cursor, bytes);
			cursor += bytes;
			void* iterator = 0;
			MonoClassField* field = mono_class_get_fields(mono_object_get_class(GetMonoComponent()), &iterator);

			while (field != nullptr)
			{
				uint32_t flags = mono_field_get_flags(field);
				if (flags & MONO_FIELD_ATTR_PUBLIC && !(flags & MONO_FIELD_ATTR_STATIC))
				{
					MonoType* type = mono_field_get_type(field);
					std::string typeName = mono_type_full_name(type);
					std::string fieldName = mono_field_get_name(field);

					if (typeName == "uint16" && fieldName == varName)
					{
						mono_field_set_value(GetMonoComponent(), field, &var);
						break;
					}
				}
				field = mono_class_get_fields(mono_object_get_class(GetMonoComponent()), (void**)&iterator);
			}
			break;
		}
		case VarType::INT:
		{
			bytes = sizeof(int32_t);
			int32_t var;
			memcpy(&var, cursor, bytes);
			cursor += bytes;
			void* iterator = 0;
			MonoClassField* field = mono_class_get_fields(mono_object_get_class(GetMonoComponent()), &iterator);

			while (field != nullptr)
			{
				uint32_t flags = mono_field_get_flags(field);
				if (flags & MONO_FIELD_ATTR_PUBLIC && !(flags & MONO_FIELD_ATTR_STATIC))
				{
					MonoType* type = mono_field_get_type(field);
					std::string typeName = mono_type_full_name(type);
					std::string fieldName = mono_field_get_name(field);

					if (typeName == "int" && fieldName == varName)
					{
						mono_field_set_value(GetMonoComponent(), field, &var);
						break;
					}
				}
				field = mono_class_get_fields(mono_object_get_class(GetMonoComponent()), (void**)&iterator);
			}
			break;
		}
		case VarType::UINT:
		{
			bytes = sizeof(uint32_t);
			uint32_t var;
			memcpy(&var, cursor, bytes);
			cursor += bytes;
			void* iterator = 0;
			MonoClassField* field = mono_class_get_fields(mono_object_get_class(GetMonoComponent()), &iterator);

			while (field != nullptr)
			{
				uint32_t flags = mono_field_get_flags(field);
				if (flags & MONO_FIELD_ATTR_PUBLIC && !(flags & MONO_FIELD_ATTR_STATIC))
				{
					MonoType* type = mono_field_get_type(field);
					std::string typeName = mono_type_full_name(type);
					std::string fieldName = mono_field_get_name(field);

					if (typeName == "uint" && fieldName == varName)
					{
						mono_field_set_value(GetMonoComponent(), field, &var);
						break;
					}
				}
				field = mono_class_get_fields(mono_object_get_class(GetMonoComponent()), (void**)&iterator);
			}
			break;
		}
		case VarType::INT64:
		{
			bytes = sizeof(int64_t);
			int64_t var;
			memcpy(&var, cursor, bytes);
			cursor += bytes;
			void* iterator = 0;
			MonoClassField* field = mono_class_get_fields(mono_object_get_class(GetMonoComponent()), &iterator);

			while (field != nullptr)
			{
				uint32_t flags = mono_field_get_flags(field);
				if (flags & MONO_FIELD_ATTR_PUBLIC && !(flags & MONO_FIELD_ATTR_STATIC))
				{
					MonoType* type = mono_field_get_type(field);
					std::string typeName = mono_type_full_name(type);
					std::string fieldName = mono_field_get_name(field);

					if (typeName == "long" && fieldName == varName)
					{
						mono_field_set_value(GetMonoComponent(), field, &var);
						break;
					}
				}
				field = mono_class_get_fields(mono_object_get_class(GetMonoComponent()), (void**)&iterator);
			}
			break;
		}
		case VarType::UINT64:
		{
			bytes = sizeof(uint64_t);
			uint64_t var;
			memcpy(&var, cursor, bytes);
			cursor += bytes;
			void* iterator = 0;
			MonoClassField* field = mono_class_get_fields(mono_object_get_class(GetMonoComponent()), &iterator);

			while (field != nullptr)
			{
				uint32_t flags = mono_field_get_flags(field);
				if (flags & MONO_FIELD_ATTR_PUBLIC && !(flags & MONO_FIELD_ATTR_STATIC))
				{
					MonoType* type = mono_field_get_type(field);
					std::string typeName = mono_type_full_name(type);
					std::string fieldName = mono_field_get_name(field);

					if (typeName == "ulong" && fieldName == varName)
					{
						mono_field_set_value(GetMonoComponent(), field, &var);
						break;
					}
				}
				field = mono_class_get_fields(mono_object_get_class(GetMonoComponent()), (void**)&iterator);
			}
			break;
		}
		case VarType::CHAR:
		{
			bytes = sizeof(char);
			char var;
			memcpy(&var, cursor, bytes);
			cursor += bytes;
			void* iterator = 0;
			MonoClassField* field = mono_class_get_fields(mono_object_get_class(GetMonoComponent()), &iterator);

			while (field != nullptr)
			{
				uint32_t flags = mono_field_get_flags(field);
				if (flags & MONO_FIELD_ATTR_PUBLIC && !(flags & MONO_FIELD_ATTR_STATIC))
				{
					MonoType* type = mono_field_get_type(field);
					std::string typeName = mono_type_full_name(type);
					std::string fieldName = mono_field_get_name(field);

					if (typeName == "char" && fieldName == varName)
					{
						mono_field_set_value(GetMonoComponent(), field, &var);
						break;
					}
				}
				field = mono_class_get_fields(mono_object_get_class(GetMonoComponent()), (void**)&iterator);
			}
			break;
		}
		case VarType::STRING:
		{
			bytes = sizeof(uint);
			uint stringLength;
			memcpy(&stringLength, cursor, bytes);
			cursor += bytes;

			bytes = stringLength;
			std::string string;
			string.resize(stringLength);
			memcpy((void*)string.c_str(), cursor, bytes);
			string.resize(stringLength);		//TODO: Check if names are deSerializing well with this resize
			cursor += bytes;

			void* iterator = 0;
			MonoClassField* field = mono_class_get_fields(mono_object_get_class(GetMonoComponent()), &iterator);

			while (field != nullptr)
			{
				uint32_t flags = mono_field_get_flags(field);
				if (flags & MONO_FIELD_ATTR_PUBLIC && !(flags & MONO_FIELD_ATTR_STATIC))
				{
					MonoType* type = mono_field_get_type(field);
					std::string typeName = mono_type_full_name(type);
					std::string fieldName = mono_field_get_name(field);

					if (typeName == "string" && fieldName == varName)
					{
						MonoString* monoString = mono_string_new(App->scripting->domain, string.c_str());
						mono_field_set_value(GetMonoComponent(), field, monoString);
						break;
					}
				}
				field = mono_class_get_fields(mono_object_get_class(GetMonoComponent()), (void**)&iterator);
			}

			break;
		}
		case VarType::GAMEOBJECT:
		{
			bytes = sizeof(uint32_t);
			uint32_t uid;
			memcpy(&uid, cursor, bytes);
			cursor += bytes;

			GameObject* go = nullptr;

			if (uid != 0)
			{
				ResourcePrefab* prefab = (ResourcePrefab*)App->res->GetResource(uid);
				if (!prefab)
				{
					go = App->GOs->GetGameObjectByUID(uid);
					if (!go)
					{
						CONSOLE_LOG(LogTypes::Error, "A Script lost a Gameobject reference");
					}
				}
				else
				{
					prefab->IncreaseReferences();
					go = prefab->GetRoot();
				}
			}

			MonoObject* monoObject = go ? App->scripting->MonoObjectFrom(go) : nullptr;

			void* iterator = 0;
			MonoClassField* field = mono_class_get_fields(mono_object_get_class(GetMonoComponent()), &iterator);

			while (field != nullptr)
			{
				uint32_t flags = mono_field_get_flags(field);
				if (flags & MONO_FIELD_ATTR_PUBLIC && !(flags & MONO_FIELD_ATTR_STATIC))
				{
					MonoType* type = mono_field_get_type(field);
					std::string typeName = mono_type_full_name(type);
					std::string fieldName = mono_field_get_name(field);

					if (typeName == "JellyBitEngine.GameObject" && fieldName == varName)
					{
						mono_field_set_value(GetMonoComponent(), field, monoObject);
						break;
					}
				}
				field = mono_class_get_fields(mono_object_get_class(GetMonoComponent()), (void**)&iterator);
			}

			break;
		}
		case VarType::TRANSFORM:
		{
			bytes = sizeof(uint32_t);
			uint32_t uid;
			memcpy(&uid, cursor, bytes);
			cursor += bytes;

			GameObject* go = nullptr;

			if (uid != 0)
			{
				ResourcePrefab* prefab = (ResourcePrefab*)App->res->GetResource(uid);
				if (!prefab)
				{
					go = App->GOs->GetGameObjectByUID(uid);
					if (!go)
					{
						CONSOLE_LOG(LogTypes::Error, "A Script lost a Transform reference");
					}
				}
				else
				{
					App->res->SetAsUsed(uid);
					go = prefab->GetRoot();
				}
			}

			MonoObject* monoObject = go ? App->scripting->MonoObjectFrom(go) : nullptr;

			void* iterator = 0;
			MonoClassField* field = mono_class_get_fields(mono_object_get_class(GetMonoComponent()), &iterator);

			while (field != nullptr)
			{
				uint32_t flags = mono_field_get_flags(field);
				if (flags & MONO_FIELD_ATTR_PUBLIC && !(flags & MONO_FIELD_ATTR_STATIC))
				{
					MonoType* type = mono_field_get_type(field);
					std::string typeName = mono_type_full_name(type);
					std::string fieldName = mono_field_get_name(field);

					if (typeName == "JellyBitEngine.Transform" && fieldName == varName)
					{
						if (monoObject)
						{
							MonoObject* monoTransform;
							mono_field_get_value(monoObject, mono_class_get_field_from_name(mono_object_get_class(monoObject), "transform"), &monoTransform);
							mono_field_set_value(GetMonoComponent(), field, monoTransform);
						}
						else
						{
							mono_field_set_value(GetMonoComponent(), field, nullptr);
						}
						break;
					}
				}
				field = mono_class_get_fields(mono_object_get_class(GetMonoComponent()), (void**)&iterator);
			}

			break;
		}
		case VarType::LAYERMASK:
		{
			//DeSerialize the var value
			bytes = sizeof(uint32_t);
			uint32_t var;
			memcpy(&var, cursor, bytes);
			cursor += bytes;

			void* iterator = 0;
			MonoClassField* field = mono_class_get_fields(mono_object_get_class(GetMonoComponent()), &iterator);

			while (field != nullptr)
			{
				uint32_t flags = mono_field_get_flags(field);
				if (flags & MONO_FIELD_ATTR_PUBLIC && !(flags & MONO_FIELD_ATTR_STATIC))
				{
					MonoType* type = mono_field_get_type(field);
					std::string typeName = mono_type_full_name(type);
					std::string fieldName = mono_field_get_name(field);

					if (typeName == "JellyBitEngine.LayerMask" && fieldName == varName)
					{
						MonoObject* layerMask;
						mono_field_get_value(GetMonoComponent(), field, &layerMask);

						mono_field_set_value(layerMask, mono_class_get_field_from_name(mono_object_get_class(layerMask), "masks"), &var);
						break;
					}
				}
				field = mono_class_get_fields(mono_object_get_class(GetMonoComponent()), (void**)&iterator);
			}
			break;
		}
		case VarType::ENUM:
		{
			//DeSerialize the var value
			bytes = sizeof(uint32_t);
			uint32_t var;
			memcpy(&var, cursor, bytes);
			cursor += bytes;

			void* iterator = 0;
			MonoClassField* field = mono_class_get_fields(mono_object_get_class(GetMonoComponent()), &iterator);

			while (field != nullptr)
			{
				uint32_t flags = mono_field_get_flags(field);
				if (flags & MONO_FIELD_ATTR_PUBLIC && !(flags & MONO_FIELD_ATTR_STATIC))
				{
					MonoType* type = mono_field_get_type(field);
					if (mono_class_is_enum(mono_type_get_class(type)))
					{	
						std::string typeName = mono_type_full_name(type);
						std::string fieldName = mono_field_get_name(field);

						if (fieldName == varName)
						{						
							mono_field_set_value(GetMonoComponent(), field, &var);
							break;
						}
					}				
				}
				field = mono_class_get_fields(mono_object_get_class(GetMonoComponent()), (void**)&iterator);
			}
			break;
		}
		default:
			break;
		}
	}
}

void ComponentScript::InstanceClass()
{
	if (scriptResUUID == 0)
		return;

	ResourceScript* scriptRes = (ResourceScript*)App->res->GetResource(scriptResUUID);

	if (!scriptRes)
	{
		System_Event event;
		event.compEvent.type = System_Event_Type::ComponentDestroyed;
		event.compEvent.component = this;
		App->PushSystemEvent(event);
		
		return;
	}

	if (!App->scripting->scriptsImage)
		return;

	MonoClass* klass = mono_class_from_name(App->scripting->scriptsImage, "", scriptName.data());

	if (!klass)
		return;

	MonoObject* compInstance = mono_object_new(App->scripting->domain, klass);
	mono_runtime_object_init(compInstance);

	int gameObjectAddress = (int)GetParent();
	int componentAddress = (int)this;

	mono_field_set_value(compInstance, mono_class_get_field_from_name(klass, "gameObjectAddress"), &gameObjectAddress);
	mono_field_set_value(compInstance, mono_class_get_field_from_name(klass, "componentAddress"), &componentAddress);
	mono_field_set_value(compInstance, mono_class_get_field_from_name(klass, "gameObject"), App->scripting->MonoObjectFrom(GetParent()));

	monoCompHandle = mono_gchandle_new(compInstance, true);

	App->scripting->monoComponentHandles.push_back(monoCompHandle);
}

void ComponentScript::InstanceClass(MonoObject* _classInstance)
{
	if (scriptResUUID == 0)
		return;

	ResourceScript* scriptRes = (ResourceScript*)App->res->GetResource(scriptResUUID);

	if (!scriptRes)
	{
		System_Event event;
		event.compEvent.type = System_Event_Type::ComponentDestroyed;
		event.compEvent.component = this;
		App->PushSystemEvent(event);

		return;
	}

	MonoClass* klass = mono_class_from_name(App->scripting->scriptsImage, "", scriptName.data());

	if (!klass)
		return;

	mono_runtime_object_init(_classInstance);

	int gameObjectAddress = (int)GetParent();
	int componentAddress = (int)this;

	mono_field_set_value(_classInstance, mono_class_get_field_from_name(klass, "gameObjectAddress"), &gameObjectAddress);
	mono_field_set_value(_classInstance, mono_class_get_field_from_name(klass, "componentAddress"), &componentAddress);
	mono_field_set_value(_classInstance, mono_class_get_field_from_name(klass, "gameObject"), App->scripting->MonoObjectFrom(GetParent()));

	monoCompHandle = mono_gchandle_new(_classInstance, true);
	App->scripting->monoComponentHandles.push_back(monoCompHandle);
}