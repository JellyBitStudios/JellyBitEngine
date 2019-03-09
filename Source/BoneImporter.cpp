#include "BoneImporter.h"
#include "Application.h"
#include "ModuleFileSystem.h"
#include "Assimp/include/mesh.h"

#include "ModuleResourceManager.h"
#include "ResourceBone.h"
#include "ComponentBone.h"
#include "GameObject.h"
#include <cstdio>

BoneImporter::BoneImporter()
{
}

BoneImporter::~BoneImporter()
{
}

uint BoneImporter::Import(mutable aiBone* new_bone, mutable uint mesh, mutable std::string& output, mutable GameObject* go) const
{
	bool ret = false;

	if (new_bone == nullptr)
		return ret;

	go->cmp_bone->res = App->GenerateRandomNumber();

	std::string outputFile = std::to_string(go->cmp_bone->res);

	ResourceData data;
	data.name = outputFile;
	data.file = outputFile;

	ResourceBoneData res_data;
	res_data.mesh_uid = mesh;
	res_data.bone_weights_size = new_bone->mNumWeights;
	memcpy(res_data.offsetMatrix.v, &new_bone->mOffsetMatrix.a1, sizeof(float) * 16);

	res_data.bone_weights_indices = new uint[res_data.bone_weights_size];
	res_data.bone_weights = new float[res_data.bone_weights_size];

	for (uint k = 0; k < res_data.bone_weights_size; ++k)
	{
		res_data.bone_weights_indices[k] = new_bone->mWeights[k].mVertexId;
		res_data.bone_weights[k] = new_bone->mWeights[k].mWeight;
	}

	App->res->ExportFile(ResourceTypes::BoneResource, data, &res_data, outputFile);

	/*
	if(SaveBone(data,res_data, outputFile, true))
		DEPRECATED_LOG("Saved bone correctly in path: [%s]", outputFile.c_str())
	else
		DEPRECATED_LOG("Error saving bone in path: [%s]", outputFile.c_str());


	output = outputFile;
	*/
	return go->cmp_bone->res;
}

void BoneImporter::Load(mutable const char * file_path, mutable ResourceData& data, mutable ResourceBoneData& bone_data, mutable uint uid_to_force)
{
	// Reading file
	char* buffer = nullptr;
	App->fs->Load(file_path,&buffer);

	// Checking for errors
	if (buffer == nullptr)
	{
		DEPRECATED_LOG("BoneImporter: Unable to open file...");
		//return false;
	}

	char* cursor = buffer;
	
	
	uint bytes = sizeof(char) * DEFAULT_BUF_SIZE;
	char name[DEFAULT_BUF_SIZE];
	memset(name, 0, sizeof(char) * DEFAULT_BUF_SIZE);
	memcpy(name, cursor, bytes);
	bone_data.name.resize(DEFAULT_BUF_SIZE);
	memcpy((char*)bone_data.name.data(), name, bytes);
	bone_data.name.shrink_to_fit();
	cursor += bytes;

	// Load mesh UID
	bytes = sizeof(uint);
	memcpy(&bone_data.mesh_uid, cursor, bytes);

	// Load offset matrix
	cursor += bytes;
	bytes = sizeof(bone_data.offsetMatrix);
	memcpy(bone_data.offsetMatrix.v, cursor, bytes);

	// Load num_weigths
	cursor += bytes;
	bytes = sizeof(bone_data.bone_weights_size);
	memcpy(&bone_data.bone_weights_size, cursor, bytes);

	// Allocate mem for indices and weights
	bone_data.bone_weights_indices = new uint[bone_data.bone_weights_size];
	bone_data.bone_weights = new float[bone_data.bone_weights_size];

	// Read indices
	cursor += bytes;
	bytes = sizeof(uint) * bone_data.bone_weights_size;
	memcpy(bone_data.bone_weights_indices, cursor, bytes);

	// Read weigths
	cursor += bytes;
	bytes = sizeof(float) * bone_data.bone_weights_size;
	memcpy(bone_data.bone_weights, cursor, bytes);

	RELEASE_ARRAY(buffer);

	//ResourceBone* resource = (ResourceBone*)App->res->CreateResource(ResourceTypes::BoneResource, data, &bone_data, uid_to_force);

}

bool BoneImporter::SaveBone(mutable ResourceData& res_data, mutable ResourceBoneData& bone_data,mutable std::string& outputFile,mutable bool overwrite) const
{
	bool ret = false;

	if (overwrite)
		outputFile = res_data.file;
	else
		outputFile = res_data.name;

	// Format: mesh UID + 16 float matrix + num_weigths uint + indices uint * num_weight + weight float * num_weights
	uint size = sizeof(char) * DEFAULT_BUF_SIZE;
	size += sizeof(bone_data.mesh_uid); // mesh_uid
	size += sizeof(bone_data.offsetMatrix);
	size += sizeof(bone_data.bone_weights_size);
	size += sizeof(uint) * bone_data.bone_weights_size;
	size += sizeof(float) * bone_data.bone_weights_size;

	// allocate mem
	char* data = new char[size];
	char* cursor = data;

	// store mesh UID
	uint bytes = sizeof(char)*DEFAULT_BUF_SIZE;
	char name[DEFAULT_BUF_SIZE];
	memset(name, 0, sizeof(char) * DEFAULT_BUF_SIZE);
	strcpy_s(name, DEFAULT_BUF_SIZE, bone_data.name.data());
	memcpy(cursor, name, bytes);

	cursor += bytes;
	// store mesh UID
	bytes = sizeof(bone_data.mesh_uid);
	memcpy(cursor, &bone_data.mesh_uid, bytes);

	// store offset matrix
	cursor += bytes;
	bytes = sizeof(bone_data.offsetMatrix);
	memcpy(cursor, &bone_data.offsetMatrix.v, bytes);

	// store num_weights
	cursor += bytes;
	bytes = sizeof(bone_data.bone_weights_size);
	memcpy(cursor, &bone_data.bone_weights_size, bytes);

	// store indices
	cursor += bytes;
	bytes = sizeof(uint) * bone_data.bone_weights_size;
	memcpy(cursor, bone_data.bone_weights_indices, bytes);

	// store weights
	cursor += bytes;
	bytes = sizeof(float) * bone_data.bone_weights_size;
	memcpy(cursor, bone_data.bone_weights, bytes);

	if (App->fs->SaveInGame((char*)data, size, FileTypes::BoneFile, outputFile) > 0)
		ret = true;

	// Deleting useless data
	RELEASE_ARRAY(data);

	return ret;
}