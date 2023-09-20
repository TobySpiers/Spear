#pragma once

namespace Spear
{
	constexpr int LINE_MAX{ 800 };
	constexpr int FLOATS_PER_POS{4};
	constexpr int FLOATS_PER_COLOR{4};
	constexpr int INSTANCE_POS_MAX{FLOATS_PER_POS * LINE_MAX};
	constexpr int INSTANCE_COL_MAX{FLOATS_PER_COLOR * LINE_MAX};

	struct LineData
	{
		Vector2D start{0.f, 0.f};
		Vector2D end{0.f, 0.f};
		float r{0.f};
		float g{0.f};
		float b{0.f};
		float alpha{1.f};
	};

	class LineRenderer
	{
		NO_COPY(LineRenderer);
		

	public:
		LineRenderer();
		void AddLine(const LineData& line);
		void Render();

	private:
		int PosDataSize(){return sizeof(GLfloat) * (m_lineCount * FLOATS_PER_POS);};
		int ColDataSize(){return sizeof(GLfloat) * (m_lineCount * FLOATS_PER_COLOR);};

		// line data
		GLfloat m_instancePosData[INSTANCE_POS_MAX];
		GLfloat m_instanceColorData[INSTANCE_COL_MAX];
		int m_lineCount{0};

		// render data
		GLuint m_vertexArrayObj{0};
		GLuint m_instancePosBuffer{0};
		GLuint m_instanceColorBuffer{0};
		GLuint m_shaderProgram{ 0 };
	};

}