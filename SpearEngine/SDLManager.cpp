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
			LOG("SDL Initialised");

			// Specify our OpenGL version: version 4.1, core profile (no backward compat)
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

			// enable double-buffer to avoid screentearing, set depth buffer to 24 bits (common balance for precision/memory use)
			SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
			SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

			// Create window with OpenGL surface
			m_window = SDL_CreateWindow(params.title, params.xpos, params.ypos, params.width, params.height, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | (params.fullscreen ? SDL_WINDOW_FULLSCREEN : 0));
			ASSERT(m_window);

			// Context for working with OpenGL
			m_context = SDL_GL_CreateContext(m_window);

			// Setup OpenGL function pointers
			gladLoadGLLoader(SDL_GL_GetProcAddress);

			LOG("scale not yet configured for glViewport");
			glViewport(0, 0, params.width, params.height);
		}
	}

	SDLManager::~SDLManager()
	{
		// Shutdown SDL
		SDL_DestroyWindow(m_window);
		SDL_Quit();
		LOG("SDL shutdown");
	}

	SDL_Window& SDLManager::GetWindow()
	{
		ASSERT(m_window);
		return *m_window;
	}

	SDL_GLContext& SDLManager::GetContext()
	{
		ASSERT(m_context);
		return m_context;
	}
}