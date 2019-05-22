#include "Globals.h"
#include "Application.h"
#include "ModuleAudio.h"
#include "ComponentAudioSource.h"
#include "ComponentAudioListener.h"
#include "Optick/include/optick.h"

#include "ModuleResourceManager.h"
#include "ResourceTypes.h"
#include "ResourceAudioBank.h"

ModuleAudio::ModuleAudio(bool start_enabled) : Module(start_enabled)
{
	this->name = "Audio";
}

ModuleAudio::~ModuleAudio()
{}

bool ModuleAudio::Start()
{
	// Init wwise
	WwiseT::InitSoundEngine();
	return true;
}

update_status ModuleAudio::Update(/*float dt*/)
{
#ifdef GAMEMODE
	OPTICK_CATEGORY("ModuleAudio_Update", Optick::Category::Audio);
	if (audioisplayed == false) {
		PlayOnAwake();
		audioisplayed = true;
	}
#endif // GAMEMODE

	//This has been moved to the transform, in order to not be doing this every frame

	return UPDATE_CONTINUE;
}

update_status ModuleAudio::PostUpdate(/*float dt*/)
{
	OPTICK_CATEGORY("ModuleAudio_PostUpdate", Optick::Category::Audio);
	WwiseT::ProcessAudio();
	return UPDATE_CONTINUE;
}

bool ModuleAudio::CleanUp()
{
	audio_sources.clear();
	audio_listeners.clear();
	event_list.clear();

	std::vector<Resource*> banks = App->res->GetResourcesByType(ResourceTypes::AudioBankResource);
	if (!banks.empty())
	{
		ResourceAudioBank* bank = (ResourceAudioBank*)banks[0];
		bank->ClearBank();
	}

	WwiseT::CloseSoundEngine();
	return true;
}

WwiseT::AudioSource * ModuleAudio::CreateSoundEmitter(const char * name)
{
	WwiseT::AudioSource* ret = WwiseT::CreateAudSource(name);
	event_list.push_back(ret);
	return ret;
}

uint ModuleAudio::GetListenerID() const
{
	return listener->GetID();
}

void ModuleAudio::PlayOnAwake() const
{
	std::list<ComponentAudioSource*>::const_iterator iterator;
	for (iterator = App->audio->audio_sources.begin(); iterator != App->audio->audio_sources.end(); ++iterator) 
	{
		if (iterator._Ptr->_Myval->GetPlayOnAwake() == true) 
		{
			iterator._Ptr->_Myval->PlayAudio();
		}
	}
}

void ModuleAudio::Stop() const
{
	WwiseT::StopAllEvents();
}

void ModuleAudio::Pause() const
{
	WwiseT::PauseAll();
}

void ModuleAudio::Resume() const
{
	WwiseT::ResumeAll();
}

void ModuleAudio::SetListener(WwiseT::AudioSource* new_listener)
{
	listener = new_listener;
	WwiseT::SetDefaultListener(new_listener->GetID());
}

void ModuleAudio::OnSystemEvent(System_Event event)
{
	switch (event.type)
	{
	case System_Event_Type::Play:
		PlayOnAwake();
		break;
	case System_Event_Type::Pause:
		Pause();
		break;
	case System_Event_Type::Stop:
	case System_Event_Type::LoadScene:
		Stop();
		break;
	}
}