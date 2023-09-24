#include "SpearEngine/Core.h"
#include "SpearEngine/ServiceLocator.h"
#include "SpearEngine/InputManager.h"
#include "SpearEngine/ScreenRenderer.h"
#include "SpearEngine/Raycaster.h"

#include "eFlowstate.h"
#include "Player.h"
#include "PlayFlowstate.h"

Spear::RaycastDDAGrid rayGrid;

Spear::RaycastWall* pWalls{nullptr};
const int wallSize{3};

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

	// Create some walls and register them with player
	Vector2f screen{Spear::Core::GetWindowSize().ToFloat()};
	pWalls = new Spear::RaycastWall[wallSize];
	pWalls[0].origin = Vector2f(screen.x / 3, 200);
	pWalls[0].vec = Vector2f(0, 400);
	pWalls[0].colour = Colour4f::Red();
	pWalls[1].origin = Vector2f(screen.x - (screen.x / 3), 200);
	pWalls[1].vec = Vector2f(0, 400);
	pWalls[1].colour = Colour4f::Blue();
	pWalls[2].origin = Vector2f(400, 200);
	pWalls[2].vec = Vector2f(800, 0);
	pWalls[2].colour = Colour4f::Green();
	player.RegisterWalls(pWalls, wallSize);

	// Create a basic DDA grid
	rayGrid.width = 10;
	rayGrid.height = 10;
	rayGrid.pValues = new u8[rayGrid.width * rayGrid.height] {
		
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
	player.RegisterGrid(&rayGrid);

	// Position player in middle of screen
	player.SetPos(Vector2f((static_cast<float>(rayGrid.width / 2)), (static_cast<float>(rayGrid.height) / 2)));
}

bool viewPerspective{false};
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
		player.m_rayParams.xResolution++;
	}
	else if (input.InputHold(INPUT_ALTSHOOT))
	{
		player.m_rayParams.xResolution--;
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
	delete[] pWalls;
	delete[] rayGrid.pValues;
}