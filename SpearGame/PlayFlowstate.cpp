#include "SpearEngine/Core.h"
#include "SpearEngine/ServiceLocator.h"
#include "SpearEngine/SDLManager.h"
#include "SpearEngine/InputManager.h"

#include "eFlowstate.h"
#include "PlayFlowstate.h"

void PlayFlowstate::StateEnter()
{
	// Configure Inputs
	int config[INPUT_COUNT];
	config[INPUT_UP] = SDL_SCANCODE_UP;
	config[INPUT_LEFT] = SDL_SCANCODE_LEFT;
	config[INPUT_RIGHT] = SDL_SCANCODE_RIGHT;
	config[INPUT_DOWN] = SDL_SCANCODE_DOWN;
	config[INPUT_SHOOT] = SDL_BUTTON_LEFT;
	config[INPUT_ALTSHOOT] = SDL_BUTTON_RIGHT;
	config[INPUT_QUIT] = SDL_SCANCODE_ESCAPE;

	Spear::ServiceLocator::GetInputManager().ConfigureInputs(config, INPUT_COUNT);
}

int PlayFlowstate::StateUpdate(float deltaTime)
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

	Spear::InputManager& input = Spear::ServiceLocator::GetInputManager();
	if (input.InputStart(INPUT_QUIT))
	{
		Spear::Core::SignalShutdown();
	}
	
	return static_cast<int>(eFlowstate::STATE_THIS);
}

void PlayFlowstate::StateRender()
{
	SDL_Renderer& renderer = Spear::ServiceLocator::GetSDLManager().GetRenderer();

	SDL_SetRenderDrawColor(&renderer, 255, 255, 255, 255);
	SDL_RenderClear(&renderer);

	SDL_SetRenderDrawColor(&renderer, 0, 0, 0, 1);
	SDL_RenderDrawLine(&renderer, 50, 50, 120, 200);

	SDL_RenderPresent(&renderer);
}

void PlayFlowstate::StateExit()
{

}