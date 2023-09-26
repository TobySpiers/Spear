#include "SpearEngine/Core.h"
#include "SpearEngine/ServiceLocator.h"
#include "SpearEngine/InputManager.h"
#include "SpearEngine/ScreenRenderer.h"
#include "SpearEngine/Raycaster.h"

#include "eFlowstate.h"
#include "Player.h"
#include "PlayFlowstate.h"

void PlayFlowstate::StateEnter()
{
	// Configure Inputs
	int config[INPUT_COUNT];
	config[INPUT_TOGGLE] = SDL_SCANCODE_SPACE;

	config[INPUT_UP] = SDL_SCANCODE_W;
	config[INPUT_LEFT] = SDL_SCANCODE_A;
	config[INPUT_RIGHT] = SDL_SCANCODE_D;
	config[INPUT_DOWN] = SDL_SCANCODE_S;

	config[INPUT_ROTATE_LEFT] = SDL_SCANCODE_Q;
	config[INPUT_ROTATE_RIGHT] = SDL_SCANCODE_E;

	config[INPUT_SHOOT] = SDL_BUTTON_LEFT;
	config[INPUT_ALTSHOOT] = SDL_BUTTON_RIGHT;
	config[INPUT_QUIT] = SDL_SCANCODE_ESCAPE;
	Spear::ServiceLocator::GetInputManager().ConfigureInputs(config, INPUT_COUNT);

	// Set background colour
	glClearColor(0.5f, 0.5f, 0.5f, 1.f);

	// Create world layout
	const int gridWidth{10};
	const int gridHeight{10};
	const s8 worldIds[gridWidth * gridHeight] = {
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 0, 0, 0, 0, 0, 0, 1, 1, 0,
		1, 0, 0, 1, 0, 0, 1, 0, 0, 0,
		1, 0, 0, 0, 1, 0, 1, 0, 1, 0,
		1, 0, 0, 0, 0, 1, 0, 0, 0, 0,
		1, 0, 0, 0, 0, 0, 0, -1, 0, 0,
		1, 1, 1, 1, 0, 0, -1, 0, -1, 0,
		1, 0, 0, 1, 0, 0, 0, -1, 0, 0,
		1, 0, 0, 0, 0, 0, 0, 0, 1, 0,
		1, 1, 4, 4, 2, 2, 3, 3, 4, 4
	};
	const u8 roofIds[gridWidth * gridHeight] = {
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 0, 0, 0, 0, 0, 0, 1, 1, 0,
		1, 0, 0, 1, 0, 0, 1, 0, 0, 0,
		1, 0, 0, 0, 1, 0, 1, 0, 1, 0,
		1, 0, 0, 0, 0, 1, 0, 0, 0, 0,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
		1, 0, 0, 1, 0, 0, 0, 0, 0, 0,
		1, 0, 0, 0, 0, 0, 0, 0, 1, 0,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1
	};
	Spear::Raycaster::SubmitNewGrid(gridWidth, gridHeight, worldIds, roofIds);

	// Position player in middle of grid
	player.SetPos(Vector2f(gridWidth / 2, gridHeight / 2));
}

bool viewPerspective{false};
Spear::RaycastParams rayParams;
int PlayFlowstate::StateUpdate(float deltaTime)
{
	Spear::InputManager& input = Spear::ServiceLocator::GetInputManager();
	if (input.InputStart(INPUT_QUIT))
	{
		Spear::Core::SignalShutdown();
	}

	if (input.InputStart(INPUT_TOGGLE))
	{
		viewPerspective = !viewPerspective;
	}

	if (input.InputHold(INPUT_SHOOT))
	{
		rayParams.xResolution++;
		rayParams.yResolution++;
		Spear::Raycaster::ApplyConfig(rayParams);
	}
	else if (input.InputHold(INPUT_ALTSHOOT))
	{
		rayParams.xResolution--;
		rayParams.yResolution--;
		Spear::Raycaster::ApplyConfig(rayParams);
	}

	player.Update(deltaTime);

	return static_cast<int>(eFlowstate::STATE_THIS);
}

void PlayFlowstate::StateRender()
{

	player.Draw(viewPerspective);

	Spear::ServiceLocator::GetLineRenderer().Render();
}

void PlayFlowstate::StateExit()
{
	
}