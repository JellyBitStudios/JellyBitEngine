#include "CurrentSelection.h"
#include "Application.h"
#include "ModuleScene.h"
#include "ModuleCameraEditor.h"
#include "GameObject.h"
#include "ComponentTransform.h"
#include "SceneImporter.h"
#include "MaterialImporter.h"
#include "ResourceMesh.h"
#include "PanelInspector.h"
#include "ModuleGui.h"
#include "ModuleUI.h"
#include "Resource.h"

#include <assert.h>

void * CurrentSelection::Get() const
{
	return cur;
}

CurrentSelection::SelectedType CurrentSelection::GetType() const
{
	return type;
}

CurrentSelection & CurrentSelection::operator=(SelectedType newSelection)
{
	assert((newSelection == SelectedType::null || newSelection == SelectedType::scene) && "Invalid operation");
	type = newSelection;
	cur = nullptr;
	return *this;
}

CurrentSelection & CurrentSelection::operator=(int null)
{
	assert(null == NULL && "Invalid operation");
	type = SelectedType::null;
	cur = nullptr;
	return *this;
}

bool CurrentSelection::operator==(const SelectedType rhs)
{
	return type == rhs;
}

bool CurrentSelection::operator==(int null)
{
	assert(null == NULL && "Invalid comparison");
	return cur == nullptr;
}

bool CurrentSelection::operator!=(const SelectedType rhs)
{
	return type != rhs;
}

bool CurrentSelection::operator!=(int null)
{
	assert(null == NULL && "Invalid comparison");
	return cur != nullptr;
}

//-----------// GAMEOBJECTS //----------//
CurrentSelection& CurrentSelection::operator=(GameObject * newSelection)
{
	assert(newSelection != nullptr && "Non valid setter. Set to SelectedType::null instead");
	cur = (void*)newSelection;
	type = SelectedType::gameObject;

	// New game object selected. Update the camera reference
	if (newSelection->transform)
		App->camera->SetReference(newSelection->transform->GetPosition());

	App->scene->multipleSelection.clear();

	return *this;
}

bool CurrentSelection::operator==(const GameObject * rhs)
{
	return cur == rhs;
}

GameObject * CurrentSelection::GetCurrGameObject()
{
	if (type == SelectedType::gameObject)
		return (GameObject*)cur;
	else
		return nullptr;
}
//-----------// GAMEOBJECTS LIST //----------//
CurrentSelection & CurrentSelection::operator=(std::list<GameObject*>* newSelection)
{
	assert(newSelection != nullptr && "Non valid setter. Set to SelectedType::null instead");
	cur = (void*)newSelection;
	type = SelectedType::multipleGO;

	return *this;
}


//-----------// RESOURCES //----------//
CurrentSelection & CurrentSelection::operator=(const Resource * newSelection)
{
	assert(newSelection != nullptr && "Non valid setter. Set to SelectedType::null instead");
	cur = (void*)newSelection;
	type = SelectedType::resource;

	if (newSelection->GetType() == ResourceTypes::TextureResource)
		App->gui->panelInspector->SetTextureImportSettings(((ResourceTexture*)newSelection)->GetSpecificData().textureImportSettings);

	return *this;
}

bool CurrentSelection::operator==(const Resource * rhs)
{
	return cur == rhs;
}


//-----------// MESH IMPORT SETTINGS //----------//
CurrentSelection & CurrentSelection::operator=(ResourceMeshImportSettings & newSelection)
{
	cur = (void*)&newSelection;
	type = SelectedType::meshImportSettings;

	App->gui->panelInspector->SetMeshImportSettings(newSelection);

	return *this;
}


//-----------// FONT IMPORT SETTINGS //----------//
CurrentSelection & CurrentSelection::operator=(FontImportSettings & newSelection)
{
	FontImportSettings* selection = App->gui->panelInspector->SetFontImportSettings(newSelection);

	cur = (void*)selection;
	type = SelectedType::fontImportSettings;

	return *this;
}
