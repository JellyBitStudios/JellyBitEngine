#ifndef __PANEL_CODE_EDITOR_H__
#define __PANEL_CODE_EDITOR_H__

#ifndef GAMEMODE

#include "Panel.h"
#include "Globals.h"

#include "ImGui\imgui.h"
#include "ImGuiColorTextEdit\TextEditor.h"

class PanelCodeEditor : public Panel
{
public:

	PanelCodeEditor(const char* name);
	~PanelCodeEditor();

	bool Draw();

	void OpenShaderInCodeEditor(uint shaderObjectUuid);
	void SetError(int line, const char* error);

private:

	bool TryCompile();

private:

	uint shaderObjectUuid = 0;

	const char* fileToEdit = "";
	TextEditor editor;
};

#endif

#endif // GAME
