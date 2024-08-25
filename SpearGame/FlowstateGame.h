#pragma once
#include "SpearEngine/FlowstateManager.h"
#include "SpearEngine/TextureArray.h"
#include "GameState.h"


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
	Spear::TextureArray m_worldTextures;
	GameState m_gameState;
};

enum InputActions
{
	INPUT_TOGGLE,

	INPUT_UP,
	INPUT_RIGHT,
	INPUT_DOWN,
	INPUT_LEFT,
	INPUT_ROTATE_LEFT,
	INPUT_ROTATE_RIGHT,

	INPUT_SHOOT,
	INPUT_ALTSHOOT,

	INPUT_QUIT,

	INPUT_COUNT
};