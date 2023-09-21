#include "SpearEngine/Core.h"
#include "SpearEngine/ServiceLocator.h"
#include "SpearEngine/WindowManager.h"
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
	elapsedTime += deltaTime * 1000.f;

	int dir{1};
	float yOffset{1.f};
	for (int x = 1; x < Spear::Core::GetWindowSize().x; x += 6)
	{
		Spear::LineData line;
		line.start = Vector2D((x + (int)elapsedTime) % (int)Spear::Core::GetWindowSize().x, yOffset);
		line.end = Vector2D((x + (int)elapsedTime) % (int)Spear::Core::GetWindowSize().x, Spear::Core::GetWindowSize().y);
		line.r = ((x / 3) % 3) == 0 ? 1.f : 0.f;
		line.g = ((x / 3) % 3) == 1 ? 1.f : 0.f;
		line.b = ((x / 3) % 3) == 2 ? 1.f : 0.f;
		Spear::ServiceLocator::GetLineRenderer().AddLine(line);

		yOffset += (10 * dir);
		if (yOffset > Spear::Core::GetWindowSize().y || yOffset < 1)
		{
			dir *= -1;
		}
	}
	Spear::ServiceLocator::GetLineRenderer().SetLineWidth(6.f);

	return static_cast<int>(eFlowstate::STATE_THIS);
}

void PlayFlowstate::StateRender()
{
	Spear::ServiceLocator::GetLineRenderer().Render();
	SDL_GL_SwapWindow(&Spear::ServiceLocator::GetWindowManager().GetWindow());
}

void PlayFlowstate::StateExit()
{

}