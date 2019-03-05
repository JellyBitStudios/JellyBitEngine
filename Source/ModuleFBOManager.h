#ifndef __FBOMANAGER
#define __FBOMANAGER

#include "Module.h"

class ModuleFBOManager : public Module
{
public:

	ModuleFBOManager();
	~ModuleFBOManager();
	bool Start();
	bool CleanUp();

	void LoadGBuffer(uint width, uint height);
	void UnloadGBuffer();
	void ResizeGBuffer(uint width, uint height);
	void BindGBuffer();
	void DrawGBufferToScreen() const;
	void MergeDepthBuffer(uint width, uint height);

public:

	uint gBuffer;
	uint gPosition;
	uint gNormal;
	uint gAlbedoSpec;
	uint rboDepth;
};

#endif
