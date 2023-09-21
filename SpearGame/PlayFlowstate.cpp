#include "SpearEngine/Core.h"
#include "SpearEngine/ServiceLocator.h"
#include "SpearEngine/InputManager.h"
#include "SpearEngine/LineRenderer.h"

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
	
	static float elapsedTime{0.f};
	elapsedTime += deltaTime;

	static int segment{3};
	static int inc{1};
	if (elapsedTime > 0.3f)
	{
		elapsedTime = 0.f;
		segment += inc;
		if (segment == 6 || segment == 2)
		{
			inc *= -1;
		}
	}

	Spear::LinePolyData poly;
	poly.pos = Spear::ServiceLocator::GetInputManager().GetMousePos();
	poly.radius = 50.f;
	poly.segments = segment;
	poly.rotation = elapsedTime;
	poly.r = 1.0f;
	Spear::ServiceLocator::GetLineRenderer().AddLinePoly(poly);

	return static_cast<int>(eFlowstate::STATE_THIS);
}

void PlayFlowstate::StateRender()
{
	Spear::ServiceLocator::GetLineRenderer().SetLineWidth(5.f);
	Spear::ServiceLocator::GetLineRenderer().Render();
}

void PlayFlowstate::StateExit()
{

}