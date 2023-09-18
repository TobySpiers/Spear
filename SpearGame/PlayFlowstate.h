#pragma once
#include "SpearEngine/FlowstateManager.h"

class PlayFlowstate : public Spear::Flowstate
{
public:
	PlayFlowstate(){};
	virtual ~PlayFlowstate(){};

	// Called once when state begins
	void StateEnter() override;

	// Update game. Return a slot id to switch state, return -1 to remain in current state.
	int StateUpdate(float deltaTime) override;

	// Render game
	void StateRender() override;

	// Called once when state ends
	void StateExit() override;
};

enum InputActions
{
	INPUT_UP,
	INPUT_RIGHT,
	INPUT_DOWN,
	INPUT_LEFT,

	INPUT_SHOOT,
	INPUT_ALTSHOOT,

	INPUT_QUIT,

	INPUT_COUNT
};