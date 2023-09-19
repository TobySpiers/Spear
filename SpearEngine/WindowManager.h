#pragma once
#include <SDL.h>

namespace Spear
{
	class WindowManager
	{
		NO_COPY(WindowManager);
	public:
		WindowManager(const WindowParams& params);
		~WindowManager();

		SDL_Window& GetWindow();
		SDL_GLContext& GetContext();

	private:
		SDL_Window* m_window{nullptr};
		SDL_GLContext m_context{nullptr};
	};
}
