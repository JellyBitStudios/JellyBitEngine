// ----------------------------------------------------
// Timer.cpp
// Body for CPU Tick Timer class
// Class for control timer in game. It's diferent from other time.
// Need list for connect it, remember to pop when you don't wanna use it
// ----------------------------------------------------

#include "GameTimer.h"
#include "Application.h"
#include "ModuleTimeManager.h"
// ---------------------------------------------
GameTimer::GameTimer()
{
	if (App)
		App->timeManager->GetGameTimerList().push_back(this);
}

GameTimer::~GameTimer()
{}

void GameTimer::Update(float dt)
{
	if(running)
		time += dt;
}

// ---------------------------------------------
void GameTimer::Start()
{
	if ((std::find(App->timeManager->GetGameTimerList().begin(), App->timeManager->GetGameTimerList().end(), this) == App->timeManager->GetGameTimerList().end()))
		App->timeManager->GetGameTimerList().push_back(this);

	running = true;
	time = 0;
}
// ---------------------------------------------
void GameTimer::Continue()
{
	running = true;
}

// ---------------------------------------------
void GameTimer::Stop()
{
	running = false;
	time = 0;
}

// ---------------------------------------------
void GameTimer::Pause()
{
	running = false;
}
// ---------------------------------------------
Uint32 GameTimer::Read() const
{
	return time * 1000.0f;
}

float GameTimer::ReadSec() const
{
	return time;
}
