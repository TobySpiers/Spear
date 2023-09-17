#pragma once

namespace Spear
{
	class InputManager
	{
	public:
		NO_COPY(InputManager);

		InputManager();
		~InputManager(){};

		void RegisterKeys(int* pKeyList, u8 size);

		void RefreshInput();
		bool KeyStart(int key);
		bool KeyHold(int key);
		bool KeyRelease(int key);

	private:
		std::unordered_map<int, u8> m_keyStates;

		enum KeyState
		{
			KEY_INACTIVE,
			KEY_START,
			KEY_ACTIVE,
			KEY_RELEASED
		};
	};
};