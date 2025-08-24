#pragma once
#include "Core/FlowstateManager.h"
#include "Graphics/TextureArray.h"
#include "GameState.h"
#include "GlobalTextureBatches.h"

class FlowstateGame : public Spear::Flowstate
{
public:
	FlowstateGame(){};
	virtual ~FlowstateGame(){};

	// Function to access flowstate data
	GameState& GetGameState() { return m_gameState; };

	// Called once when state begins
	void StateEnter() override;

	// Update game. Return a slot id to switch state, return -1 to remain in current state.
	int StateUpdate(float deltaTime) override;

	// Render game
	void StateRender() override;

	// Called once when state ends
	void StateExit() override;

private:
	Spear::TextureArray m_textures[GlobalTextureBatches::BATCH_TOTALS];
	GameState m_gameState;

	enum DebugInputModes
	{
		DEBUGINPUT_DISABLED,
		DEBUGINPUT_KEYBOARD,
		DEBUGINPUT_FULL,

		DEBUGINPUT_COUNT
	};
	DebugInputModes debugInputMode{ DEBUGINPUT_KEYBOARD };
	const char* GetDebugInputModeText();

	bool m_view3D{false};
};

enum InputActions
{
	INPUT_FORWARD,
	INPUT_BACKWARD,
	INPUT_STRAFE_RIGHT,
	INPUT_STRAFE_LEFT,
	INPUT_ROTATE_LEFT,
	INPUT_ROTATE_RIGHT,
	INPUT_SPRINT,
	INPUT_SHOOT,
	INPUT_ALTSHOOT,

	INPUT_TOGGLE_RAYCASTER,
	INPUT_TOGGLE_IMGUI,
	INPUT_TOGGLE_IMGUI_INPUT,
	INPUT_QUIT,

	INPUT_COUNT
};