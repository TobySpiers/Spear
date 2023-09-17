#include "Core.h"
#include "SDLManager.h"

namespace Spear
{
	SDLManager::SDLManager()
	{
		
	}

	void SDLManager::CreateWindow(const WindowParams& params)
	{
		// SDL setup
		if (SDL_Init(SDL_INIT_EVERYTHING) == 0) // 0 = no error returned
		{
			std::cout << "SDL Initialised" << std::endl;

			int windowFlags = params.fullscreen ? SDL_WINDOW_FULLSCREEN : 0;
			m_window = SDL_CreateWindow(params.title, params.xpos, params.ypos, params.width, params.height, windowFlags);
			ASSERT(m_window);

			m_renderer = SDL_CreateRenderer(m_window, -1, 0);
			ASSERT(m_renderer);

			SDL_RenderSetScale(m_renderer, params.scale, params.scale);
		}
	}

	SDLManager::~SDLManager()
	{
		// Shutdown SDL
		SDL_DestroyWindow(m_window);
		SDL_DestroyRenderer(m_renderer);
		SDL_Quit();
		std::cout << "SpearEngine::Core succesful shutdown" << std::endl;
	}

	SDL_Window& SDLManager::GetWindow()
	{
		ASSERT(m_window);
		return *m_window;
	}

	SDL_Renderer& SDLManager::GetRenderer()
	{
		ASSERT(m_renderer);
		return *m_renderer;
	}
}