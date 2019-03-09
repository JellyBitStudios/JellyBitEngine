#include "ResourceAudioBank.h"

#include "Application.h"
#include "ModuleFileSystem.h"
#include "ModuleResourceManager.h"
#include "ModuleAudio.h"

#include <assert.h>

bool ResourceAudioBank::bankLoaded = false;

ResourceAudioBank::ResourceAudioBank(uint uuid, ResourceData data, ResourceAudioBankData audioData) : Resource(ResourceTypes::AudioBankResource, uuid, data)
{
	if (bankLoaded)
		CONSOLE_LOG(LogTypes::Error, "MULTIPLE BANKS LOADED, FIX THAT");

	bankData = audioData;
	bankLoaded = true;
}

ResourceAudioBank::~ResourceAudioBank()
{
}

bool ResourceAudioBank::GenerateLibraryFiles() const
{
	return false;
}

ResourceAudioBank* ResourceAudioBank::ImportFile(const char* file)
{
	std::string extension;
	App->fs->GetExtension(file, extension);

	ResourceTypes type = App->res->GetResourceTypeByExtension(extension.data());

	assert(type == ResourceTypes::AudioBankResource);

	uint bankID = 0u;

	char* buffer;
	uint size = App->fs->Load(file, &buffer);
	if (size > 0)
	{
		bankID = WwiseT::LoadBank(buffer, size);
	}

	if (bankID <= 0)
		return nullptr;

	char metaFile[DEFAULT_BUF_SIZE];
	sprintf(metaFile, "%s%s", file, EXTENSION_META);

	uint forcedUID = 0;

	bool createMeta = false;

	if (App->fs->Exists(metaFile))
	{
		char* metaBuffer;
		uint metaSize = App->fs->Load(metaFile, &metaBuffer);
		if (metaSize > 0)
		{
			char* cursor = metaBuffer;
			cursor += sizeof(int64_t) + sizeof(uint);
			memcpy(&forcedUID, cursor, sizeof(uint));

			delete[] metaBuffer;
		}

	}
	else
	{
		createMeta = true;
	}

	std::string fileName;
	App->fs->GetFileName(file, fileName, true);

	ResourceData data;
	data.file = file;
	data.exportedFile = "";
	data.name = fileName;

	ResourceAudioBankData bankData;
	bankData.bankID = bankID;
	bankData.buffer = buffer;

	ResourceAudioBank* bankRes = (ResourceAudioBank*)App->res->CreateResource(ResourceTypes::AudioBankResource, data, &bankData, forcedUID);
	if(createMeta)
		bankRes->WriteMeta();

	return bankRes;
}

void ResourceAudioBank::ClearBank()
{
	WwiseT::UnLoadBank(bankData.bankID, bankData.buffer);
	delete[] bankData.buffer;
}

bool ResourceAudioBank::WriteMeta() const
{
	char metaFile[DEFAULT_BUF_SIZE];
	sprintf(metaFile, "%s%s", data.file.data(), EXTENSION_META);

	uint size = bytesToMeta();
	char* buffer = new char[size];

	int64_t lastModTime = App->fs->GetLastModificationTime(data.file.data());
	uint bytes = sizeof(int64_t);

	char* cursor = buffer;
	memcpy(cursor, &lastModTime, bytes);
	cursor += bytes;

	bytes = sizeof(uint);
	uint temp = 0u;
	memcpy(cursor, &temp, bytes);
	cursor += bytes;

	memcpy(cursor, &uuid, bytes);
	cursor += bytes;

	memcpy(cursor, &bankData.bankID, bytes);
	cursor += bytes;

	App->fs->Save(metaFile, buffer, size);

	delete[] buffer;

	return true;
}