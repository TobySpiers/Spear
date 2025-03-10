#include "Core/Core.h"
#include "Core/ServiceLocator.h"
#include "Core/InputManager.h"
#include "Core/ImguiManager.h"
#include "Graphics/ScreenRenderer.h"
#include "UiButton.h"

#include "eFlowstate.h"
#include "FlowstateMenu.h"

void FlowstateMenu::StateEnter()
{
	// Configure Inputs
	int config[INPUT_COUNT];
	config[INPUT_SELECT] = SDL_BUTTON_LEFT;
	config[INPUT_QUIT] = SDL_SCANCODE_ESCAPE;
	Spear::ServiceLocator::GetInputManager().ConfigureInputBindings(config, INPUT_COUNT);

	// Set background colour
	glClearColor(0.5f, 0.5f, 0.5f, 1.f);

	// Load menu textures
	m_menuTextures.Allocate(64, 64, 2); // 64x64 textures (2 slots)
	m_menuTextures.SetDataFromFile(0, "../Assets/SPRITES/mode_Editor.png");
	m_menuTextures.SetDataFromFile(1, "../Assets/SPRITES/mode_Play.png");
	Spear::ServiceLocator::GetScreenRenderer().CreateSpriteBatch(m_menuTextures, 100);

	// Editor Button
	m_buttons[0].Initialise(m_menuTextures);
	m_buttons[0].m_sprite.texLayer = 0;
	m_buttons[0].m_sprite.pos = Vector2f((Spear::Core::GetWindowSize().x / 2) - 250, Spear::Core::GetWindowSize().y / 2);
	m_buttons[0].m_sprite.size = Vector2f(2.5f, 2.5f);
	m_buttons[0].m_sprite.opacity = 0.75;

	// Play Button
	m_buttons[1].Initialise(m_menuTextures);
	m_buttons[1].m_sprite.texLayer = 1;
	m_buttons[1].m_sprite.pos = Vector2f((Spear::Core::GetWindowSize().x / 2) + 250, Spear::Core::GetWindowSize().y / 2);
	m_buttons[1].m_sprite.size = Vector2f(2.5f, 2.5f);
	m_buttons[1].m_sprite.opacity = 0.75;

	Spear::ImguiManager::Get().SetPanelsEnabled(true);
	Spear::ImguiManager::Get().SetMenuBarLabel("F1 to Toggle In-Game");
}

int FlowstateMenu::StateUpdate(float deltaTime)
{
	Spear::InputManager& input = Spear::ServiceLocator::GetInputManager();
	if (input.InputRelease(INPUT_QUIT))
	{
		Spear::Core::SignalShutdown();
	}

	for (int i = 0; i < MENU_BUTTON_TOTAL; i++)
	{
		m_buttons[i].Update();
	}

	// Editor Button
	if (m_buttons[0].Clicked())
	{
		return static_cast<int>(eFlowstate::STATE_EDIT);
	}
	// Play Button
	else if (m_buttons[1].Clicked())
	{
		return static_cast<int>(eFlowstate::STATE_PLAY);
	}

	return static_cast<int>(eFlowstate::STATE_THIS);
}

void FlowstateMenu::StateRender()
{
	for (int i = 0; i < MENU_BUTTON_TOTAL; i++)
	{
		m_buttons[i].Draw(0);
	}

	Spear::ServiceLocator::GetScreenRenderer().Render();
}

void FlowstateMenu::StateExit()
{
	Spear::ServiceLocator::GetScreenRenderer().ReleaseAll();

	Spear::ImguiManager::Get().SetPanelsEnabled(false);
	Spear::ImguiManager::Get().SetMenuBarLabel(nullptr);
}