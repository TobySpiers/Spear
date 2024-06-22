#pragma once
namespace Spear
{
	class FlowstateManager;
	class WindowManager;
	class InputManager;
	class ScreenRenderer;
	class ThreadManager;

	class ServiceLocator
	{
		// Mark as non-constructable, static functions/data only
		NO_CONSTRUCT(ServiceLocator);

	public:
		static void Initialise(const WindowParams& params);
		static void Shutdown();

		static FlowstateManager& GetFlowstateManager();
		static WindowManager& GetWindowManager();
		static InputManager& GetInputManager();
		static ScreenRenderer& GetScreenRenderer();
		static ThreadManager& GetThreadManager();
	};
}