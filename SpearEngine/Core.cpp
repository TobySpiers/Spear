#include "Core.h"
#include "ServiceLocator.h"
#include "FlowstateManager.h"
#include "InputManager.h"
#include "WindowManager.h"
#include "AudioManager.h"
#include "GameObject.h"
#include "SDL_Image.h"

#if _DEBUG
#include "FrameProfiler.h"
#endif

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

		// Specify our OpenGL version: version 4.1, profile mask = core profile (no backward compat)
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

		// Initialise all services
		ServiceLocator::Initialise(params);
	}

	void Core::RunGameloop(int targetFPS)
	{		
		InputManager& inputManager = ServiceLocator::GetInputManager();
		FlowstateManager& stateManager = ServiceLocator::GetFlowstateManager();
		AudioManager& audioManager = ServiceLocator::GetAudioManager();

		#if _DEBUG
		FrameProfiler::Initialise();
		#endif

		u64 frameStart;
		float deltaTime{1.f / targetFPS };
		const u64 targetFrequency = SDL_GetPerformanceFrequency()/targetFPS;
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
				switch (event.type)
				{
				case SDL_QUIT:
					SignalShutdown();
					break;

				case SDL_MOUSEWHEEL:
					if (event.wheel.y > 0)
					{
						mousewheelInput = 1;
					}
					else if (event.wheel.y < 0)
					{
						mousewheelInput = -1;
					}
					break;
				}
			}

			// Refresh input data
			inputManager.RefreshInput(mousewheelInput);

			// State: update
			stateManager.Update(deltaTime);

			// GameObjects: update
			GameObject::GlobalTick(deltaTime);

			// Audio: update
			audioManager.UpdateStreamingSounds();

			// GameObjects: render
			GameObject::GlobalDraw();

			// State: render
			stateManager.Render();

			// Swap buffers
			SDL_GL_SwapWindow(&ServiceLocator::GetWindowManager().GetWindow());
			glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

			#if _DEBUG
			FrameProfiler::EndFrame(SDL_GetPerformanceCounter());
			#endif

			// spinlock to keep thread active while waiting
			while(SDL_GetPerformanceCounter() - frameStart < targetFrequency)
			{}
			deltaTime = static_cast<float>(SDL_GetPerformanceCounter() - frameStart) / SDL_GetPerformanceFrequency();
		}

		#if _DEBUG
		FrameProfiler::SaveProfile("profile.csv");
		#endif
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
		return Vector2i(m_windowParams.width, m_windowParams.height);
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
