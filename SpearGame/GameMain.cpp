#include "SpearEngine/Core.h"
#include "SpearEngine/ServiceLocator.h"
#include "SpearEngine/FlowstateManager.h"

#include "FlowstateMenu.h"
#include "FlowstateGame.h"
#include "eFlowstate.h"

int main(int argc, char* argv[])
{
	// Initialise Spear engine
	Spear::WindowParams params;
	params.title = "Spear";
	params.fullscreen = false;
	params.xpos = SDL_WINDOWPOS_CENTERED;
	params.ypos = SDL_WINDOWPOS_CENTERED;
	params.width = 1920;	// windowed resolution
	params.height = 1080;	// windowed resolution
	Spear::Core::Initialise(params);

	// Setup states
	Spear::FlowstateManager& stateManager = Spear::ServiceLocator::GetFlowstateManager();

	FlowstateMenu stateMenu;
	stateManager.RegisterState(&stateMenu, (u32)eFlowstate::STATE_MENU);

	FlowstateGame stateGame;
	stateManager.RegisterState(&stateGame, (u32)eFlowstate::STATE_PLAY);

	// Entry state
	stateManager.SetInitialState((u32)eFlowstate::STATE_MENU);

	// Run game
	Spear::Core::RunGameloop(60);

	// Shutdown
	Spear::Core::Cleanup();

	return 0;
}