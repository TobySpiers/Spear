#pragma once

struct Vector2D;

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
		Vector2D GetMousePos();

	private:
		void UpdateInputState(bool active, int* state);

		static const int KEYBINDINGS_LIMIT{ 20 };
		Vector2D m_mousePos{0.f, 0.f};
		int m_inputBindings[KEYBINDINGS_LIMIT]; // bind ENUM INTs to SDL_SCANCODEs
		int m_inputStates[KEYBINDINGS_LIMIT]; // bind ENUM INTS to InputState vals
		u8 m_bindingsSize{ 0 };

		enum InputState
		{
			INPUT_INACTIVE,
			INPUT_START,
			INPUT_ACTIVE,
			INPUT_RELEASED
		};
	};
};