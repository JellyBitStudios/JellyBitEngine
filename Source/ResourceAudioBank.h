#ifndef __RESOURCEAUDIOBANK_H__
#define __RESOURCEAUDIOBANK_H__

#include "Resource.h"

struct ResourceAudioBankData
{
	uint bankID = 0u;
	char* buffer = nullptr;
};

class ResourceAudioBank : public Resource
{
public:
	ResourceAudioBank(uint uuid, ResourceData data, ResourceAudioBankData audioData);
	~ResourceAudioBank();

	//We will not expose this resource to the user
	void OnPanelAssets() {}

	bool GenerateLibraryFiles() const;

public:
	static ResourceAudioBank* ImportFile(const char* file);
	void ClearBank();

private:
	bool LoadInMemory() { return true; };
	bool UnloadFromMemory() { return true; };
	bool WriteMeta() const;
	inline uint bytesToMeta() const { return sizeof(int64_t) + sizeof(uint) * 3; }

private:
	ResourceAudioBankData bankData;

	static bool bankLoaded;

};



#endif //ResourceAudioBank.h