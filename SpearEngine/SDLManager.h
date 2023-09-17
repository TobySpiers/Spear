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
		SDL_Renderer& GetRenderer();

	private:
		SDL_Window* m_window{nullptr};
		SDL_Renderer* m_renderer{nullptr};
	};
}
