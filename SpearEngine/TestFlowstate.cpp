#include "Core.h"
#include "TestFlowstate.h"
#include "SDLManager.h"
#include "ServiceLocator.h"

void TestFlowstate::StateEnter()
{

}

int TestFlowstate::StateUpdate()
{
	// Handle SDL events
	SDL_Event event;
	SDL_PollEvent(&event); // get any pending event
	switch (event.type)
	{
	case SDL_QUIT:
		Spear::Core::SignalShutdown();
		break;

	default:
		break;
	}

	return -1;
}

void TestFlowstate::StateRender()
{
	SDL_Renderer& renderer = Spear::ServiceLocator::GetSDLManager().GetRenderer();

	SDL_SetRenderDrawColor(&renderer, 255, 255, 255, 255);
	SDL_RenderClear(&renderer);

	SDL_SetRenderDrawColor(&renderer, 0, 0, 0, 1);
	SDL_RenderDrawLine(&renderer, 50, 50, 120, 200);

	SDL_RenderPresent(&renderer);
}

void TestFlowstate::StateExit()
{

}