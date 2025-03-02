#include "Core.h"
#include "WindowManager.h"
#include "ServiceLocator.h"
#include "SDL_image.h"

namespace Spear
{
	WindowManager::WindowManager(const WindowParams& params)
	{
		// Create window with OpenGL surface
		const Uint32 windowFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL | (params.fullscreen ? SDL_WINDOW_FULLSCREEN : SDL_WINDOW_MAXIMIZED);
		m_window = SDL_CreateWindow(params.title, params.xpos, params.ypos, params.width * params.scale, params.height * params.scale, windowFlags);
		ASSERT(m_window);

		// Context for working with OpenGL
		m_context = SDL_GL_CreateContext(m_window);

		// Setup OpenGL function pointers
		if (!gladLoadGLLoader(SDL_GL_GetProcAddress))
		{
			LOG("ERROR: glad load failed");
		}

		glViewport(0, 0, params.width, params.height);

		std::cout << "\nOpenGL Initialised..."
					<< "\n\t Vendor: " << glGetString(GL_VENDOR)
					<< "\n\t GPU: " << glGetString(GL_RENDERER)
					<< "\n\t Version: " << glGetString(GL_VERSION)
					<< "\n\t Shading Language: " << glGetString(GL_SHADING_LANGUAGE_VERSION)
					<< std::endl;
		
		m_defaultWindowTitle = params.title;

		SDL_Surface* icon = IMG_Load("../Assets/icon.png");
		SDL_SetWindowIcon(m_window, icon);
		SDL_FreeSurface(icon);
	}

	WindowManager::~WindowManager()
	{
		// Shutdown OpenGL window
		SDL_GL_DeleteContext(m_context);
		SDL_DestroyWindow(m_window);
		LOG("Window shutdown...");
	}

	WindowManager& WindowManager::Get()
	{
		return ServiceLocator::GetWindowManager();
	}

	SDL_Window& WindowManager::GetWindow()
	{
		ASSERT(m_window);
		return *m_window;
	}

	SDL_GLContext& WindowManager::GetContext()
	{
		ASSERT(m_context);
		return m_context;
	}

	void WindowManager::SetWindowSize(float x, float y)
	{
		SDL_SetWindowSize(m_window, x, y);
	}

	void WindowManager::SetWindowSize(Vector2i size)
	{
		SDL_SetWindowSize(m_window, size.x, size.y);
	}

	void WindowManager::SetWindowFullscreenMode(eFullscreenMode mode)
	{
		SDL_SetWindowFullscreen(m_window, static_cast<Uint32>(mode));
	}

	void WindowManager::SetWindowTitleOverride(const char* title)
	{
		SDL_SetWindowTitle(m_window, title != nullptr ? title : m_defaultWindowTitle);
	}

	Vector2i WindowManager::GetWindowSize()
	{
		Vector2i result;
		SDL_GetWindowSize(m_window, &result.x, &result.y);
		return result;
	}
}