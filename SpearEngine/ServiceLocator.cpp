#include "Core.h"
#include "FlowstateManager.h"
#include "SDLManager.h"

#include "ServiceLocator.h"

namespace Spear
{
	static FlowstateManager* s_pFlowstateManager{nullptr};
	static SDLManager* s_pSdlManager{nullptr};

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
	}

	void ServiceLocator::Shutdown()
	{
		delete s_pFlowstateManager;
		s_pFlowstateManager = nullptr;

		delete s_pSdlManager;
		s_pSdlManager = nullptr;
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
}