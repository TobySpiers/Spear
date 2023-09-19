#include "SpearEngine/Core.h"
#include "SpearEngine/ServiceLocator.h"
#include "SpearEngine/WindowManager.h"
#include "SpearEngine/InputManager.h"
#include "SpearEngine/Renderer.h"

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
	Spear::ServiceLocator::GetRenderer().Render();
	SDL_GL_SwapWindow(&Spear::ServiceLocator::GetWindowManager().GetWindow());
}

void PlayFlowstate::StateExit()
{

}