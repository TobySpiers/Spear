#include "Core.h"
#include "ScreenRenderer.h"
#include "ShaderCompiler.h"

namespace Spear
{

	ScreenRenderer::ScreenRenderer()
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

		// + POS ATTRIB
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

		// + COL ATTRIB
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(
			1,
			4, // 4 values (rgba)
			GL_FLOAT,
			GL_FALSE,
			0,
			(GLvoid*)0
		);
		glVertexAttribDivisor(1, 1);

		// SETUP UV BUFFER
		glGenBuffers(2, &m_instanceUVBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_instanceUVBuffer);
		glBufferData(
			GL_ARRAY_BUFFER,
			LINE_MAX * sizeof(GLfloat),
			nullptr,
			GL_STATIC_DRAW
		);

		// + UV ATTRIB
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(
			2,
			1, // 1 value per instance (uv X pos)
			GL_FLOAT,
			GL_FALSE,
			0,
			(GLvoid*)0
		);
		glVertexAttribDivisor(2, 1);

		// COMPILE SHADER
		m_shaderProgram = ShaderCompiler::CreateShaderProgram("../Shaders/LineVS.glsl", "../Shaders/LineFS.glsl");

		// TEMP: LOAD TEXTURE
//		m_texture.SetDataFromFile("../Assets/wall8.png");

		// testing custom data (2x2 RGB)
		GLfloat rawTex[2 * 2 * 3] = {
			1, 1, 1,	0, 0, 0,
			1, 0, 1,	0, 1, 0
		};
		m_texture.SetDataFromArrayRGB(rawTex, 2, 2);

		LOG("LOG: Line renderer reserved " << sizeof(GLfloat) * (INSTANCE_COL_MAX + INSTANCE_POS_MAX + INSTANCE_UV_MAX) << " bytes in CPU/GPU memory");
	}

	void ScreenRenderer::AddLine(const LineData& line)
	{
		if (m_lineCount >= LINE_MAX)
		{
			LOG("WARNING: ScreenRenderer line count exceeded! Skipping lines");
			return;
		}

		const Vector2f startPos{Core::GetNormalizedDeviceCoordinate(line.start)};
		const Vector2f endPos{Core::GetNormalizedDeviceCoordinate(line.end)};

		int posIndex{FLOATS_PER_POS * m_lineCount};
		m_instancePosData[posIndex + 0] = startPos.x;
		m_instancePosData[posIndex + 1] = startPos.y;
		m_instancePosData[posIndex + 2] = endPos.x;
		m_instancePosData[posIndex + 3] = endPos.y;

		int colIndex{FLOATS_PER_COLOR * m_lineCount};
		m_instanceColorData[colIndex + 0] = line.colour.r;
		m_instanceColorData[colIndex + 1] = line.colour.g;
		m_instanceColorData[colIndex + 2] = line.colour.b;
		m_instanceColorData[colIndex + 3] = line.colour.a;

		int uvIndex{m_lineCount};
		m_instanceUVData[uvIndex] = line.texPosX;

		m_lineCount++;
	}

	void ScreenRenderer::AddLinePoly(const LinePolyData& poly)
	{
		// line, triangle, square, or bigger
		ASSERT(poly.segments > 1);

		// decompose into individual lines
		// not the most efficient approach memory-wise as vertices get duplicated
		// but line-polys are not the main use case and there will never be very many at once
		float increment {(PI * 2) / poly.segments};
		for (int i = 0; i < poly.segments; i++)
		{
			int nextIndex{ (i == poly.segments - 1) ? 0 : i + 1 };

			LineData line;
			line.start = poly.pos + Vector2f(cos(poly.rotation + (increment * i)), sin(poly.rotation + (increment * i))) * poly.radius;
			line.end = poly.pos + Vector2f(cos(poly.rotation + (increment * nextIndex)), sin(poly.rotation + (increment * nextIndex))) * poly.radius;
			line.colour = poly.colour;

			AddLine(line);
		}
	}

	void ScreenRenderer::Render()
	{
		// Enable blending for transparency... #Refactor?
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// SETUP VAO
		glUseProgram(m_shaderProgram);
		glBindVertexArray(m_vertexArrayObj);

		// UPLOAD POS DATA
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

		// UPLOAD UV DATA
		glBindBuffer(GL_ARRAY_BUFFER, m_instanceUVBuffer);
		glBufferSubData(
			GL_ARRAY_BUFFER,
			0,
			UVDataSize(),
			m_instanceUVData
		);

		// BIND TEXTURE
		glBindTexture(GL_TEXTURE_2D, m_texture.GetTextureId());

		// UPLOAD CONSTANT DATA
		GLint texLoc = glGetUniformLocation(m_shaderProgram, "texture");
		glUniform1i(texLoc, 0);
		GLint widthLoc = glGetUniformLocation(m_shaderProgram, "lineWidth");
		glUniform2f(widthLoc, m_lineWidth / Core::GetWindowSize().x, m_lineWidth / Core::GetWindowSize().y);
		
		// RENDER
		GLCheck(glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, m_lineCount)); // 4 vertices per instance, m_lineCount instances
		m_lineCount = 0;
	}
}