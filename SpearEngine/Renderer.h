#pragma once

namespace Spear
{
	class Renderer
	{
		NO_COPY(Renderer);

	public:
		Renderer();
		void Render();

	private:
		GLuint m_vertexArray{ 0 };
		GLuint m_shaderProgram{ 0 };

		GLuint m_vertexBufferPos{ 0 };
		GLuint m_vertexBufferCol{ 0 };
		GLuint m_indexBuffer{0};
	};
}