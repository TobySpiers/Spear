#include "Core.h"
#include "InputManager.h"

namespace Spear
{
	void InputManager::ConfigureInputBindings(int* bindings, int totalBindings)
	{
		m_bindingsSize = totalBindings;
		for (int i = 0; i < m_bindingsSize; i++)
		{
			m_inputBindings[i] = bindings[i];
			m_inputStates[i] = INPUT_INACTIVE;
		}
	}

	void InputManager::RefreshInput(int mousewheelInput)
	{
		// Get latest input data
		SDL_GetRelativeMouseState(&m_mouseAxis.x, &m_mouseAxis.y);
		const u32 rawMouseState = SDL_GetMouseState(&m_mousePos.x, &m_mousePos.y);
		const u8* rawKeyState = SDL_GetKeyboardState(NULL);

		// Update InputStates
		m_mousewheelState = mousewheelInput;
		UpdateInputState(rawMouseState & SDL_BUTTON(SDL_BUTTON_LEFT), &m_clickLeft);
		UpdateInputState(rawMouseState & SDL_BUTTON(SDL_BUTTON_RIGHT), &m_clickRight);
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

	void InputManager::ClearMouseInput()
	{
		m_mouseAxis = Vector2i(0, 0);
		m_mousePos = Vector2i(0, 0);
		m_clickLeft = INPUT_INACTIVE;
		m_clickRight = INPUT_INACTIVE;
		m_mousewheelState = 0;
	}

	void InputManager::ClearKeyInput()
	{
		for (int i = 0; i < m_bindingsSize; i++)
		{
			if (m_inputStates[i] != INPUT_INACTIVE)
			{
				m_inputStates[i] = INPUT_IGNORED;
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
				case INPUT_IGNORED:
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

	Vector2i InputManager::GetMousePos()
	{
		return m_mousePos;
	}

	Vector2i InputManager::GetMouseAxis()
	{
		return m_mouseAxis;
	}
}