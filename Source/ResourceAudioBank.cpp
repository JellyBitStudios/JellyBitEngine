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
	assert(data.file.data() != nullptr);

	// Search for the meta associated to the file
	char metaFile[DEFAULT_BUF_SIZE];
	strcpy_s(metaFile, strlen(data.file.data()) + 1, data.file.data()); // file
	strcat_s(metaFile, strlen(metaFile) + strlen(EXTENSION_META) + 1, EXTENSION_META); // extension

	// 1. Copy meta
	if (App->fs->Exists(metaFile))
	{
		// Read the info of the meta
		char* buffer;
		uint size = App->fs->Load(metaFile, &buffer);
		if (size > 0)
		{
			// Create a new name for the meta

			char newMetaFile[DEFAULT_BUF_SIZE];
			sprintf_s(newMetaFile, "%s/%s%s", DIR_LIBRARY_AUDIO, data.name.data(), EXTENSION_META);

			// Save the new meta (info + new name)
			size = App->fs->Save(newMetaFile, buffer, size);
			if (size > 0)
				delete[] buffer;
		}
	}

	//2 Copy audio bank file
	// Read the info of the meta
	if (App->fs->Exists(data.file.data()))
	{
		char* buffer;
		uint size = App->fs->Load(data.file.data(), &buffer);
		if (size > 0)
		{
			char newFile[DEFAULT_BUF_SIZE];
			sprintf_s(newFile, "%s/%s", DIR_LIBRARY_AUDIO, data.name.data());

			// Save the new file
			size = App->fs->Save(newFile, buffer, size);
			if (size > 0)
				delete[] buffer;
		}
	}

	return true;
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

void ResourceAudioBank::Modified()
{
	App->audio->Stop();

	ClearBank();

	char* buffer;
	uint size = App->fs->Load(data.file.data(), &buffer);
	if (size > 0)
	{
		bankData.bankID = WwiseT::LoadBank(buffer, size);
		bankData.buffer = buffer;
	}
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