#include "Core.h"
#include "LineRenderer.h"
#include "ShaderCompiler.h"

namespace Spear
{

	LineRenderer::LineRenderer()
	{
		// temp for testing
		const std::vector<GLfloat> vertexPos{
			-0.5f, -1.f,	// line1 bottom
			-0.5f, 1.f,		// line1 top
			0.5f, -0.8f,	// line2 A
			0.3f, 0.7f		// line2 B
		};

		const std::vector<GLfloat> lineCol{
			0.f, 1.f, 1.f, 1.f,	// line a
			0.f, 1.f, 1.f, 1.f,	// line a
			1.f, 0.f, 0.f, 1.f,	// line b
			1.f, 0.f, 0.f, 1.f,	// line b
		};

		// Create VAO
		glGenVertexArrays(1, &m_vertexArrayObj);
		glBindVertexArray(m_vertexArrayObj);

		// Pos: Create Buffer
		glGenBuffers(1, &m_vertexPosBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_vertexPosBuffer);
		glBufferData(
			GL_ARRAY_BUFFER,
			vertexPos.size() * sizeof(GLfloat),
			vertexPos.data(),
			GL_STATIC_DRAW
		);

		// Pos: Attrib Array
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(
			0,
			2, // 2 values per vertex
			GL_FLOAT,
			GL_FALSE,
			0,
			(GLvoid*)0
		);

		// Col: Create Buffer
		glGenBuffers(1, &m_vertexColBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_vertexColBuffer);
		glBufferData(
			GL_ARRAY_BUFFER,
			lineCol.size() * sizeof(GLfloat),
			lineCol.data(),
			GL_STATIC_DRAW
		);

		// Col: Attrib Array
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(
			1,
			4,
			GL_FLOAT,
			GL_FALSE,
			0,
			(GLvoid*)0
		);

		m_shaderProgram = ShaderCompiler::CreateShaderProgram("../Shaders/LineVS.glsl", "../Shaders/LineFS.glsl");
	}

	void LineRenderer::Render()
	{
		// CLEAR
		glClearColor(0.5f, 0.5f, 0.5f, 1.f);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		// RENDER
		glUseProgram(m_shaderProgram);
		glBindVertexArray(m_vertexArrayObj);

		// Pass Uniform into shader: line width
		//glUniform1f(glGetUniformLocation(m_shaderProgram, "lineWidth"), 2.0f);
		
		//glEnable(GL_LINE_SMOOTH);
		//glLineWidth(10.0f);

		glDrawArrays(GL_LINES, 0, 4); // start from 0, draw 4 vertices

	}

}