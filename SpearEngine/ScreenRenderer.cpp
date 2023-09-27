#include "Core.h"
#include "ScreenRenderer.h"
#include "ShaderCompiler.h"

namespace Spear
{

	ScreenRenderer::ScreenRenderer()
	{
		InitialiseLineBuffers();
		InitialiseSpriteBuffers();

		// COMPILE SHADERS
		m_lineShader = ShaderCompiler::CreateShaderProgram("../Shaders/LineVS.glsl", "../Shaders/LineFS.glsl");
		m_spriteShader = ShaderCompiler::CreateShaderProgram("../Shaders/SpriteVS.glsl", "../Shaders/SpriteFS.glsl");
		m_backgroundShader = ShaderCompiler::CreateShaderProgram("../Shaders/BackgroundVS.glsl", "../Shaders/BackgroundFS.glsl");

		LOG("LOG: Line renderer reserved " << sizeof(GLfloat) * (LINE_COL_MAXBYTES + LINE_POS_MAXBYTES + LINE_UV_MAXBYTES) << " bytes in CPU + GPU memory");
	}

	void ScreenRenderer::InitialiseLineBuffers()
	{
		// CREATE VAO
		glGenVertexArrays(1, &m_lineVAO);
		glBindVertexArray(m_lineVAO);

		// SETUP POS BUFFER
		glGenBuffers(1, &m_linePosBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_linePosBuffer);
		glBufferData(
			GL_ARRAY_BUFFER,
			LINE_POS_MAXBYTES * sizeof(GLfloat),
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
		glGenBuffers(1, &m_lineColorBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_lineColorBuffer);
		glBufferData(
			GL_ARRAY_BUFFER,
			LINE_COL_MAXBYTES * sizeof(GLfloat),
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
		glGenBuffers(2, &m_lineUVBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_lineUVBuffer);
		glBufferData(
			GL_ARRAY_BUFFER,
			LINE_UV_MAXBYTES * sizeof(GLfloat),
			nullptr,
			GL_STATIC_DRAW
		);

		// + UV ATTRIB
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(
			2,
			2, // 2 value per instance (uv X pos)
			GL_FLOAT,
			GL_FALSE,
			0,
			(GLvoid*)0
		);
		glVertexAttribDivisor(2, 1);
	}

	void ScreenRenderer::InitialiseSpriteBuffers()
	{
		// CREATE VAO
		glGenVertexArrays(1, &m_spriteVAO);
		glBindVertexArray(m_spriteVAO);

		// SETUP POS BUFFER
		glGenBuffers(1, &m_spritePosBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_spritePosBuffer);
		glBufferData(
			GL_ARRAY_BUFFER,
			SPRITE_POS_MAXBYTES * sizeof(GLfloat),
			nullptr,
			GL_STATIC_DRAW
		);

		// + POS ATTRIB
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(
			0,
			4, // 4 values (2 for xy, 2 for width/height)
			GL_FLOAT,
			GL_FALSE,
			0,
			(GLvoid*)0
		);
		glVertexAttribDivisor(0, 1);

		// SETUP COL BUFFER
		glGenBuffers(1, &m_spriteColBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_spriteColBuffer);
		glBufferData(
			GL_ARRAY_BUFFER,
			SPRITE_COL_MAXBYTES * sizeof(GLfloat),
			nullptr,
			GL_STATIC_DRAW
		);

		// + COL ATTRIB
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(
			1,
			2, // 2 values (x: texture depth, y: opacity)
			GL_FLOAT,
			GL_FALSE,
			0,
			(GLvoid*)0
		);
		glVertexAttribDivisor(1, 1);
	}

	void ScreenRenderer::SetBackgroundTextureData(GLfloat* pDataRGB, int width, int height)
	{
		m_backgroundTexture.SetDataFromArrayRGB(pDataRGB, width, height);
	}

	void ScreenRenderer::SetTextureArrayData(const TextureArray& textureArray)
	{
		m_pTextureArray = &textureArray;
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

		int posIndex{LINE_FLOATS_PER_POS * m_lineCount};
		m_linePosData[posIndex + 0] = startPos.x;
		m_linePosData[posIndex + 1] = startPos.y;
		m_linePosData[posIndex + 2] = endPos.x;
		m_linePosData[posIndex + 3] = endPos.y;

		int colIndex{LINE_FLOATS_PER_COLOR * m_lineCount};
		m_lineColorData[colIndex + 0] = line.colour.r;
		m_lineColorData[colIndex + 1] = line.colour.g;
		m_lineColorData[colIndex + 2] = line.colour.b;
		m_lineColorData[colIndex + 3] = line.colour.a;

		int uvIndex{LINE_FLOATS_PER_UV * m_lineCount};
		m_lineUVData[uvIndex + 0] = line.texPosX;
		m_lineUVData[uvIndex + 1] = line.texLayer;

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

	void ScreenRenderer::AddSprite(const SpriteData& sprite)
	{
		if (m_spriteCount >= SPRITE_MAX)
		{
			LOG("WARNING: ScreenRenderer sprite count exceeded! Skipping sprites");
			return;
		}

		const Vector2f screenPos{ Core::GetNormalizedDeviceCoordinate(sprite.pos) };

		int posIndex{ SPRITE_FLOATS_PER_POS * m_spriteCount };
		m_spritePosData[posIndex + 0] = screenPos.x;
		m_spritePosData[posIndex + 1] = screenPos.y;
		m_spritePosData[posIndex + 2] = sprite.scale.x;
		m_spritePosData[posIndex + 3] = sprite.scale.y;

		int colIndex{ SPRITE_FLOATS_PER_COLOR * m_spriteCount };
		m_spriteColData[colIndex + 0] = sprite.opacity;
		m_spriteColData[colIndex + 1] = sprite.textureSlot;

		m_spriteCount++;
	}

	void ScreenRenderer::Render()
	{
		// Enable blending for transparency... #Refactor?
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// DRAW BACKGROUND
		glUseProgram(m_backgroundShader);
		glBindTexture(GL_TEXTURE_2D, m_backgroundTexture.GetTextureId());
		GLCheck(glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, 1));

		RenderLines();

		RenderSprites();
	}

	void ScreenRenderer::RenderLines()
	{
		// SETUP VAO
		glUseProgram(m_lineShader);
		glBindVertexArray(m_lineVAO);

		// UPLOAD POS DATA
		glBindBuffer(GL_ARRAY_BUFFER, m_linePosBuffer);
		glBufferSubData(
			GL_ARRAY_BUFFER,
			0,
			LinePosSize(),
			m_linePosData
		);

		// UPLOAD COL DATA
		glBindBuffer(GL_ARRAY_BUFFER, m_lineColorBuffer);
		glBufferSubData(
			GL_ARRAY_BUFFER,
			0,
			LineColSize(),
			m_lineColorData
		);

		// UPLOAD UV DATA
		glBindBuffer(GL_ARRAY_BUFFER, m_lineUVBuffer);
		glBufferSubData(
			GL_ARRAY_BUFFER,
			0,
			LineUVSize(),
			m_lineUVData
		);

		// BIND TEXTURE
		glBindTexture(GL_TEXTURE_2D_ARRAY, m_pTextureArray->GetTextureId());

		// UPLOAD CONSTANT DATA
		GLint texLoc = glGetUniformLocation(m_lineShader, "textureSampler");
		glUniform1i(texLoc, 0);
		GLint widthLoc = glGetUniformLocation(m_lineShader, "lineWidth");
		glUniform2f(widthLoc, m_lineWidth / Core::GetWindowSize().x, m_lineWidth / Core::GetWindowSize().y);

		// RENDER
		GLCheck(glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, m_lineCount)); // 4 vertices per instance, m_lineCount instances
		m_lineCount = 0;
	}

	void ScreenRenderer::RenderSprites()
	{
		// Set sprite shader + VAO
		glUseProgram(m_spriteShader);
		glBindVertexArray(m_spriteVAO);

		// UPLOAD POS
		glBindBuffer(GL_ARRAY_BUFFER, m_spritePosBuffer);
		glBufferSubData(
			GL_ARRAY_BUFFER,
			0,
			SpritePosSize(),
			m_spritePosData
		);

		// UPLOAD COL
		glBindBuffer(GL_ARRAY_BUFFER, m_spriteColBuffer);
		glBufferSubData(
			GL_ARRAY_BUFFER,
			0,
			SpriteColSize(),
			m_spriteColData
		);

		// BIND TEXTURE
		glBindTexture(GL_TEXTURE_2D_ARRAY, m_pTextureArray->GetTextureId());

		// UPLOAD CONSTANT DATA
		GLint sizeLoc = glGetUniformLocation(m_spriteShader, "spriteSize");
		glUniform2f(
			sizeLoc, 
			(static_cast<float>(m_pTextureArray->GetWidth()) / Core::GetWindowSize().x),
			(static_cast<float>(m_pTextureArray->GetHeight()) / Core::GetWindowSize().y) 
		);

		// RENDER
		GLCheck(glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, m_spriteCount)); // 4 vertices per instance, m_spriteCount instances
		m_spriteCount = 0;
	}
}