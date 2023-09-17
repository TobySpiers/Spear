#pragma once
#include "FlowstateManager.h"

class TestFlowstate : public Spear::Flowstate
{
public:
	TestFlowstate(){};
	virtual ~TestFlowstate() {};

	// Called once when state begins
	void StateEnter() override;

	// Update game. Return a slot id to switch state, return -1 to remain in current state.
	int StateUpdate() override;

	// Render game
	void StateRender() override;

	// Called once when state ends
	void StateExit() override;
};