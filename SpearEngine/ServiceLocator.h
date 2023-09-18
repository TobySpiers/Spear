#pragma once
namespace Spear
{
	class FlowstateManager;
	class GfxManager;
	class InputManager;

	class ServiceLocator
	{
		// Mark as non-constructable, static functions/data only
		NO_CONSTRUCT(ServiceLocator);

		public:
		static void Initialise();
		static void Shutdown();

		static FlowstateManager& GetFlowstateManager();
		static GfxManager& GetGfxManager();
		static InputManager& GetInputManager();
	};
}