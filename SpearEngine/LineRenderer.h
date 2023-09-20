#pragma once

namespace Spear
{

	class LineRenderer
	{
		NO_COPY(LineRenderer);

	public:
		LineRenderer();
		void Render();

	private:
		GLuint m_vertexArrayObj{0};
		GLuint m_shaderProgram{0};

		GLuint m_vertexPosBuffer{0};
		GLuint m_vertexColBuffer{0};
	};

}