#include "Core/Core.h"
#include "Core/ServiceLocator.h"
#include "Core/InputManager.h"
#include "Graphics/ScreenRenderer.h"
#include "Audio/AudioManager.h"
#include "GameObject/AudioEmitter.h"

#include "Raycaster/Raycaster.h"
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

	// Audio setup
	Spear::AudioManager& audio = Spear::AudioManager::Get();
	audio.InitSoundsFromFolder("../ASSETS/SFX/");				// Load SFX from folder
	audio.GlobalPlayStream("../ASSETS/MUSIC/Ambience1.mp3");	// Test file streaming

	// Load world textures
	m_worldTextures.InitialiseFromDirectory("../Assets/TILESETS/64");
	Spear::ServiceLocator::GetScreenRenderer().CreateSpriteBatch(m_worldTextures, 500);

	// Load world
	LevelFileManager::LoadLevel("level", m_gameState.mapData);
	Raycaster::Init(m_gameState.mapData);

	// Set darkness
	Spear::ServiceLocator::GetScreenRenderer().SetBackgroundDepthFalloff(3.f);

	// Position player at PlayerStart
	m_gameState.player.SetPos(m_gameState.mapData.playerStart.ToFloat() + Vector2f(0.5f, 0.5f));
	SDL_SetRelativeMouseMode(SDL_TRUE);

	// Create CrowSFX AudioEmitter GameObject
	//GameObjectPtr<AudioEmitter> emitter = GameObject::Create<AudioEmitter>();
	//emitter->m_bPlayOnStart = true;
	//emitter->m_soundSource.SetSound(0);
	//emitter->m_soundSource.Play();
	//GameObject::GlobalSerialize("../Assets/MAPS/test.objects");

	// Load GameObject (originally serialized via above commented code)
	GameObject::GlobalDeserialize("../Assets/MAPS/test.objects");
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

	return static_cast<int>(eFlowstate::STATE_THIS);
}

void FlowstateGame::StateRender()
{
	m_gameState.player.Draw(view3D);

	Spear::ServiceLocator::GetScreenRenderer().Render();
}

void FlowstateGame::StateExit()
{
	GameObject::GlobalDestroy();

	Spear::AudioManager::Get().StopAllAudio();

	view3D = false;
	Spear::ServiceLocator::GetScreenRenderer().ReleaseAll();
	SDL_SetRelativeMouseMode(SDL_FALSE);

	Spear::ServiceLocator::GetScreenRenderer().SetInternalResolution(Spear::Core::GetWindowSize().x, Spear::Core::GetWindowSize().y);
}