#ifndef __COMPONENT_AUDIO_LISTENER_H__
#define __COMPONENT_AUDIO_LISTENER_H__

#include "Component.h"
#include "WwiseT.h"
#include "Globals.h"

class ComponentAudioListener : public Component
{

public:
	ComponentAudioListener(GameObject* parent, bool includeComponents);
	ComponentAudioListener(const ComponentAudioListener& componentAudioListener, GameObject* parent, bool includeComponents);
	~ComponentAudioListener();
	void Update();

	void UpdateListenerPos();
	void OnUniqueEditor();

	//Serialization
	uint GetInternalSerializationBytes();
	virtual void OnInternalSave(char*& cursor);
	virtual void OnInternalLoad(char*& cursor);

public:
	WwiseT::AudioSource* listener;
};

#endif // !__COMPONENT_AUDIO_LISTENER_H__