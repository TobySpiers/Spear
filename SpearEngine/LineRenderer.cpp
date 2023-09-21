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
		glBindBuffer(GL_ARRAY_BUFFER, m_instancePosBuffer);
		glBufferData(
			GL_ARRAY_BUFFER,
			INSTANCE_POS_MAX * sizeof(GLfloat),
			nullptr,
			GL_STATIC_DRAW
		);

		// SETUP POS ATTRIB
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
		glBindBuffer(GL_ARRAY_BUFFER, m_instanceColorBuffer);
		glBufferData(
			GL_ARRAY_BUFFER,
			INSTANCE_COL_MAX * sizeof(GLfloat),
			nullptr,
			GL_STATIC_DRAW
		);

		// SETUP COL ATTRIB
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

		LOG("LOG: Line renderer reserved " << sizeof(GLfloat) * (INSTANCE_COL_MAX + INSTANCE_POS_MAX) << " bytes in CPU/GPU memory");
	}

	void LineRenderer::AddLine(const LineData& line)
	{
		if (m_lineCount >= LINE_MAX)
		{
			LOG("WARNING: LineRenderer line count exceeded! Skipping lines");
			return;
		}

		const Vector2D startPos{Core::GetNormalizedDeviceCoordinate(line.start)};
		const Vector2D endPos{Core::GetNormalizedDeviceCoordinate(line.end)};

		int posIndex{FLOATS_PER_POS * m_lineCount};
		m_instancePosData[posIndex + 0] = startPos.x;
		m_instancePosData[posIndex + 1] = startPos.y;
		m_instancePosData[posIndex + 2] = endPos.x;
		m_instancePosData[posIndex + 3] = endPos.y;

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

		// SETUP VAO
		glUseProgram(m_shaderProgram);
		glBindVertexArray(m_vertexArrayObj);

		// REFRESH POS DATA
		glBindBuffer(GL_ARRAY_BUFFER, m_instancePosBuffer);
		glBufferSubData(
			GL_ARRAY_BUFFER,
			0,
			PosDataSize(),
			m_instancePosData
		);

		// UPLOAD COL DATA
		glBindBuffer(GL_ARRAY_BUFFER, m_instanceColorBuffer);
		glBufferSubData(
			GL_ARRAY_BUFFER,
			0,
			ColDataSize(),
			m_instanceColorData
		);

		//glEnable(GL_LINE_SMOOTH);
		glLineWidth(m_lineWidth);

		GLCheck(glDrawArraysInstanced(GL_LINES, 0, 2, m_lineCount)); // 2 vertices per instance, m_lineCount instances
		m_lineCount = 0;
	}
}