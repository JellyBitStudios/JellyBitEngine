#include "Globals.h"
#include "Application.h"
#include "ModuleAudio.h"
#include "ComponentAudioSource.h"
#include "Brofiler/Brofiler.h"

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
	BROFILER_CATEGORY(__FUNCTION__, Profiler::Color::PapayaWhip);
	if (audioisplayed == false) {
		PlayOnAwake();
		audioisplayed = true;
	}
#endif // GAMEMODE
	return UPDATE_CONTINUE;
}

update_status ModuleAudio::PostUpdate(/*float dt*/)
{
	BROFILER_CATEGORY(__FUNCTION__, Profiler::Color::PapayaWhip);
	WwiseT::ProcessAudio();
	return UPDATE_CONTINUE;
}

bool ModuleAudio::CleanUp()
{
	audio_sources.clear();
	event_list.clear();

	ResourceAudioBank* bank = (ResourceAudioBank*)App->res->GetResourcesByType(ResourceTypes::AudioBankResource).front();
	bank->ClearBank();

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
		WwiseT::PauseAll();
		break;
	case System_Event_Type::Stop:
		WwiseT::StopAllEvents();
		break;
	}
}