#include "Core.h"
#include "ServiceLocator.h"
#include "FlowstateManager.h"
#include "SDLManager.h"
#include "InputManager.h"

namespace Spear
{
	bool Core::m_shutdown{false};
	SDL_Window* Core::m_window{nullptr};
	SDL_Renderer* Core::m_renderer{nullptr};

	void Core::Initialise(const WindowParams& params)
	{
		// Initialise SpearEngine services
		ServiceLocator::Initialise();

		// Create window managed by ServiceLocator
		SDLManager& sdlManager = ServiceLocator::GetSDLManager();
		sdlManager.CreateWindow(params);
	}

	void Core::Cleanup()
	{
		// Shutdown SpearEngine services
		ServiceLocator::Shutdown();
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
}