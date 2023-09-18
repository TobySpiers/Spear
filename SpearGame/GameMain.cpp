#include "SpearEngine/Core.h"
#include "SpearEngine/ServiceLocator.h"
#include "SpearEngine/FlowstateManager.h"

#include "PlayFlowstate.h"
#include "eFlowstate.h"

int main(int argc, char* argv[])
{
	// Initialise Spear engine
	Spear::WindowParams params;
	params.title = "Game Window";
	params.fullscreen = false;
	params.xpos = SDL_WINDOWPOS_CENTERED;
	params.ypos = SDL_WINDOWPOS_CENTERED;
	params.width = 1600;
	params.height = 800;
	params.scale = 1;
	Spear::Core::Initialise(params);

	// Setup states
	Spear::FlowstateManager& stateManager = Spear::ServiceLocator::GetFlowstateManager();
	PlayFlowstate statePlay;
	stateManager.RegisterState(&statePlay, (u32)eFlowstate::STATE_PLAY);

	// Entry state
	stateManager.SetInitialState((u32)eFlowstate::STATE_PLAY);

	// Run game
	Spear::Core::RunGameloop(60);

	// Shutdown
	Spear::Core::Cleanup();

	return 0;
}