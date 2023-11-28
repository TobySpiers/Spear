#include "SpearEngine/Core.h"
#include "SpearEngine/ServiceLocator.h"
#include "SpearEngine/InputManager.h"
#include "SpearEngine/ScreenRenderer.h"

#include "Raycaster.h"
#include "eFlowstate.h"
#include "Player.h"
#include "FlowstateGame.h"

void FlowstateGame::StateEnter()
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
	glClearColor(0.f, 1.f, 0.f, 1.f);

	// Load world textures
	m_worldTextures.Allocate(64, 64, 4); // 64x64 textures (4 slots)
	m_worldTextures.SetDataFromFile(0, "../Assets/SPRITES/wall64_wolf.png");
	m_worldTextures.SetDataFromFile(1, "../Assets/SPRITES/wall64_rough.png");
	m_worldTextures.SetDataFromFile(2, "../Assets/SPRITES/wall64_rough.png");
	m_worldTextures.SetDataFromFile(3, "../Assets/SPRITES/wall64_wolf.png");
	Spear::ServiceLocator::GetScreenRenderer().CreateSpriteBatch(m_worldTextures, 500);
	Spear::ServiceLocator::GetScreenRenderer().CreateLineBatch(m_worldTextures, 1800);

	// Load world
	Raycaster::LoadLevel("level");

	// Position player in middle of grid
	m_player.SetPos(Vector2f(5, 5));
	SDL_SetRelativeMouseMode(SDL_TRUE);
}

bool viewPerspective{false};
RaycastParams rayParams;
int FlowstateGame::StateUpdate(float deltaTime)
{
	Spear::InputManager& input = Spear::ServiceLocator::GetInputManager();
	if (input.InputRelease(INPUT_QUIT))
	{
		return static_cast<int>(eFlowstate::STATE_MENU);
	}

	if (input.InputStart(INPUT_TOGGLE))
	{
		viewPerspective = !viewPerspective;
	}

	if (input.InputHold(INPUT_SHOOT))
	{
		rayParams.xResolution++;
		rayParams.yResolution++;
		Raycaster::ApplyConfig(rayParams);
	}
	else if (input.InputHold(INPUT_ALTSHOOT))
	{
		rayParams.xResolution--;
		rayParams.yResolution--;
		Raycaster::ApplyConfig(rayParams);
	}

	m_player.Update(deltaTime);

	Spear::ScreenRenderer::TextData fpsText;
	fpsText.text = std::to_string(deltaTime);
	fpsText.pos = Vector2f(Spear::Core::GetWindowSize().x / 2, 20.f);
	fpsText.alignment = Spear::ScreenRenderer::TEXT_ALIGN_MIDDLE;
	Spear::ServiceLocator::GetScreenRenderer().AddText(fpsText);

	return static_cast<int>(eFlowstate::STATE_THIS);
}

void FlowstateGame::StateRender()
{

	m_player.Draw(viewPerspective);

	Spear::ServiceLocator::GetScreenRenderer().Render();
}

void FlowstateGame::StateExit()
{
	Spear::ServiceLocator::GetScreenRenderer().ReleaseAll();
	SDL_SetRelativeMouseMode(SDL_FALSE);
}