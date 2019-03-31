#include "ComponentTrail.h"

#include "Application.h"
#include "ModuleInput.h"

#include "GameObject.h"

#include "glew/include/GL/glew.h"



ComponentTrail::ComponentTrail(GameObject * parent) : Component(parent, ComponentTypes::TrailComponent)
{
	timer.Start();
}

ComponentTrail::ComponentTrail(const ComponentTrail & componentTransform, GameObject * parent) : Component(parent, ComponentTypes::TrailComponent)
{
	timer.Start();

}

ComponentTrail::~ComponentTrail()
{
}

void ComponentTrail::Update() 
{

	if (timer.Read() > 1000 && create)
	{
		TrailNode* node = new TrailNode();

		node->originHigh = parent->boundingBox.FaceCenterPoint(3);
		node->originLow = parent->boundingBox.FaceCenterPoint(2);

		if (!test.empty())
		{
			//math::Plane plane = math::Plane(test.back()->destHigh, node->originHigh, node->originLow);
			//node->direction = plane.normal;
		}

		test.push_back(node);

		timer.Start();
	}

	if (App->input->GetKey(SDL_SCANCODE_1) == KEY_DOWN)
	{
		test.clear();
	}
	if (App->input->GetKey(SDL_SCANCODE_2) == KEY_DOWN)
	{
		create = !create;
	}
}
