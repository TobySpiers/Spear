#pragma once
#include <SDL.h>

namespace Spear
{
	class SDLManager
	{
		NO_COPY(SDLManager);
	public:
		SDLManager();
		~SDLManager();

		void CreateWindow(const WindowParams& params);

		SDL_Window& GetWindow();
		SDL_GLContext& GetContext();

	private:
		SDL_Window* m_window{nullptr};
		SDL_GLContext m_context;
	};
}
