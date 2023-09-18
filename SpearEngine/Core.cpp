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
		FlowstateManager& stateManager = ServiceLocator::GetFlowstateManager();
		InputManager& inputManager = ServiceLocator::GetInputManager();

		u64 frameStart{SDL_GetTicks64()};
		u64 frameLength;
		while (!m_shutdown)
		{
			float deltaTime = static_cast<float>(SDL_GetTicks64() - frameStart) / 1000.f;
			frameStart = SDL_GetTicks64();

			// Refresh input data
			inputManager.RefreshInput();

			// Update state
			stateManager.Update(deltaTime);

			// Delay frame to match targetFPS if exceeded
			const int frameDelay{ 1000 / targetFPS };
			frameLength = SDL_GetTicks64() - frameStart;
			if (frameDelay > frameLength)
			{
				SDL_Delay(frameDelay - frameLength);
			}
		}
	}

	void Core::SignalShutdown()
	{
		m_shutdown = true;
	}
}