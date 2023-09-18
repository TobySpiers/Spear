#pragma once

namespace Spear
{
	class InputManager
	{
	public:
		NO_COPY(InputManager);

		InputManager(){};
		~InputManager(){};

		void ConfigureInputs(int* bindings, int totalBindings);
		void RefreshInput();

		bool InputStart(int key);
		bool InputHold(int key);
		bool InputRelease(int key);

	private:
		void UpdateInputState(bool active, int* state);

		static const int KEYBINDINGS_LIMIT{ 20 };
		u8 m_bindingsSize{0};
		int m_inputBindings[KEYBINDINGS_LIMIT]; // bind ENUM INTs to SDL_SCANCODEs
		int m_inputStates[KEYBINDINGS_LIMIT]; // bind ENUM INTS to InputState vals

		enum InputState
		{
			INPUT_INACTIVE,
			INPUT_START,
			INPUT_ACTIVE,
			INPUT_RELEASED
		};
	};
};