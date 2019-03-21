#include "ComponentTrail.h"

#include "Application.h"
#include "ModuleInput.h"

#include "GameObject.h"

#include "glew/include/GL/glew.h"



ComponentTrail::ComponentTrail(GameObject * parent) : Component(parent, ComponentTypes::TrailComponent)
{

}

ComponentTrail::ComponentTrail(const ComponentTrail & componentTransform, GameObject * parent) : Component(parent, ComponentTypes::TrailComponent)
{

}

ComponentTrail::~ComponentTrail()
{
}

void ComponentTrail::Update() 
{
	if (App->input->GetKey(SDL_SCANCODE_1) == KEY_DOWN)
	{
		test.originHigh = parent->boundingBox.FaceCenterPoint(3);
		test.originLow = parent->boundingBox.FaceCenterPoint(2);

		ready1 = true;
	}

	else if (App->input->GetKey(SDL_SCANCODE_2) == KEY_DOWN)
	{
		test.destHigh = parent->boundingBox.FaceCenterPoint(3);
		test.destLow = parent->boundingBox.FaceCenterPoint(2);
		test2.originHigh = test.destHigh;
		test2.originLow = test.destLow;


		ready2 = true;
	}

	if (App->input->GetKey(SDL_SCANCODE_3) == KEY_DOWN)
	{
		test2.destHigh = parent->boundingBox.FaceCenterPoint(3);
		test2.destLow = parent->boundingBox.FaceCenterPoint(2);

		ready1 = true;
	}
	
}
