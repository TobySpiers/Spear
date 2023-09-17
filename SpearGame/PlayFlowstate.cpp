#include "SpearEngine/Core.h"
#include "PlayFlowstate.h"
#include "SpearEngine/ServiceLocator.h"
#include "SpearEngine/SDLManager.h"
#include "SpearEngine/InputManager.h"
#include "eFlowstate.h"

void PlayFlowstate::StateEnter()
{
	Spear::InputManager& inputManager = Spear::ServiceLocator::GetInputManager();

	// temp: configure input keys
	const int keyCount{5};
	int keys[keyCount] =
	{
		SDL_SCANCODE_LEFT,
		SDL_SCANCODE_RIGHT,
		SDL_SCANCODE_UP,
		SDL_SCANCODE_DOWN,

		SDL_SCANCODE_ESCAPE
	};

	inputManager.RegisterKeys(keys, keyCount);
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

	// Input
	Spear::InputManager& inputManager = Spear::ServiceLocator::GetInputManager();

	if (inputManager.KeyStart(SDL_SCANCODE_ESCAPE))
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