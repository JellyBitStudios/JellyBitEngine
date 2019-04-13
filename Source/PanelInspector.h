#ifndef __PANEL_INSPECTOR_H__
#define __PANEL_INSPECTOR_H__

#include "ResourceMesh.h"
#include "ResourceTexture.h"
#include "ResourceFont.h"
#ifndef GAMEMODE

#include "Panel.h"

class Component;
enum ComponentTypes;

class PanelInspector : public Panel
{
public:

	PanelInspector(const char* name);
	~PanelInspector();

	bool Draw();
	
	void SetMeshImportSettings(ResourceMeshImportSettings& is) { m_is = is; }
	void SetTextureImportSettings(ResourceTextureImportSettings& is) { t_is = is; }
	FontImportSettings* SetFontImportSettings(FontImportSettings is) { f_is = is; return &f_is; }

private:

	void ShowGameObjectInspector() const;
	void ShowGameObjectListInspector() const;
	bool CheckIsComponent(ComponentTypes type) const;
	void DragnDropSeparatorTarget(Component* target) const;

	void ShowMeshResourceInspector() const;
	void ShowTextureResourceInspector() const;
	void ShowMeshImportSettingsInspector();
	void ShowFontImportSettingsInspector();
	void ShowTextureImportSettingsInspector() const;
	void ShowShaderObjectInspector() const;
	void ShowShaderProgramInspector() const;
	void ShowMaterialInspector() const;
	void ShowAvatarInspector() const;
	void ShowAnimationInspector() const;

	ResourceMeshImportSettings m_is;
	ResourceTextureImportSettings t_is;
	FontImportSettings f_is;
};

#endif

#endif // GAME
