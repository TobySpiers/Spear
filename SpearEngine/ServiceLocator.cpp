#include "Core.h"
#include "FlowstateManager.h"
#include "GfxManager.h"
#include "InputManager.h"

#include "ServiceLocator.h"

namespace Spear
{
	static FlowstateManager* s_pFlowstateManager{nullptr};
	static GfxManager* s_pGfxManager{nullptr};
	static InputManager* s_pInputManager{nullptr};

	void ServiceLocator::Initialise()
	{
		if (!s_pFlowstateManager)
		{
			s_pFlowstateManager = new FlowstateManager;
		}

		if (!s_pGfxManager)
		{
			s_pGfxManager = new GfxManager;
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

		delete s_pGfxManager;
		s_pGfxManager = nullptr;

		delete s_pInputManager;
		s_pInputManager = nullptr;
	}

	FlowstateManager& ServiceLocator::GetFlowstateManager()
	{
		ASSERT(s_pFlowstateManager);
		return *s_pFlowstateManager;
	}

	GfxManager& ServiceLocator::GetGfxManager()
	{
		ASSERT(s_pGfxManager);
		return *s_pGfxManager;
	}

	InputManager& ServiceLocator::GetInputManager()
	{
		ASSERT(s_pInputManager);
		return *s_pInputManager;
	}
}