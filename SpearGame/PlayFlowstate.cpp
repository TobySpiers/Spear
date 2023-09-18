#include "SpearEngine/Core.h"
#include "SpearEngine/ServiceLocator.h"
#include "SpearEngine/GfxManager.h"
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
	Spear::GfxManager& gfx = Spear::ServiceLocator::GetGfxManager();

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	glClearColor(1.0f, 0.5f, 0.5f, 1.f);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT); 

	glUseProgram(gfx.GetPipeline());

	glBindVertexArray(gfx.GetVertexArray());
	glBindBuffer(GL_ARRAY_BUFFER, gfx.GetVertexBuffer());

	glDrawArrays(GL_TRIANGLES, 0, 3);

	SDL_GL_SwapWindow(&gfx.GetWindow());
}

void PlayFlowstate::StateExit()
{

}