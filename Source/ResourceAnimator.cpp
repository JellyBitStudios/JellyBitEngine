#include "ResourceAnimator.h"
#include "imgui/imgui.h"
#include "ModuleScene.h"
#include "ModuleFileSystem.h"
#include "Application.h"
#include "Optick/include/optick.h"
#include "ModuleTimeManager.h"
#include "Application.h"
#include "ResourceAvatar.h"
#include "ModuleResourceManager.h"

#include <assert.h>

ResourceAnimator::ResourceAnimator(ResourceTypes type, uint uuid, ResourceData data, ResourceAnimatorData animator_data) : Resource(type, uuid, data), animator_data(animator_data)
{}

ResourceAnimator::~ResourceAnimator()
{
}

bool ResourceAnimator::LoadInMemory()
{
	return true;
}

bool ResourceAnimator::UnloadFromMemory()
{
	return true;
}

void ResourceAnimator::OnPanelAssets()
{
#ifndef GAMEMODE

	ImGuiTreeNodeFlags flags = 0;
	flags |= ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_Leaf;

	if (App->scene->selectedObject == this)
		flags |= ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_Selected;

	char id[DEFAULT_BUF_SIZE];
	sprintf(id, "%s##%d", data.name.data(), uuid);

	if (ImGui::TreeNodeEx(id, flags))
		ImGui::TreePop();

	if (ImGui::IsMouseReleased(0) && ImGui::IsItemHovered() /*&& (mouseDelta.x == 0 && mouseDelta.y == 0)*/)
	{
		SELECT(this);
	}

	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
	{
		ImGui::SetDragDropPayload("ANIMATOR_INSPECTOR_SELECTOR", &uuid, sizeof(uint));
		ImGui::EndDragDropSource();
	}
#endif // !GAMEMODE
}

bool ResourceAnimator::ImportFile(const char* file, std::string& name, std::string& outputFile)
{
	assert(file != nullptr);

	// Search for the meta associated to the file
	char metaFile[DEFAULT_BUF_SIZE];
	strcpy_s(metaFile, strlen(file) + 1, file); // file
	strcat_s(metaFile, strlen(metaFile) + strlen(EXTENSION_META) + 1, EXTENSION_META); // extension

	if (App->fs->Exists(metaFile))
	{
		// Read the meta
		uint uuid = 0;
		int64_t lastModTime = 0;
		bool result = ResourceAnimator::ReadMeta(metaFile, lastModTime, uuid, name);
		assert(result);

		// The uuid of the resource would be the entry
		char entry[DEFAULT_BUF_SIZE];
		sprintf_s(entry, "%u", uuid);
		outputFile = entry;
	}

	return true;
}

bool ResourceAnimator::ExportFile(ResourceData& resourceData, ResourceAnimatorData& animData, std::string& outputFile, bool overwrite)
{
	bool ret = false;

	uint nameSize = DEFAULT_BUF_SIZE;
	// Name
	char animator_name[DEFAULT_BUF_SIZE];
	strcpy_s(animator_name, DEFAULT_BUF_SIZE, animData.name.data());

	uint animations_size = animData.animations_uuids.size();
	uint meshes_size = animData.meshes_uuids.size();

	uint size =
		sizeof(uint) +
		sizeof(uint) +
		sizeof(uint) * animations_size +
		sizeof(uint) +
		sizeof(uint) * meshes_size +
		sizeof(uint) +				// name size
		sizeof(char) * nameSize;	// name

	char* buffer = new char[size];
	char* cursor = buffer;

	// 1. Store avatar uuid
	uint bytes = sizeof(uint);
	memcpy(cursor, &animData.avatar_uuid, bytes);

	cursor += bytes;

	// 2. Store animations size

	bytes = sizeof(uint);
	memcpy(cursor, &animations_size, bytes);

	cursor += bytes;

	// 3. Store animations
	for (uint i = 0; i < animations_size; ++i)
	{
		bytes = sizeof(uint);
		memcpy(cursor, &animData.animations_uuids[i], bytes);

		cursor += bytes;
	}

	// 4. Store meshes size

	bytes = sizeof(uint);
	memcpy(cursor, &meshes_size, bytes);

	cursor += bytes;

	// 5. Store Meshes
	for (uint i = 0; i < meshes_size; ++i)
	{
		bytes = sizeof(uint);
		memcpy(cursor, &animData.meshes_uuids[i], bytes);

		cursor += bytes;
	}

	// 2. Store name size
	bytes = sizeof(uint);
	memcpy(cursor, &nameSize, bytes);

	cursor += bytes;

	// 3. Store name
	bytes = sizeof(char) * nameSize;
	memcpy(cursor, &animator_name, bytes);

	//cursor += bytes;

	// --------------------------------------------------

	// Build the path of the file
	if (overwrite)
		outputFile = resourceData.file;
	else
		outputFile = resourceData.name;

	// Save the file
	ret = App->fs->SaveInGame(buffer, size, FileTypes::AnimatorFile, outputFile, overwrite) > 0;

	if (ret)
	{
		CONSOLE_LOG(LogTypes::Normal, "Resource Animator: Successfully saved Animator '%s'", outputFile.data());
	}
	else
		CONSOLE_LOG(LogTypes::Error, "Resource Animator: Could not save Animator '%s'", outputFile.data());

	RELEASE_ARRAY(buffer);

	return ret;
}

uint ResourceAnimator::CreateMeta(const char* file, uint animatorUuid, std::string& name, std::string& outputMetaFile)
{
	assert(file != nullptr);

	uint uuidsSize = 1;
	uint nameSize = DEFAULT_BUF_SIZE;

	// Name
	char animator_name[DEFAULT_BUF_SIZE];
	strcpy_s(animator_name, DEFAULT_BUF_SIZE, name.data());

	// --------------------------------------------------

	uint size =
		sizeof(int64_t) +
		sizeof(uint) +
		sizeof(uint) * uuidsSize +
		sizeof(uint) + // name size
		sizeof(char) * nameSize;

	char* data = new char[size];
	char* cursor = data;

	// 1. Store last modification time
	int64_t lastModTime = App->fs->GetLastModificationTime(file);
	assert(lastModTime > 0);
	uint bytes = sizeof(int64_t);
	memcpy(cursor, &lastModTime, bytes);

	cursor += bytes;

	// 2. Store uuids size
	bytes = sizeof(uint);
	memcpy(cursor, &uuidsSize, bytes);

	cursor += bytes;

	// 3. Store animator uuid
	bytes = sizeof(uint) * uuidsSize;
	memcpy(cursor, &animatorUuid, bytes);

	cursor += bytes;

	// 4. Store animator name size
	bytes = sizeof(uint);
	memcpy(cursor, &nameSize, bytes);

	cursor += bytes;

	// 5. Store animator name
	bytes = sizeof(char) * nameSize;
	memcpy(cursor, animator_name, bytes);

	cursor += bytes;



	// --------------------------------------------------

	// Build the path of the meta file and save it
	outputMetaFile = file;
	outputMetaFile.append(EXTENSION_META);
	uint resultSize = App->fs->Save(outputMetaFile.data(), data, size);
	if (resultSize > 0)
	{
		CONSOLE_LOG(LogTypes::Normal, "Resource Animator: Successfully saved meta '%s'", outputMetaFile.data());
	}
	else
	{
		CONSOLE_LOG(LogTypes::Error, "Resource Animator: Could not save meta '%s'", outputMetaFile.data());
		return 0;
	}

	return lastModTime;
}

bool ResourceAnimator::ReadMeta(const char* metaFile, int64_t& lastModTime, uint& animatorUuid, std::string& name)
{
	assert(metaFile != nullptr);

	char* buffer;
	uint size = App->fs->Load(metaFile, &buffer);

	if (size > 0)
	{
		char* cursor = (char*)buffer;

		// 1. Load last modification time
		uint bytes = sizeof(int64_t);
		memcpy(&lastModTime, cursor, bytes);

		cursor += bytes;

		// 2. Load uuids size
		uint uuidsSize = 0;
		bytes = sizeof(uint);
		memcpy(&uuidsSize, cursor, bytes);
		assert(uuidsSize > 0);

		cursor += bytes;

		// 3. Load animation uuid
		bytes = sizeof(uint) * uuidsSize;
		memcpy(&animatorUuid, cursor, bytes);

		cursor += bytes;

		// 4. Load animation name size
		uint nameSize = 0;
		bytes = sizeof(uint);
		memcpy(&nameSize, cursor, bytes);
		assert(nameSize > 0);

		cursor += bytes;

		// 5. Load animation name
		name.resize(nameSize);
		bytes = sizeof(char) * nameSize;
		memcpy(&name[0], cursor, bytes);

		CONSOLE_LOG(LogTypes::Normal, "Resource Animator: Successfully loaded meta '%s'", metaFile);
		RELEASE_ARRAY(buffer);
	}
	else
	{
		CONSOLE_LOG(LogTypes::Error, "Resource Animator: Could not load meta '%s'", metaFile);
		return false;
	}

	return true;
}

bool ResourceAnimator::LoadFile(const char* file, ResourceAnimatorData& outputAnimationData)
{
	assert(file != nullptr);

	bool ret = false;

	char* buffer;
	uint size = App->fs->Load(file, &buffer);
	if (size > 0)
	{
		char* cursor = (char*)buffer;

		// 1. Load avatar uuid
		uint bytes = sizeof(uint);
		memcpy(&outputAnimationData.avatar_uuid, cursor, bytes);

		cursor += bytes;

		// 2. Load animations size
		uint animations_size = 0u;
		bytes = sizeof(uint);
		memcpy(&animations_size, cursor, bytes);

		cursor += bytes;

		// 3. Load animations
		outputAnimationData.animations_uuids.reserve(animations_size);
		for (uint i = 0; i < animations_size; ++i)
		{
			bytes = sizeof(uint);
			uint anim_uuid = 0u;
			memcpy(&anim_uuid, cursor, bytes);
			outputAnimationData.animations_uuids.push_back(anim_uuid);

			cursor += bytes;
		}
		outputAnimationData.animations_uuids.shrink_to_fit();


		// 2. Load meshes size
		uint meshes_size = 0u;
		bytes = sizeof(uint);
		memcpy(&meshes_size, cursor, bytes);

		cursor += bytes;

		// 3. Load meshes
		outputAnimationData.meshes_uuids.reserve(meshes_size);
		for (uint i = 0; i < meshes_size; ++i)
		{
			bytes = sizeof(uint);
			uint mesh_uuid = 0u;
			memcpy(&mesh_uuid, cursor, bytes);
			outputAnimationData.meshes_uuids.push_back(mesh_uuid);

			cursor += bytes;
		}
		outputAnimationData.meshes_uuids.shrink_to_fit();


		// 2. Load name size
		bytes = sizeof(uint);
		uint nameSize = 0;
		memcpy(&nameSize, cursor, bytes);
		assert(nameSize > 0);

		cursor += bytes;

		// 3. Load name
		if (nameSize > 0) {
			bytes = sizeof(char) * nameSize;
			outputAnimationData.name.resize(nameSize);
			memcpy(&outputAnimationData.name[0], cursor, bytes);
		}


		CONSOLE_LOG(LogTypes::Normal, "Resource Animator: Successfully loaded Animator '%s'", file);
		RELEASE_ARRAY(buffer);
	}
	else
		CONSOLE_LOG(LogTypes::Error, "Resource Animator: Could not load Animator '%s'", file);

	return ret;
}

bool ResourceAnimator::GenerateLibraryFiles() const
{
	assert(data.file.data() != nullptr);

	// Search for the meta associated to the file
	char metaFile[DEFAULT_BUF_SIZE];
	strcpy_s(metaFile, strlen(data.file.data()) + 1, data.file.data()); // file
	strcat_s(metaFile, strlen(metaFile) + strlen(EXTENSION_META) + 1, EXTENSION_META); // extension

	// 1. Copy meta
	if (App->fs->Exists(metaFile))
	{
		std::string outputFile;
		uint size = App->fs->Copy(metaFile, DIR_LIBRARY_ANIMATORS, outputFile);

		if (size > 0)
		{
			// 2. Copy Animator
			outputFile.clear();
			uint size = App->fs->Copy(data.file.data(), DIR_LIBRARY_ANIMATORS, outputFile);

			if (size > 0)
				return true;
		}
	}

	return false;
}

uint ResourceAnimator::SetNameToMeta(const char* metaFile, const std::string& name)
{
	assert(metaFile != nullptr);

	int64_t lastModTime = 0;
	uint materialUuid = 0;
	std::string oldName;
	ReadMeta(metaFile, lastModTime, materialUuid, oldName);

	uint uuidsSize = 1;
	uint nameSize = DEFAULT_BUF_SIZE;

	// Name
	char materialName[DEFAULT_BUF_SIZE];
	strcpy_s(materialName, DEFAULT_BUF_SIZE, name.data());

	uint size =
		sizeof(int64_t) +
		sizeof(uint) +
		sizeof(uint) * uuidsSize +

		sizeof(uint) + // name size
		sizeof(char) * nameSize; // name

	char* data = new char[size];
	char* cursor = data;

	// 1. Store last modification time
	uint bytes = sizeof(int64_t);
	memcpy(cursor, &lastModTime, bytes);

	cursor += bytes;

	// 2. Store uuids size
	bytes = sizeof(uint);
	memcpy(cursor, &uuidsSize, bytes);

	cursor += bytes;

	// 3. Store animator uuid
	bytes = sizeof(uint) * uuidsSize;
	memcpy(cursor, &materialUuid, bytes);

	cursor += bytes;

	// 4. Store animator name size
	bytes = sizeof(uint);
	memcpy(cursor, &nameSize, bytes);

	cursor += bytes;

	// 5. Store animator name
	bytes = sizeof(char) * nameSize;
	memcpy(cursor, materialName, bytes);
	cursor += bytes;

	// --------------------------------------------------

	// Build the path of the meta file and save it
	uint retSize = App->fs->Save(metaFile, data, size);
	if (retSize > 0)
	{
		CONSOLE_LOG(LogTypes::Normal, "Resource Animator: Successfully saved meta '%s'", metaFile);
	}
	else
	{
		CONSOLE_LOG(LogTypes::Error, "Resource Animator: Could not save meta '%s'", metaFile);
		return 0;
	}

	return lastModTime;
}

void ResourceAnimator::InitAnimator()
{
	if (current_anim) {
		current_anim->interpolate = true;
		current_anim->loop = true;
	}
	anim_state = AnimationState::PLAYING;
}

bool ResourceAnimator::Update()
{
#ifndef GAMEMODE
	OPTICK_CATEGORY("ResourceAnimator_Update", Optick::Category::Animation);
#endif // GAMEMODE

	//if (App->GetEngineState() != engine_states::ENGINE_PLAY)
		//return update_status::UPDATE_CONTINUE;
	 
	if (stop_all)
		return update_status::UPDATE_CONTINUE;
	if (current_anim == nullptr)
		return update_status::UPDATE_CONTINUE;

	float dt = App->timeManager->GetDt();
	float predicted_time = dt + current_anim->anim_timer;
	if (predicted_time >= current_anim->duration && current_anim->duration > 0.0f)
	{
		current_anim->finished = true;

		if (!current_anim->loop) 
			anim_state = AnimationState::STOPPED;
		else
			current_anim->anim_timer = 0.0f;
	}
	else 
	{
		current_anim->finished = false;
	}

	switch (anim_state)
	{
	case AnimationState::PLAYING: {
		current_anim->anim_timer += dt * current_anim->anim_speed * animation_speed_mod;
		ResourceAvatar* tmp_avatar = (ResourceAvatar*)App->res->GetResource(this->animator_data.avatar_uuid);
		if (tmp_avatar) {
			tmp_avatar->StepBones(current_anim->animation_uuid, current_anim->anim_timer);
		}
	}
		break;

	case AnimationState::PAUSED:
		break;

	case AnimationState::STOPPED: {
		ResourceAvatar* tmp_avatar = (ResourceAvatar*)App->res->GetResource(this->animator_data.avatar_uuid);
		if (tmp_avatar) {
			tmp_avatar->StepBones(current_anim->animation_uuid, current_anim->anim_timer);
		}
		current_anim->anim_timer = 0.0f;
		PauseAnimation();
	}
		break;

	case AnimationState::BLENDING: {
		last_anim->anim_timer += dt * last_anim->anim_speed * animation_speed_mod;
		current_anim->anim_timer += dt * current_anim->anim_speed * animation_speed_mod;
		blend_timer += dt;
		float blend_percentage = blend_timer / 1.0f;
		
		ResourceAvatar* tmp_avatar = (ResourceAvatar*)App->res->GetResource(this->animator_data.avatar_uuid);
		if (tmp_avatar) {
			tmp_avatar->StepBones(last_anim->animation_uuid, last_anim->anim_timer, blend_percentage);
			tmp_avatar->StepBones(current_anim->animation_uuid, current_anim->anim_timer, blend_percentage);
		}

		if (blend_percentage >= blend_timelapse)
			anim_state = PLAYING;
	}
		break;
	}

	return true;
}

void ResourceAnimator::SetAnimationSpeed(float new_speed)
{
	animation_speed_mod = new_speed;
}

void ResourceAnimator::SetAnimationBlendTime(float new_blend)
{
	blend_timelapse = new_blend;
}

void ResourceAnimator::ClearAnimations() {
	for (uint i = 0u; i < animations.size(); i++)
	{
		Animation* animation = animations[i];
		delete animation;
	}
	animations.clear();
	current_anim = nullptr;
}

void ResourceAnimator::AddAnimationFromAnimationResource(ResourceAnimation * res)
{
	Animation* animation = new Animation();
	animation->name = res->animationData.name;
	animation->animation_uuid = res->GetUuid();

	animation->duration = res->animationData.duration;
	animation->numKeys = res->animationData.numKeys;
	animation->boneKeys = res->animationData.boneKeys;
	animations.push_back(animation);
	current_anim = animations[0];
	current_anim->interpolate = true;
	current_anim->loop = true;
}

float ResourceAnimator::GetCurrentAnimationTime() const
{
	return current_anim->anim_timer;
}

const char * ResourceAnimator::GetAnimationName(int index) const
{
	return animations[index]->name.c_str();
}

uint ResourceAnimator::GetAnimationsNumber() const
{
	return (uint)animations.size();
}

ResourceAnimator::Animation * ResourceAnimator::GetCurrentAnimation() const
{
	return current_anim;
}


void ResourceAnimator::SetCurrentAnimationTime(float time)
{
	current_anim->anim_timer = time;

	ResourceAvatar* tmp_avatar = (ResourceAvatar*)App->res->GetResource(this->animator_data.avatar_uuid);
	if (tmp_avatar) {
		tmp_avatar->StepBones(current_anim->animation_uuid, current_anim->anim_timer);
	}
}

bool ResourceAnimator::SetCurrentAnimation(const char * anim_name)
{
	for (uint i = 0u; i < animations.size(); i++)
	{
		Animation* it_anim = animations[i];
		if (strcmp(it_anim->name.c_str(), anim_name) == 0) {
			anim_state = BLENDING;
			blend_timer = 0.0f;
			last_anim = current_anim;
			current_anim = it_anim;
			SetCurrentAnimationTime(0.0f);
			current_anim->finished = false;
			current_anim->anim_timer = 0.0f;
			return true;
		}
	}

	return false;
}

void ResourceAnimator::PlayAnimation()
{
	anim_state = AnimationState::PLAYING;
	current_anim->anim_timer = 0.0f;
	current_anim->finished = false;
	ResourceAvatar* ava = (ResourceAvatar*)App->res->GetResource(this->animator_data.avatar_uuid);
	ava->SetIsAnimated(true);
}

void ResourceAnimator::PauseAnimation()
{
	anim_state = AnimationState::PAUSED;
	ResourceAvatar* ava = (ResourceAvatar*)App->res->GetResource(this->animator_data.avatar_uuid);
	//ava->SetIsAnimated(false);
}

void ResourceAnimator::StopAnimation()
{
	anim_state = AnimationState::STOPPED;
	ResourceAvatar* ava = (ResourceAvatar*)App->res->GetResource(this->animator_data.avatar_uuid);
	//ava->SetIsAnimated(false);
}

void ResourceAnimator::StepBackwards()
{
	if (current_anim->anim_timer > 0.0f)
	{
		float dt = 0.0f;
		dt = App->GetDt();
#ifdef GAMEMODE
		dt = App->timeManager->GetDt();
#endif // GAMEMODE
		current_anim->anim_timer -= dt * current_anim->anim_speed;

		if (current_anim->anim_timer < 0.0f)
			current_anim->anim_timer = 0.0f;
		else {
			ResourceAvatar* tmp_avatar = (ResourceAvatar*)App->res->GetResource(this->animator_data.avatar_uuid);
			if (tmp_avatar) {
				tmp_avatar->StepBones(current_anim->animation_uuid, current_anim->anim_timer);
			}
		}

		PauseAnimation();
	}
}

void ResourceAnimator::StepForward()
{
	if (current_anim->anim_timer < current_anim->duration)
	{
		float dt = 0.0f;
		dt = App->GetDt();
#ifdef GAMEMODE
		dt = App->timeManager->GetDt();
#endif // GAMEMODE
		current_anim->anim_timer += dt * current_anim->anim_speed;

		if (current_anim->anim_timer > current_anim->duration)
			current_anim->anim_timer = 0.0f;
		else {
			ResourceAvatar* tmp_avatar = (ResourceAvatar*)App->res->GetResource(this->animator_data.avatar_uuid);
			if (tmp_avatar) {
				tmp_avatar->StepBones(current_anim->animation_uuid, current_anim->anim_timer);
			}
		}

		PauseAnimation();
	}
}
