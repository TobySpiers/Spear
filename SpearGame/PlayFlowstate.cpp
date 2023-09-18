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

	Spear::InputManager& input = Spear::ServiceLocator::GetInputManager(); // temp

	// Clear screen white
	SDL_SetRenderDrawColor(&renderer, 255, 255, 255, 255);
	SDL_RenderClear(&renderer);

	// Mouse Rect
	SDL_SetRenderDrawColor(&renderer, 0, 0, 0, 1);
	SDL_Rect rect;
	rect.x = input.MousePos().x;
	rect.y = input.MousePos().y;
	rect.h = 50;
	rect.w = 50;
	SDL_RenderDrawRect(&renderer, &rect);

	// Auto Rect
	static int recX = 0;
	recX += 1;
	SDL_Rect rectAuto;
	rectAuto.x = recX;
	rectAuto.y = 50;
	rectAuto.h = 50;
	rectAuto.w = 50;
	SDL_RenderFillRect(&renderer, &rectAuto);
	

	// Present screen
	SDL_RenderPresent(&renderer);
}

void PlayFlowstate::StateExit()
{

}