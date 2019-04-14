#ifndef __CURRENT_SELECTION_H__
#define __CURRENT_SELECTION_H__

#ifndef GAMEMODE

#include <list>

class Resource;
class GameObject;
struct ResourceMeshImportSettings;
struct FontImportSettings;

struct CurrentSelection
{
	enum class SelectedType { null, gameObject, scene, resource, meshImportSettings, fontImportSettings, multipleGO };

private:
	void* cur = nullptr;
	SelectedType type = SelectedType::null;

public:
	void* Get() const;

	SelectedType GetType() const;

	CurrentSelection& operator=(SelectedType newSelection);
	CurrentSelection& operator=(int null);
	bool operator==(const SelectedType rhs);
	bool operator==(int null);
	bool operator!=(const SelectedType rhs);
	bool operator!=(int null);


	//-----------// GAMEOBJECTS //----------//
	CurrentSelection& operator=(GameObject* newSelection);
	bool operator==(const GameObject* rhs);
	GameObject* GetCurrGameObject();

	//-----------// GAMEOBJECTS LIST //----------//
	CurrentSelection& operator=(std::list<GameObject*>* newSelection);

	//-----------// RESOURCES //----------//
	CurrentSelection& operator=(const Resource* newSelection);
	bool operator==(const Resource* rhs);


	//Mesh Import Settings ----------------------------------
	CurrentSelection& operator=(ResourceMeshImportSettings& newSelection);

	//Font Import Settings ----------------------------------
	CurrentSelection& operator=(FontImportSettings& newSelection);

	// Add operators in case of new kinds of selection :)
};

#endif
#endif