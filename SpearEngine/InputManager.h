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
		void RefreshInput(int mousewheelInput);

		bool InputStart(int key);
		bool InputHold(int key);
		bool InputRelease(int key);

		bool ClickStart() {return (m_clickLeft == INPUT_START); };
		bool ClickHold() {return (m_clickLeft == INPUT_START || m_clickLeft == INPUT_ACTIVE); };
		bool ClickRelease() {return (m_clickLeft == INPUT_RELEASED); };

		bool RightClickStart() { return (m_clickRight == INPUT_START); };
		bool RightClickHold() { return (m_clickRight == INPUT_START || m_clickRight == INPUT_ACTIVE); };
		bool RightClickRelease() { return (m_clickRight == INPUT_RELEASED); };

		int Wheel() { return m_mousewheelState; };
		bool WheelUp() { return m_mousewheelState > 0; };
		bool WheelDown() { return m_mousewheelState < 0; };

		Vector2i GetMousePos();		// exact position
		Vector2i GetMouseAxis();	// movement since last frame

	private:
		void UpdateInputState(bool active, int* state);

		enum InputState
		{
			INPUT_INACTIVE,
			INPUT_START,
			INPUT_ACTIVE,
			INPUT_RELEASED
		};

		static const int KEYBINDINGS_LIMIT{ 20 };
		Vector2i m_mousePos{0, 0};
		Vector2i m_mouseAxis{0, 0};
		int m_clickLeft{INPUT_INACTIVE};
		int m_clickRight{INPUT_INACTIVE};
		int m_mousewheelState{ 0 };

		int m_inputBindings[KEYBINDINGS_LIMIT] = {}; // bind ENUM INTs to SDL_SCANCODEs
		int m_inputStates[KEYBINDINGS_LIMIT] = {}; // bind ENUM INTS to InputState vals
		u8 m_bindingsSize{ 0 };

	};
};