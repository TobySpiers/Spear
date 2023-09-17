#include "Core.h"
#include "InputManager.h"

namespace Spear
{
	InputManager::InputManager()
	{
		m_keyStates.reserve(32); // assuming a max limit of 32 unique keys needed for gameplay
	}

	void InputManager::RegisterKeys(int* pKeyList, u8 size)
	{
		m_keyStates.clear();
		for (u8 i = 0; i < size; i++)
		{
			m_keyStates.insert({pKeyList[i], 0});
		}
	}

	void InputManager::RefreshInput()
	{
		const u8* rawState = SDL_GetKeyboardState(NULL);

		for (const auto& pair : m_keyStates)
		{
			// if registered key is currently pressed...
			if (rawState[pair.first])
			{
				// inactive -> start
				// start -> active
				switch (m_keyStates.at(pair.first))
				{
				case KEY_INACTIVE:
					m_keyStates.at(pair.first) = KEY_START;
					break;

				case KEY_START:
					m_keyStates.at(pair.first) = KEY_ACTIVE;
					break;
				}
			}
			else
			{
				// start/active -> released
				// released -> inactive
				switch (m_keyStates.at(pair.first))
				{
				case KEY_START:
				case KEY_ACTIVE:
					m_keyStates.at(pair.first) = KEY_RELEASED;
					break;

				case KEY_RELEASED:
					m_keyStates.at(pair.first) = KEY_INACTIVE;
					break;
				}
			}
		}
	}

	bool InputManager::KeyStart(int key)
	{
		ASSERT(m_keyStates.count(key));
		return (m_keyStates.at(key) == KEY_START);
	}

	bool InputManager::KeyHold(int key)
	{
		ASSERT(m_keyStates.count(key));
		return (m_keyStates.at(key) == KEY_ACTIVE || m_keyStates.at(key) == KEY_START);
	}

	bool InputManager::KeyRelease(int key)
	{
		ASSERT(m_keyStates.count(key));
		return (m_keyStates.at(key) == KEY_RELEASED);
	}
}