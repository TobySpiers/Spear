#pragma once
#include <SDL.h>

namespace Spear
{
	enum class eFullscreenMode
	{
		WINDOWED = 0,
		FULLSCREEN = SDL_WINDOW_FULLSCREEN,
		BORDERLESS = SDL_WINDOW_FULLSCREEN_DESKTOP
	};

	class WindowManager
	{
		NO_COPY(WindowManager);
	public:
		WindowManager(const WindowParams& params);
		~WindowManager();

		SDL_Window& GetWindow();
		SDL_GLContext& GetContext();

		void SetWindowSize(float x, float y);
		void SetWindowFullscreenMode(eFullscreenMode mode);

	private:
		SDL_Window* m_window{nullptr};
		SDL_GLContext m_context{nullptr};
	};
}
