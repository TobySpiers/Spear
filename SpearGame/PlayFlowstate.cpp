#include "SpearEngine/Core.h"
#include "SpearEngine/ServiceLocator.h"
#include "SpearEngine/InputManager.h"
#include "SpearEngine/LineRenderer.h"
#include "SpearEngine/Raycaster.h"

#include "eFlowstate.h"
#include "Player.h"
#include "PlayFlowstate.h"

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

	config[INPUT_ROTATE_LEFT] = SDL_SCANCODE_LEFT;
	config[INPUT_ROTATE_RIGHT] = SDL_SCANCODE_RIGHT;

	config[INPUT_SHOOT] = SDL_BUTTON_LEFT;
	config[INPUT_ALTSHOOT] = SDL_BUTTON_RIGHT;
	config[INPUT_QUIT] = SDL_SCANCODE_ESCAPE;
	Spear::ServiceLocator::GetInputManager().ConfigureInputs(config, INPUT_COUNT);

	// Set background colour
	glClearColor(0.5f, 0.5f, 0.5f, 1.f);

	// Position player in middle of screen
	Vector2D screen{Spear::Core::GetWindowSize()};
	player.SetPos(Vector2D(screen.x / 2, screen.y / 2));

	// Create some walls and register them with player
	pWalls = new Spear::RaycastWall[wallSize];
	pWalls[0].origin = Vector2D(screen.x / 3, 200);
	pWalls[0].vec = Vector2D(0, 400);
	pWalls[0].colour = Colour::Red();

	pWalls[1].origin = Vector2D(screen.x - (screen.x / 3), 200);
	pWalls[1].vec = Vector2D(0, 400);
	pWalls[1].colour = Colour::Blue();

	pWalls[2].origin = Vector2D(400, 200);
	pWalls[2].vec = Vector2D(800, 0);
	pWalls[2].colour = Colour::Green();

	player.RegisterWalls(pWalls, wallSize);
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

	player.Update();

	return static_cast<int>(eFlowstate::STATE_THIS);
}

void PlayFlowstate::StateRender()
{

	player.Draw(viewPerspective);

	if(!viewPerspective)
	{
		for (int i = 0; i < wallSize; i++)
		{
			pWalls[i].Draw();
		}
	}
	Spear::ServiceLocator::GetLineRenderer().Render();
}

void PlayFlowstate::StateExit()
{

}