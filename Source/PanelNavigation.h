#ifndef __PANEL_NAVIGATION_H__
#define __PANEL_NAVIGATION_H__



#ifndef GAMEMODE

#include "Panel.h"

struct CommonSettings
{
	// Dont reorder the elements cause of memcpy. If required, you would also reorder buildSettings at InputGeom.

	float p_cellSize;
	float p_cellHeight;
	float p_agentHeight;
	float p_agentRadius;
	float p_agentMaxClimb;
	float p_agentMaxSlope;
	float p_regionMinSize;
	float p_regionMergeSize;
	float p_edgeMaxLen;
	float p_edgeMaxError;
	float p_vertsPerPoly;
	float p_detailSampleDist;
	float p_detailSampleMaxError;
	int	  p_partitionType;
};

class PanelNavigation : public Panel
{
public:

	PanelNavigation(char* name);
	~PanelNavigation();

	bool Draw();

private:
	void ResetCommonSettings();
	void HandleInputMeshes() const;

private:
	CommonSettings cs;
};

#endif

#endif // GAME
