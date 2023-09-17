#include "Core.h"
#include "ServiceLocator.h"
#include "FlowstateManager.h"
#include "SDLManager.h"

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
		Spear::ServiceLocator::Shutdown();
	}

	void Core::LaunchGameloop()
	{
		FlowstateManager& stateManager = ServiceLocator::GetFlowstateManager();
		while (!m_shutdown)
		{
			// Update state
			stateManager.Update();
		}
	}

	void Core::SignalShutdown()
	{
		m_shutdown = true;
	}
}