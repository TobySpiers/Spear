#include "Core.h"
#include "FlowstateManager.h"
#include "WindowManager.h"
#include "InputManager.h"
#include "Renderer.h"
#include "ScreenRenderer.h"

#include "ServiceLocator.h"

namespace Spear
{
	static FlowstateManager* s_pFlowstateManager{nullptr};
	static WindowManager* s_pWindowManager{nullptr};
	static InputManager* s_pInputManager{nullptr};
	static Renderer* s_pRenderer{nullptr};
	static ScreenRenderer* s_pLineRenderer{nullptr};

	void ServiceLocator::Initialise(const WindowParams& params)
	{
		if (!s_pFlowstateManager)
		{
			s_pFlowstateManager = new FlowstateManager;
		}

		if (!s_pWindowManager)
		{
			s_pWindowManager = new WindowManager(params);
		}

		if (!s_pInputManager)
		{
			s_pInputManager = new InputManager;
		}

		if (!s_pRenderer)
		{
			s_pRenderer = new Renderer;
		}

		if (!s_pLineRenderer)
		{
			s_pLineRenderer = new ScreenRenderer;
		}
	}

	void ServiceLocator::Shutdown()
	{
		delete s_pFlowstateManager;
		s_pFlowstateManager = nullptr;

		delete s_pWindowManager;
		s_pWindowManager = nullptr;

		delete s_pInputManager;
		s_pInputManager = nullptr;

		delete s_pRenderer;
		s_pRenderer = nullptr;

		delete s_pLineRenderer;
		s_pLineRenderer = nullptr;
	}

	FlowstateManager& ServiceLocator::GetFlowstateManager()
	{
		ASSERT(s_pFlowstateManager);
		return *s_pFlowstateManager;
	}

	WindowManager& ServiceLocator::GetWindowManager()
	{
		ASSERT(s_pWindowManager);
		return *s_pWindowManager;
	}

	InputManager& ServiceLocator::GetInputManager()
	{
		ASSERT(s_pInputManager);
		return *s_pInputManager;
	}

	Renderer& ServiceLocator::GetRenderer()
	{
		ASSERT(s_pRenderer);
		return *s_pRenderer;
	}

	ScreenRenderer& ServiceLocator::GetLineRenderer()
	{
		ASSERT(s_pLineRenderer);
		return *s_pLineRenderer;
	}
}