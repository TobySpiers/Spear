#include "Core.h"
#include "ServiceLocator.h"
#include "FlowstateManager.h"
#include "InputManager.h"
#include "WindowManager.h"
#include "Audio/AudioManager.h"
#include "ImguiManager.h"
#include "GameObject/GameObject.h"
#include "SDL_Image.h"
#include "imgui.h"

#if _DEBUG
#include "FrameProfiler.h"
#endif
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"

extern "C"
{
	// External Nvidia/AMD flags to request dedicated GPU on laptops/machines with an iGPU (integrated) and dGPU (dedicated)
	_declspec(dllexport) unsigned NvOptimusEnablement = 1;
	_declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

namespace Spear
{
	bool Core::m_shutdown{false};
	WindowParams Core::m_windowParams;

	void GLClearErrors()
	{
		while (GLenum error = glGetError()) {}
	}
	void GLPrintErrors(const char* file, const char* function, int line)
	{
		while (GLenum error = glGetError())
		{
			std::cout << "OpenGL Error:"
				<< "\n\tFile: " << file
				<< "\n\tLine: " << line
				<< "\n\tFunction: " << function
				<< "\n\tError Code: " << error
				<< std::endl;
		}
	}

	void Core::Initialise(const WindowParams& params)
	{
		// Store window params
		m_windowParams = params;

		// SDL setup
		if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
		{
			LOG("SDL failed to initialise...");
		}

		// SDL_Image init (#ToDo: move to some type of LoadedTextures class)
		if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG)
		{
			LOG("SDL_Image failed to initialise..");
		}

		// Specify our OpenGL version: version 4.3 for compute shader support, profile mask = core profile (no backward compat)
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

		// Initialise all services
		ServiceLocator::Initialise(params);
	}

	void Core::RunGameloop(int targetFPS)
	{		
		InputManager& inputManager = ServiceLocator::GetInputManager();
		FlowstateManager& stateManager = ServiceLocator::GetFlowstateManager();
		AudioManager& audioManager = ServiceLocator::GetAudioManager();
		ImguiManager& imguiManager = ServiceLocator::GetImguiManager();

		u64 frameStart;
		float deltaTime{1.f / targetFPS };
		const u64 targetFrequency = SDL_GetPerformanceFrequency()/targetFPS;

		ImGuiIO& imguiIO = ImGui::GetIO();
		while (!m_shutdown)
		{
			frameStart = SDL_GetPerformanceCounter();
			#if _DEBUG
			FrameProfiler::StartFrame(frameStart);
			#endif

			// Handle SDL events
			SDL_Event event;
			int mousewheelInput{ 0 };
			while(SDL_PollEvent(&event)) // get any pending event
			{
				ImGui_ImplSDL2_ProcessEvent(&event);

				switch (event.type)
				{
				case SDL_QUIT:
					SignalShutdown();
					break;

				case SDL_MOUSEWHEEL:
					if (event.wheel.y != 0)
					{
						mousewheelInput = (event.wheel.y > 0 ? 1 : -1);
					}
					break;
				}
			}

			START_PROFILE("UPDATE");
			// Refresh input data
			inputManager.RefreshInput(mousewheelInput);

			// State: update
			stateManager.Update(deltaTime);

			// GameObjects: update
			GameObject::GlobalTick(deltaTime);

			// Audio: update
			audioManager.UpdatePlayingStreams();
			END_PROFILE("UPDATE");

			START_PROFILE("DRAW");
			// GameObjects: render
			GameObject::GlobalDraw();

			// State: render
			stateManager.Render();
			END_PROFILE("DRAW");

			// ImGui: update & render
			imguiManager.MakePanels();

			// Swap buffers
			SDL_GL_SwapWindow(&ServiceLocator::GetWindowManager().GetWindow());
			glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

			// spinlock to keep thread active while waiting
			while(SDL_GetPerformanceCounter() - frameStart < targetFrequency)
			{}
			deltaTime = static_cast<float>(SDL_GetPerformanceCounter() - frameStart) / SDL_GetPerformanceFrequency();
		}
	}

	void Core::SignalShutdown()
	{
		m_shutdown = true;
	}

	void Core::Cleanup()
	{
		// Shutdown SpearEngine services
		GameObject::GlobalDestroy();
		ServiceLocator::Shutdown();
		IMG_Quit();
		SDL_Quit();
	}

	Vector2i Core::GetWindowSize()
	{
		return ServiceLocator::GetWindowManager().GetWindowSize();
	}
	float Core::GetWindowScale()
	{
		return m_windowParams.scale;
	}

	// Convert WindowCoordinate (0,0 : x,x > BottomLeft : TopRight) to DeviceCoordinate (-1,-1 : 1, 1 > BottomLeft : TopRight)
	Vector2f Core::GetNormalizedDeviceCoordinate(const Vector2f& inCoord)
	{
		return GetNormalizedDeviceCoordinate(inCoord, Vector2f(m_windowParams.width, m_windowParams.height));
	}

	Vector2f Core::GetNormalizedDeviceCoordinate(const Vector2f& inCoord, const Vector2f& viewport)
	{
		return Vector2f(
			-1 + (2 * (inCoord.x / viewport.x)),
			1 - (2 * (inCoord.y / viewport.y))
		);
	}
}
