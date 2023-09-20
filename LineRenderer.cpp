#include "Core.h"
#include "LineRenderer.h"
#include "ShaderCompiler.h"

namespace Spear
{

	LineRenderer::LineRenderer()
	{
		// CREATE VAO
		glGenVertexArrays(1, &m_vertexArrayObj);
		glBindVertexArray(m_vertexArrayObj);

		// SETUP POS BUFFER
		glGenBuffers(1, &m_instancePosBuffer);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(
			0,
			4, // 4 values (2 for start, 2 for end)
			GL_FLOAT,
			GL_FALSE,
			0,
			(GLvoid*)0
		);
		glVertexAttribDivisor(0, 1);

		// SETUP COL BUFFER
		glGenBuffers(1, &m_instanceColorBuffer);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(
			1,
			4, // 4 values per instance
			GL_FLOAT,
			GL_FALSE,
			0,
			(GLvoid*)0
		);
		glVertexAttribDivisor(1, 1);

		// COMPILE SHADER
		m_shaderProgram = ShaderCompiler::CreateShaderProgram("../Shaders/LineVS.glsl", "../Shaders/LineFS.glsl");
	}

	void LineRenderer::AddLine(const LineData& line)
	{
		if (m_lineCount >= LINE_MAX)
		{
			LOG("WARNING: LineRenderer line count exceeded! Skipping lines");
			return;
		}

		int posIndex{FLOATS_PER_POS * m_lineCount};
		m_instancePosData[posIndex + 0] = line.start.x;
		m_instancePosData[posIndex + 1] = line.start.y;
		m_instancePosData[posIndex + 2] = line.end.x;
		m_instancePosData[posIndex + 3] = line.end.y;

		int colIndex{FLOATS_PER_COLOR * m_lineCount};
		m_instanceColorData[posIndex + 0] = line.r;
		m_instanceColorData[posIndex + 1] = line.g;
		m_instanceColorData[posIndex + 2] = line.b;
		m_instanceColorData[posIndex + 3] = line.alpha;

		m_lineCount++;
	}

	void LineRenderer::Render()
	{
		// Enable blending for transparency... #Refactor?
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// CLEAR
		glClearColor(0.5f, 0.5f, 0.5f, 1.f);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		// SETUP DATA
		glUseProgram(m_shaderProgram);
		glBindVertexArray(m_vertexArrayObj);

		// UPLOAD POS DATA
		glBindBuffer(GL_ARRAY_BUFFER, m_instancePosBuffer);
		glBufferData(
			GL_ARRAY_BUFFER,
			INSTANCE_POS_MAX * sizeof(GLfloat),
			m_instancePosData,
			GL_STATIC_DRAW
		);

		// UPLOAD COL DATA
		glBindBuffer(GL_ARRAY_BUFFER, m_instanceColorBuffer);
		glBufferData(
			GL_ARRAY_BUFFER,
			INSTANCE_COL_MAX * sizeof(GLfloat),
			m_instanceColorData,
			GL_STATIC_DRAW
		);

		//glEnable(GL_LINE_SMOOTH);
		//glLineWidth(10.0f);

		GLCheck(glDrawArraysInstanced(GL_LINES, 0, 2, m_lineCount)); // 2 vertices per instance, 3 instances
		m_lineCount = 0;
	}
}