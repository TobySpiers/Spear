#include "Core.h"
#include "FlowstateManager.h"
#include "SDLManager.h"
#include "InputManager.h"

#include "ServiceLocator.h"

namespace Spear
{
	static FlowstateManager* s_pFlowstateManager{nullptr};
	static SDLManager* s_pSdlManager{nullptr};
	static InputManager* s_pInputManager{nullptr};

	void ServiceLocator::Initialise()
	{
		if (!s_pFlowstateManager)
		{
			s_pFlowstateManager = new FlowstateManager;
		}

		if (!s_pSdlManager)
		{
			s_pSdlManager = new SDLManager;
		}

		if (!s_pInputManager)
		{
			s_pInputManager = new InputManager;
		}
	}

	void ServiceLocator::Shutdown()
	{
		delete s_pFlowstateManager;
		s_pFlowstateManager = nullptr;

		delete s_pSdlManager;
		s_pSdlManager = nullptr;

		delete s_pInputManager;
		s_pInputManager = nullptr;
	}

	FlowstateManager& ServiceLocator::GetFlowstateManager()
	{
		ASSERT(s_pFlowstateManager);
		return *s_pFlowstateManager;
	}

	SDLManager& ServiceLocator::GetSDLManager()
	{
		ASSERT(s_pSdlManager);
		return *s_pSdlManager;
	}

	InputManager& ServiceLocator::GetInputManager()
	{
		ASSERT(s_pInputManager);
		return *s_pInputManager;
	}
}