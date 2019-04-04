#include "ModuleTrails.h"

#include "ModuleTimeManager.h"
#include "ModuleInput.h"

#include "Brofiler/Brofiler.h"
#include <algorithm>
#include "MathGeoLib/include/Math/float4x4.h"
#include "Application.h"
#include "DebugDrawer.h"
#include "ComponentTransform.h"

#include "glew/include/GL/glew.h"

ModuleTrails::ModuleTrails(bool start_enabled) : Module(start_enabled)
{}

ModuleTrails::~ModuleTrails()
{}

update_status ModuleTrails::Update()
{
#ifndef GAMEMODE
	BROFILER_CATEGORY(__FUNCTION__, Profiler::Color::PapayaWhip);
#endif // !GAMEMODE

	
	return UPDATE_CONTINUE;
}

void ModuleTrails::Draw()
{
#ifndef GAMEMODE
	BROFILER_CATEGORY(__FUNCTION__, Profiler::Color::PapayaWhip);
#endif // !GAMEMODE

	for (std::list<ComponentTrail*>::iterator trail = trails.begin; trail != trails.end(); ++trail)
	{
		if ((*trail)->trailVertex.empty())
		{
			glColor3f(1.0f, 1.0f, 1.0f);

			glPushMatrix();
			math::float4x4 trans = math::float4x4::identity;
			glMultMatrixf(trans.Transposed().ptr());

			glBegin(GL_TRIANGLES);

			std::list<TrailNode*>::iterator begin = (*trail)->trailVertex.begin();
			TrailNode* end = (*trail)->trailVertex.back();

			if ((*begin)->originHigh.x > end->originHigh.x)
			{

				for (std::list<TrailNode*>::iterator curr = (*trail)->trailVertex.begin(); curr != (*trail)->trailVertex.end(); ++curr)
				{
					std::list<TrailNode*>::iterator next = curr;
					++next;
					if (next != (*trail)->trailVertex.end())
					{
						glVertex3fv((const GLfloat*)(*curr)->originHigh.ptr());
						glVertex3fv((const GLfloat*)(*curr)->originLow.ptr());
						glVertex3fv((const GLfloat*)(*next)->originHigh.ptr());


						glVertex3fv((const GLfloat*)(*curr)->originLow.ptr());
						glVertex3fv((const GLfloat*)(*next)->originLow.ptr());
						glVertex3fv((const GLfloat*)(*next)->originHigh.ptr());

					}
				}
			}

			else
			{
				for (std::list<TrailNode*>::reverse_iterator curr = (*trail)->trailVertex.rbegin(); curr != (*trail)->trailVertex.rend(); ++curr)
				{
					std::list<TrailNode*>::reverse_iterator next = curr;
					++next;
					if (next != (*trail)->trailVertex.rend())
					{
						glVertex3fv((const GLfloat*)(*curr)->originHigh.ptr());
						glVertex3fv((const GLfloat*)(*curr)->originLow.ptr());
						glVertex3fv((const GLfloat*)(*next)->originHigh.ptr());


						glVertex3fv((const GLfloat*)(*curr)->originLow.ptr());
						glVertex3fv((const GLfloat*)(*next)->originLow.ptr());
						glVertex3fv((const GLfloat*)(*next)->originHigh.ptr());
					}
				}
			}

			glEnd();
			glPopMatrix();

		}
	}
}


void ModuleTrails::DebugDraw() const
{
	// Todo
}



void ModuleTrails::OnSystemEvent(System_Event event)
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

void ModuleTrails::RemoveTrail(ComponentTrail* trail)
{
	trails.remove(trail);
}
