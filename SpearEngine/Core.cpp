#include "Core.h"
#include "ServiceLocator.h"
#include "FlowstateManager.h"
#include "WindowManager.h"
#include "InputManager.h"
#include "SDL_Image.h"

namespace Spear
{
	bool Core::m_shutdown{false};
	SDL_Window* Core::m_window{nullptr};
	SDL_Renderer* Core::m_renderer{nullptr};

	void Core::Initialise(const WindowParams& params)
	{
		// SDL setup
		if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
		{
			LOG("SDL failed to initialise...");
		}

		// SDL_Image init (#ToDo: move to some type of LoadedTextures class)
		if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG)
		{
			LOG("SDL_Image failed to initialise..");
		}

		// Specify our OpenGL version: version 4.1, profile mask = core profile (no backward compat)
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

		// enable double-buffer to avoid screentearing, set depth buffer to 24 bits (common balance for precision/memory use)
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

		// Initialise all services
		ServiceLocator::Initialise(params);
	}

	void Core::Cleanup()
	{
		// Shutdown SpearEngine services
		ServiceLocator::Shutdown();
		IMG_Quit();
		SDL_Quit();
	}

	void Core::RunGameloop(int targetFPS)
	{		
		InputManager& inputManager = ServiceLocator::GetInputManager();
		FlowstateManager& stateManager = ServiceLocator::GetFlowstateManager();

		u64 frameStart;
		float deltaTime{0.016f};
		const u64 targetFrequency = SDL_GetPerformanceFrequency()/targetFPS;
		while (!m_shutdown)
		{
			frameStart = SDL_GetPerformanceCounter();

			// Handle SDL events
			SDL_Event event;
			SDL_PollEvent(&event); // get any pending event
			switch (event.type)
			{
			case SDL_QUIT:
				SignalShutdown();
				break;
			}

			// Refresh input data
			inputManager.RefreshInput();

			// Update state
			stateManager.Update(deltaTime);

			// spinlock to keep thread active while waiting
			while(SDL_GetPerformanceCounter() - frameStart < targetFrequency)
			{}
			deltaTime = static_cast<float>(SDL_GetPerformanceCounter() - frameStart) / SDL_GetPerformanceFrequency();
		}
	}

	void Core::SignalShutdown()
	{
		m_shutdown = true;
	}


	void GLClearErrors()
	{
		while(GLenum error = glGetError()){}
	}
	void GLPrintErrors(const char* file, const char* function, int line)
	{
		while (GLenum error = glGetError())
		{
			std::cout << "OpenGL Error:"
				<< "\n\tFile: " << file
				<< "\n\tLine: " << line
				<< "\n\tFunction: " << function
				<< "\n\tError Code: " << error
				<< std::endl;
		}
	}
}
