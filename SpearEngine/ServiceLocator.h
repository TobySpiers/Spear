#pragma once
namespace Spear
{
	class FlowstateManager;
	class SDLManager;
	class InputManager;

	class ServiceLocator
	{
		// Mark as non-constructable, static functions/data only
		NO_CONSTRUCT(ServiceLocator);

		public:
		static void Initialise();
		static void Shutdown();

		static FlowstateManager& GetFlowstateManager();
		static SDLManager& GetSDLManager();
		static InputManager& GetInputManager();
	};
}