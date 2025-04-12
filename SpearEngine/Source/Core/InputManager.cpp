#include "Core.h"
#include "InputManager.h"
#include <imgui.h>
#include <Core/ServiceLocator.h>

namespace Spear
{
	InputManager& InputManager::Get()
	{
		return ServiceLocator::GetInputManager();
	}

	void InputManager::ConfigureInputBindings(int* bindings, int totalBindings)
	{
		ASSERT(totalBindings <= KEYBINDINGS_LIMIT);

		m_bindingsSize = totalBindings;
		for (int i = 0; i < m_bindingsSize; i++)
		{
			m_inputBindings[i] = bindings[i];
			m_inputStates[i] = INPUT_INACTIVE;
		}
	}

	void InputManager::RefreshInput(int mousewheelInput)
	{
		// Refresh Cursor state
		SDL_GetRelativeMouseState(&m_mouseAxis.x, &m_mouseAxis.y);
		u32 rawMouseState;
		if (ImGui::GetIO().WantCaptureMouseUnlessPopupClose)
		{
			rawMouseState = 0;
			m_mousePos.x = 0;
			m_mousePos.y = 0;
			m_mousewheelState = 0;
		}
		else
		{
			rawMouseState = SDL_GetMouseState(&m_mousePos.x, &m_mousePos.y);
			m_mousewheelState = mousewheelInput;
		}

		// Refresh Bound states
		const u8* rawKeyState = SDL_GetKeyboardState(NULL);
		UpdateInputState(rawMouseState & SDL_BUTTON(SDL_BUTTON_LEFT), &m_clickLeft);
		UpdateInputState(rawMouseState & SDL_BUTTON(SDL_BUTTON_RIGHT), &m_clickRight);
		UpdateInputState(rawMouseState & SDL_BUTTON(SDL_BUTTON_MIDDLE), &m_clickMiddle);
		for (int i = 0; i < m_bindingsSize; i++)
		{
			if (m_inputBindings[i] <= SDL_BUTTON_RIGHT)
			{
				// Update MouseBinding
				UpdateInputState(rawMouseState & SDL_BUTTON(m_inputBindings[i]), &m_inputStates[i]);
			}
			else
			{
				// Update KeyBinding
				UpdateInputState(ImGui::GetIO().WantCaptureKeyboard ? false : rawKeyState[m_inputBindings[i]], &m_inputStates[i]);
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