#pragma once
#include <SDL.h>

namespace Spear
{
	class GfxManager
	{
		NO_COPY(GfxManager);
	public:
		GfxManager();
		~GfxManager();

		void CreateWindow(const WindowParams& params);

		SDL_Window& GetWindow();
		SDL_GLContext& GetContext();

		// temp function
		GLuint GetPipeline(){return m_renderPipeline;};
		GLuint GetVertexArray(){return m_vertexArray;};
		GLuint GetVertexBuffer(){return m_vertexBuffer;};

	private:
		SDL_Window* m_window{nullptr};
		SDL_GLContext m_context;

		// temp: move into own render class
		GLuint m_vertexArray{0};
		GLuint m_vertexBuffer{0};
		GLuint m_renderPipeline{0};
	};
}
