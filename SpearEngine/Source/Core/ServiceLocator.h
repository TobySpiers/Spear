#pragma once
namespace Collision
{
	class CollisionSystem2D;
}

namespace Spear
{
	class FlowstateManager;
	class WindowManager;
	class InputManager;
	class Renderer;
	class ThreadManager;
	class AudioManager;
	class ImguiManager;

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
		static Renderer& GetScreenRenderer();
		static ThreadManager& GetThreadManager();
		static AudioManager& GetAudioManager();
		static ImguiManager& GetImguiManager();
		static Collision::CollisionSystem2D& GetCollisionSystem();
	};
}