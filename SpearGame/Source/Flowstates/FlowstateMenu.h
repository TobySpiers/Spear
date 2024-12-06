#pragma once
#include "Core/FlowstateManager.h"
#include "Graphics/TextureArray.h"
#include "UiButton.h"

constexpr int MENU_BUTTON_TOTAL{2};

class FlowstateMenu : public Spear::Flowstate
{
	enum InputActions
	{
		INPUT_SELECT,
		INPUT_QUIT,

		INPUT_COUNT
	};

public:
	FlowstateMenu() {};
	virtual ~FlowstateMenu() {};

	// Called once when state begins
	void StateEnter() override;

	// Update game. Return a slot id to switch state, return -1 to remain in current state.
	int StateUpdate(float deltaTime) override;

	// Render game
	void StateRender() override;

	// Called once when state ends
	void StateExit() override;

	Spear::TextureArray m_menuTextures;
	Spear::UiButton m_buttons[MENU_BUTTON_TOTAL];
};
