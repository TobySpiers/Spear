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

		static WindowManager& Get();

		SDL_Window& GetWindow();
		SDL_GLContext& GetContext();

		void SetWindowSize(float x, float y);
		void SetWindowSize(Vector2i size);
		void SetWindowFullscreenMode(eFullscreenMode mode);
		void SetWindowTitleOverride(const char* title);

		Vector2i GetWindowSize();

	private:
		SDL_Window* m_window{nullptr};
		SDL_GLContext m_context{nullptr};
		const char* m_defaultWindowTitle{ nullptr };
	};
}
