#include "Core.h"
#include "InputManager.h"

namespace Spear
{
	void InputManager::ConfigureInputs(int* bindings, int totalBindings)
	{
		m_bindingsSize = totalBindings;
		for (int i = 0; i < m_bindingsSize; i++)
		{
			m_inputBindings[i] = bindings[i];
			m_inputStates[i] = INPUT_INACTIVE;
		}
	}

	void InputManager::RefreshInput()
	{
		// Get latest input data
		int mouseX{0};
		int mouseY{0};
		const u32 rawMouseState = SDL_GetMouseState(&mouseX, &mouseY);
		const u8* rawKeyState = SDL_GetKeyboardState(NULL);
		m_mousePos.x = mouseX;
		m_mousePos.y = mouseY;

		// Update InputStates
		for (int i = 0; i < m_bindingsSize; i++)
		{
			if (m_inputBindings[i] <= SDL_BUTTON_RIGHT)
			{
				// Update MouseBinding
				UpdateInputState((rawMouseState & SDL_BUTTON(m_inputBindings[i])), &m_inputStates[i]);
			}
			else
			{
				// Update KeyBinding
				UpdateInputState(rawKeyState[m_inputBindings[i]], &m_inputStates[i]);
			}
		}
	}

	void InputManager::UpdateInputState(bool isActive, int* state)
	{
		ASSERT(state != nullptr);
		if (isActive)
		{
			switch (*state)
			{
				case INPUT_INACTIVE:
				*state = INPUT_START;
				break;

				case INPUT_START:
				*state = INPUT_ACTIVE;
				break;
			}
		}
		else
		{
			switch (*state)
			{
				case INPUT_ACTIVE:
				case INPUT_START:
				*state = INPUT_RELEASED;
				break;

				case INPUT_RELEASED:
				*state = INPUT_INACTIVE;
				break;
			}
		}
	}

	bool InputManager::InputStart(int input)
	{
		ASSERT(input < m_bindingsSize && input >= 0);
		return (m_inputStates[input] == INPUT_START);
	}

	bool InputManager::InputHold(int input)
	{
		ASSERT(input < m_bindingsSize && input >= 0);
		return (m_inputStates[input] == INPUT_ACTIVE || m_inputStates[input] == INPUT_START);
	}

	bool InputManager::InputRelease(int input)
	{
		ASSERT(input < m_bindingsSize && input >= 0);
		return (m_inputStates[input] == INPUT_RELEASED);
	}

	Vector2D InputManager::GetMousePos()
	{
		return m_mousePos;
	}
}