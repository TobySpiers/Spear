#include "Core.h"
#include "ServiceLocator.h"
#include "FlowstateManager.h"

#include "TestFlowstate.h"


int main(int argc, char* argv[])
{
	// Initialise Spear engine
	Spear::WindowParams params;
	params.title = "Game Window";
	params.fullscreen = false;
	params.xpos = SDL_WINDOWPOS_CENTERED;
	params.ypos = SDL_WINDOWPOS_CENTERED;
	params.width = 800;
	params.height = 600;
	params.scale = 1;
	Spear::Core::Initialise(params);

	// Setup states
	Spear::FlowstateManager& stateManager = Spear::ServiceLocator::GetFlowstateManager();
	TestFlowstate stateTest;
	stateManager.RegisterState(&stateTest, 0);

	// Entry state
	stateManager.SetInitialState(0);

	// Run game
	Spear::Core::LaunchGameloop();

	// Shutdown
	Spear::Core::Cleanup();

	return 0;
}