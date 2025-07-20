#include "Core/Core.h"
#include "Core/ServiceLocator.h"
#include "Core/InputManager.h"
#include "Core/ImguiManager.h"
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
	config[INPUT_FORWARD] = SDL_SCANCODE_W;
	config[INPUT_BACKWARD] = SDL_SCANCODE_S;
	config[INPUT_STRAFE_LEFT] = SDL_SCANCODE_A;
	config[INPUT_STRAFE_RIGHT] = SDL_SCANCODE_D;
	config[INPUT_ROTATE_LEFT] = SDL_SCANCODE_Q;
	config[INPUT_ROTATE_RIGHT] = SDL_SCANCODE_E;
	config[INPUT_SPRINT] = SDL_SCANCODE_LSHIFT;
	config[INPUT_SHOOT] = SDL_BUTTON_LEFT;
	config[INPUT_ALTSHOOT] = SDL_BUTTON_RIGHT;

	config[INPUT_TOGGLE_RAYCASTER] = SDL_SCANCODE_SPACE;
	config[INPUT_TOGGLE_IMGUI] = SDL_SCANCODE_F1;
	config[INPUT_TOGGLE_IMGUI_INPUT] = SDL_SCANCODE_F2;
	config[INPUT_QUIT] = SDL_SCANCODE_ESCAPE;
	Spear::ServiceLocator::GetInputManager().ConfigureInputBindings(config, INPUT_COUNT);

	// Set background colour
	glClearColor(0.f, 0.f, 0.f, 1.f);

	// Audio setup
	Spear::AudioManager& audio = Spear::AudioManager::Get();
	audio.InitSoundsFromFolder("../ASSETS/SFX/");				// Load SFX from folder
	audio.GlobalPlayStream("../ASSETS/MUSIC/Ambience1.mp3");	// Test file streaming

	// Load globally used textures (map/sprites)
	// If any game-specific texture batches are required, these can be loaded separately after
	GlobalTextureBatches::InitialiseBatches(m_textures);

	// Load world
	LevelFileManager::LoadLevel("Test.level", m_gameState.mapData);
	Raycaster::Init(m_gameState.mapData);

	// Set darkness
	Spear::ServiceLocator::GetScreenRenderer().SetBackgroundDepthFalloff(3.f);

	// Position player at PlayerStart
	m_gameState.player.SetPos(m_gameState.mapData.playerStart.ToFloat() + Vector2f(0.5f, 0.5f));
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

	Spear::ImguiManager& imgui = Spear::ImguiManager::Get();
	if (input.InputStart(INPUT_TOGGLE_IMGUI))
	{
		bool imguiEnabled = !imgui.ArePanelsEnabled();
		imgui.SetPanelsEnabled(imguiEnabled);
		imgui.SetMenuBarLabel(GetDebugInputModeText());
		if (!imguiEnabled)
		{
			SDL_SetRelativeMouseMode(SDL_TRUE);
		}
		else 
		{
			SDL_SetRelativeMouseMode(debugInputMode == DEBUGINPUT_FULL ? SDL_TRUE : SDL_FALSE);
		}
	}
	if (imgui.ArePanelsEnabled())
	{
		if (input.InputStart(INPUT_TOGGLE_IMGUI_INPUT))
		{
			debugInputMode = static_cast<FlowstateGame::DebugInputModes>((debugInputMode + 1) % DEBUGINPUT_COUNT);
			switch (debugInputMode)
			{
			case DEBUGINPUT_DISABLED:
				SDL_SetRelativeMouseMode(SDL_FALSE);
				break;
			case DEBUGINPUT_KEYBOARD:
				SDL_SetRelativeMouseMode(SDL_FALSE);
				break;
			case DEBUGINPUT_FULL:
				SDL_SetRelativeMouseMode(SDL_TRUE);
				break;
			}

			imgui.SetMenuBarLabel(GetDebugInputModeText());
		}
		switch (debugInputMode)
		{
		case DEBUGINPUT_DISABLED:
			input.ClearMouseInput();
			input.ClearKeyInput();
			break;
		case DEBUGINPUT_KEYBOARD:
			input.ClearMouseInput();
			break;
		}
	}

	if (input.InputStart(INPUT_TOGGLE_RAYCASTER))
	{
		view3D = !view3D;
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

	GameObject::GlobalTick(deltaTime);

	return static_cast<int>(eFlowstate::STATE_THIS);
}

void FlowstateGame::StateRender()
{
	GameObject::GlobalDraw();

	m_gameState.player.Draw(view3D);

	Spear::ServiceLocator::GetScreenRenderer().Render();

	Raycaster::ClearSprites();
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

const char* FlowstateGame::GetDebugInputModeText()
{
	switch (debugInputMode)
	{
	case DEBUGINPUT_DISABLED:
		return "Game Input: Disabled (F2)";
	case DEBUGINPUT_KEYBOARD:
		return "Game Input: Keyboard Only (F2)";
	case DEBUGINPUT_FULL:
		return "Game Input: Full (F2)";
	default:
		return "Game Input: Unknown (F2)";
	}
}
