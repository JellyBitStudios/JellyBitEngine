#include "ResourceMaterial.h"
#include "ModuleInterpolation.h"

#include "Brofiler/Brofiler.h"


ModuleInterpolation::ModuleInterpolation(bool start_enabled) : Module(start_enabled)
{}

ModuleInterpolation::~ModuleInterpolation()
{
	interpolations.clear();
}

update_status ModuleInterpolation::Update()
{
#ifndef GAMEMODE
	BROFILER_CATEGORY(__FUNCTION__, Profiler::Color::PapayaWhip);
#endif // !GAMEMODE

	for (std::list<ComponentInterpolation*>::iterator inter = interpolations.begin(); inter != interpolations.end(); ++inter)
	{
		(*inter)->Update();
	}

	return UPDATE_CONTINUE;
}


void ModuleInterpolation::OnSystemEvent(System_Event event)
{
	switch (event.type)
	{
		case System_Event_Type::Play:
		case System_Event_Type::LoadFinished:
			// Todo
			break;
		case System_Event_Type::Stop:
			// Todo
			break;
	}
}
