#include "Core.h"
#include "LineRenderer.h"
#include "ShaderCompiler.h"

namespace Spear
{

	LineRenderer::LineRenderer()
	{
		const std::vector<GLfloat> instancePosData{
			-0.5f, -1.f,	// line1 bottom
			-0.5f, 1.f,		// line1 top
			-0.5f, 0.1f,	// line2 A
			0.5f, 0.1f,		// line2 B			
			0.f, -0.5f,		// line3 A
			0.f, 0.7f		// line3 B
		};

		const std::vector<GLfloat> instanceColorData{
			1.f, 0.f, 0.f, 1.f,	// line 1
			0.f, 1.f, 0.f, 0.8f,	// line 2
			0.f, 0.f, 1.f, 1.f, // line 3
		};


		// Create VAO
		glGenVertexArrays(1, &m_vertexArrayObj);
		glBindVertexArray(m_vertexArrayObj);

		// Instanced Pos
		glGenBuffers(1, &m_instancePosBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_instancePosBuffer);
		glBufferData(
			GL_ARRAY_BUFFER,
			instancePosData.size() * sizeof(GLfloat),
			instancePosData.data(),
			GL_STATIC_DRAW
		);

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

		// Instanced Col
		glGenBuffers(1, &m_instanceColorBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_instanceColorBuffer);
		glBufferData(
			GL_ARRAY_BUFFER,
			instanceColorData.size() * sizeof(GLfloat),
			instanceColorData.data(),
			GL_STATIC_DRAW
		);

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

		m_shaderProgram = ShaderCompiler::CreateShaderProgram("../Shaders/LineVS.glsl", "../Shaders/LineFS.glsl");
	}

	void LineRenderer::Render()
	{
		// Enable blending for transparency... #Refactor?
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// CLEAR
		glClearColor(0.5f, 0.5f, 0.5f, 1.f);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		// RENDER
		glUseProgram(m_shaderProgram);
		glBindVertexArray(m_vertexArrayObj);
		
		glEnable(GL_LINE_SMOOTH);
		//glLineWidth(10.0f);

		glDrawArraysInstanced(GL_LINES, 0, 2, 3); // 2 vertices per instance, 3 instances

	}

}