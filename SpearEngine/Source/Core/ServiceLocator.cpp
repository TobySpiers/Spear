#include "Core.h"
#include "FlowstateManager.h"
#include "WindowManager.h"
#include "InputManager.h"
#include "ThreadManager.h"
#include "ImguiManager.h"
#include "Graphics/ScreenRenderer.h"
#include "Audio/AudioManager.h"
#include "Collision/CollisionSystem.h"

#include "ServiceLocator.h"

namespace Spear
{
	static FlowstateManager* s_pFlowstateManager{nullptr};
	static WindowManager* s_pWindowManager{nullptr};
	static InputManager* s_pInputManager{nullptr};
	static Renderer* s_pScreenRenderer{nullptr};
	static ThreadManager* s_pThreadManager{nullptr};
	static AudioManager* s_pAudioManager{nullptr};
	static ImguiManager* s_pImguiManager{ nullptr };
	static Collision::CollisionSystem2D* s_pCollisionSystem{nullptr};

	void ServiceLocator::Initialise(const WindowParams& params)
	{
		if (!s_pWindowManager)
		{
			s_pWindowManager = new WindowManager(params);
		}

		if (!s_pImguiManager)
		{
			s_pImguiManager = new ImguiManager;
		}

		if (!s_pFlowstateManager)
		{
			s_pFlowstateManager = new FlowstateManager;
		}

		if (!s_pInputManager)
		{
			s_pInputManager = new InputManager;
		}

		if (!s_pScreenRenderer)
		{
			s_pScreenRenderer = new Renderer;
		}

		if (!s_pThreadManager)
		{
			s_pThreadManager = new ThreadManager;
		}

		if (!s_pAudioManager)
		{
			s_pAudioManager = new AudioManager;
			s_pAudioManager->OnCreated();
		}

		if (!s_pCollisionSystem)
		{
			s_pCollisionSystem = new Collision::CollisionSystem2D;
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

		delete s_pScreenRenderer;
		s_pScreenRenderer = nullptr;

		delete s_pThreadManager;
		s_pThreadManager = nullptr;

		delete s_pAudioManager;
		s_pAudioManager = nullptr;

		delete s_pImguiManager;
		s_pImguiManager = nullptr;

		delete s_pCollisionSystem;
		s_pCollisionSystem = nullptr;
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

	Renderer& ServiceLocator::GetScreenRenderer()
	{
		ASSERT(s_pScreenRenderer);
		return *s_pScreenRenderer;
	}

	ThreadManager& ServiceLocator::GetThreadManager()
	{
		ASSERT(s_pThreadManager);
		return *s_pThreadManager;
	}

	AudioManager& ServiceLocator::GetAudioManager()
	{
		ASSERT(s_pAudioManager);
		return *s_pAudioManager;
	}
	ImguiManager& ServiceLocator::GetImguiManager()
	{
		ASSERT(s_pImguiManager);
		return *s_pImguiManager;
	}
	Collision::CollisionSystem2D& ServiceLocator::GetCollisionSystem()
	{
		ASSERT(s_pCollisionSystem);
		return *s_pCollisionSystem;
	}
}