#include "SpearEngine/Core.h"
#include "SpearEngine/ServiceLocator.h"
#include "SpearEngine/InputManager.h"
#include "SpearEngine/ScreenRenderer.h"
#include "SpearEngine/AudioManager.h"

#include "Raycaster.h"
#include "eFlowstate.h"
#include "Player.h"
#include "FlowstateGame.h"
#include "LevelFileManager.h"

void FlowstateGame::StateEnter()
{
	// Configure Inputs
	int config[INPUT_COUNT];
	config[INPUT_TOGGLE] = SDL_SCANCODE_SPACE;

	config[INPUT_FORWARD] = SDL_SCANCODE_W;
	config[INPUT_STRAFE_LEFT] = SDL_SCANCODE_A;
	config[INPUT_BACKWARD] = SDL_SCANCODE_S;
	config[INPUT_STRAFE_RIGHT] = SDL_SCANCODE_D;
	config[INPUT_ROTATE_LEFT] = SDL_SCANCODE_Q;
	config[INPUT_ROTATE_RIGHT] = SDL_SCANCODE_E;

	config[INPUT_SPRINT] = SDL_SCANCODE_LSHIFT;

	config[INPUT_SHOOT] = SDL_BUTTON_LEFT;
	config[INPUT_ALTSHOOT] = SDL_BUTTON_RIGHT;
	config[INPUT_QUIT] = SDL_SCANCODE_ESCAPE;
	Spear::ServiceLocator::GetInputManager().ConfigureInputs(config, INPUT_COUNT);

	// Set background colour
	glClearColor(0.f, 0.f, 0.f, 1.f);

	// Load world textures
	m_worldTextures.InitialiseFromDirectory("../Assets/TILESETS/64");
	Spear::ServiceLocator::GetScreenRenderer().CreateSpriteBatch(m_worldTextures, 500);

	// Load world
	LevelFileManager::LoadLevel("level", m_gameState.mapData);
	Raycaster::Init(m_gameState.mapData);

	// Set darkness
	Spear::ServiceLocator::GetScreenRenderer().SetBackgroundDepthFalloff(3.f);

	// Trigger ambient audio loop
	Spear::ServiceLocator::GetAudioManager().LoadMusic("../ASSETS/MUSIC/");
	Spear::ServiceLocator::GetAudioManager().PlayMusic(0);

	// Test CROW SFX
	Spear::ServiceLocator::GetAudioManager().LoadSounds("../ASSETS/SFX/");
	Spear::ServiceLocator::GetAudioManager().PlaySound(0);

	// Position player in middle of grid
	m_gameState.player.SetPos(Vector2f(5, 5));
	SDL_SetRelativeMouseMode(SDL_TRUE);
}

bool view3D{false};
int FlowstateGame::StateUpdate(float deltaTime)
{
	m_gameState.deltaTime = deltaTime;
	m_gameState.gameTime += deltaTime;

	Spear::InputManager& input = Spear::ServiceLocator::GetInputManager();
	if (input.InputRelease(INPUT_QUIT))
	{
		return static_cast<int>(eFlowstate::STATE_MENU);
	}

	if (input.InputStart(INPUT_TOGGLE))
	{
		view3D = !view3D;

		if (view3D)
		{
			const Vector2i res = Raycaster::GetResolution();
			Spear::ServiceLocator::GetScreenRenderer().SetInternalResolution(res.x, res.y);
		}
		else
		{
			Spear::ServiceLocator::GetScreenRenderer().SetInternalResolution(Spear::Core::GetWindowSize().x, Spear::Core::GetWindowSize().y);
		}
	}

	if (input.InputHold(INPUT_SHOOT))
	{
		// stub
	}
	else if (input.InputHold(INPUT_ALTSHOOT))
	{
		// stub
	}

	m_gameState.player.Update(deltaTime);

	Spear::ScreenRenderer::TextData fpsText;
	fpsText.text = std::to_string(deltaTime);
	fpsText.pos = Vector2f(Spear::Core::GetWindowSize().x / 2, 20.f);
	fpsText.alignment = Spear::ScreenRenderer::TEXT_ALIGN_MIDDLE;
	Spear::ServiceLocator::GetScreenRenderer().AddText(fpsText);

	return static_cast<int>(eFlowstate::STATE_THIS);
}

void FlowstateGame::StateRender()
{
	m_gameState.player.Draw(view3D);

	Spear::ServiceLocator::GetScreenRenderer().Render();
}

void FlowstateGame::StateExit()
{
	Spear::ServiceLocator::GetAudioManager().StopMusic();

	view3D = false;
	Spear::ServiceLocator::GetScreenRenderer().ReleaseAll();
	SDL_SetRelativeMouseMode(SDL_FALSE);

	Spear::ServiceLocator::GetScreenRenderer().SetInternalResolution(Spear::Core::GetWindowSize().x, Spear::Core::GetWindowSize().y);
}