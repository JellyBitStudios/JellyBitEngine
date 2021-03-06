#include "ComponentAudioListener.h"

#include "Application.h"
#include "ModuleAudio.h"
#include "ComponentTransform.h"
#include "GameObject.h"
#include "MathGeoLib/include/Math/Quat.h"
#include "MathGeoLib/include/Math/float3.h"

#include "imgui\imgui.h"
#include "imgui\imgui_internal.h"

ComponentAudioListener::ComponentAudioListener(GameObject* parent, bool includeComponents) : Component(parent, ComponentTypes::AudioListenerComponent) 
{
	if (includeComponents)
	{
		listener = App->audio->CreateSoundEmitter("listener");
		App->audio->audio_listeners.push_back(this);
		App->audio->SetListener(listener);

		UpdateListenerPos();
	}
}

ComponentAudioListener::ComponentAudioListener(const ComponentAudioListener& componentAudioListener, GameObject* parent, bool includeComponents) : 
	Component(parent, ComponentTypes::AudioListenerComponent)
{
	if (includeComponents)
	{
		listener = App->audio->CreateSoundEmitter("listener");
		App->audio->audio_listeners.push_back(this);
		App->audio->SetListener(listener);

		UpdateListenerPos();
	}
}

ComponentAudioListener::~ComponentAudioListener()
{
	App->audio->audio_listeners.remove(this);
	parent->cmp_audioListener = nullptr;
	//RELEASE(listener);
}

void ComponentAudioListener::Update()
{
}

void ComponentAudioListener::UpdateListenerPos()
{
	if (parent->transform != nullptr && listener)
	{
		math::float3 vector_pos;
		math::float3 vector_front;
		math::float3 vector_up;

		math::Quat rotation;
		math::float3 scale;

		math::float4x4 global = parent->transform->GetGlobalMatrix();

		global.Decompose(vector_pos, rotation, scale);

		vector_front = rotation * math::float3{0,0,1};
		vector_up = rotation * math::float3{ 0,1,0 };

		listener->SetListenerPos(vector_pos.x, vector_pos.y, vector_pos.z, vector_front.x, vector_front.y, vector_front.z, 
			vector_up.x, vector_up.y, vector_up.z);
	}
}

void ComponentAudioListener::OnUniqueEditor()
{
#ifndef GAMEMODE
	if (ImGui::CollapsingHeader("AudioListener", ImGuiTreeNodeFlags_DefaultOpen))
	{	}

#endif // !GAMEMODE

}

uint ComponentAudioListener::GetInternalSerializationBytes()
{
	return sizeof(WwiseT::AudioSource*);
}

void ComponentAudioListener::OnInternalSave(char*& cursor)
{
	cursor += sizeof(WwiseT::AudioSource*);
}

void ComponentAudioListener::OnInternalLoad(char*& cursor)
{
	cursor += sizeof(WwiseT::AudioSource*);

	UpdateListenerPos();
}