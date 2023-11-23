#include "Core.h"
#include "ScreenRenderer.h"
#include "ShaderCompiler.h"
#include "TextureFont.h"

namespace Spear
{

	ScreenRenderer::ScreenRenderer()
	{
		InitialiseRawLineBuffers();
		InitialiseTexturedLineBuffers();
		InitialiseSpriteBuffers();

		// COMPILE SHADERS
		m_lineShaderTextured = ShaderCompiler::CreateShaderProgram("../Shaders/TexturedLineVS.glsl", "../Shaders/TexturedLineFS.glsl");
		m_lineShaderColour = ShaderCompiler::CreateShaderProgram("../Shaders/LineVS.glsl", "../Shaders/LineFS.glsl");
		m_spriteShader = ShaderCompiler::CreateShaderProgram("../Shaders/SpriteVS.glsl", "../Shaders/SpriteFS.glsl");
		m_backgroundShader = ShaderCompiler::CreateShaderProgram("../Shaders/BackgroundVS.glsl", "../Shaders/BackgroundFS.glsl");

		LOG("LOG: Line Rendering reserved " << sizeof(GLfloat) * (LINE_POS_MAXBYTES + LINE_UV_MAXBYTES) << " bytes in CPU + GPU memory");
	}

	void ScreenRenderer::InitialiseRawLineBuffers()
	{
		// CREATE VAO
		glGenVertexArrays(1, &m_rawlineVAO);
		glBindVertexArray(m_rawlineVAO);

		// SETUP POS BUFFER
		glGenBuffers(1, &m_rawlinePosBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_rawlinePosBuffer);
		glBufferData(
			GL_ARRAY_BUFFER,
			RAWLINE_POS_MAXBYTES * sizeof(GLfloat),
			nullptr,
			GL_STATIC_DRAW
		);

		// + POS ATTRIB
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(
			0,
			RAWLINE_FLOATS_PER_POS,
			GL_FLOAT,
			GL_FALSE,
			0,
			(GLvoid*)0
		);
		glVertexAttribDivisor(0, 1);

		// SETUP COL BUFFER
		glGenBuffers(1, &m_rawlineColorBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_rawlineColorBuffer);
		glBufferData(
			GL_ARRAY_BUFFER,
			RAWLINE_COL_MAXBYTES * sizeof(GLfloat),
			nullptr,
			GL_STATIC_DRAW
		);

		// + COL ATTRIB
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(
			1,
			RAWLINE_FLOATS_PER_COLOR, 
			GL_FLOAT,
			GL_FALSE,
			0,
			(GLvoid*)0
		);
		glVertexAttribDivisor(1, 1);
	}

	void ScreenRenderer::InitialiseTexturedLineBuffers()
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
			LINE_FLOATS_PER_POS, // 4 values (2 for start, 2 for end)
			GL_FLOAT,
			GL_FALSE,
			0,
			(GLvoid*)0
		);
		glVertexAttribDivisor(0, 1);

		// SETUP UV BUFFER - DONT FORGET: UPDATE THESE TO USE SLOT 1 INSTEAD OF 2 (BECAUSE WE REMOVED COLOUR BUFFER)
		glGenBuffers(2, &m_lineUVBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_lineUVBuffer);
		glBufferData(
			GL_ARRAY_BUFFER,
			LINE_UV_MAXBYTES * sizeof(GLfloat),
			nullptr,
			GL_STATIC_DRAW
		);

		// + UV ATTRIB
		glEnableVertexAttribArray(1); // shares a shader with RawLines... 2 = texture data, 1 = raw colour data
		glVertexAttribPointer(
			1,
			LINE_FLOATS_PER_UV, // 2 value per instance (uv X pos)
			GL_FLOAT,
			GL_FALSE,
			0,
			(GLvoid*)0
		);
		glVertexAttribDivisor(1, 1);
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
			SPRITE_FLOATS_PER_POS,
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
			SPRITE_FLOATS_PER_COLOR, 
			GL_FLOAT,
			GL_FALSE,
			0,
			(GLvoid*)0
		);
		glVertexAttribDivisor(1, 1);
	}

	int ScreenRenderer::CreateSpriteBatch(const TextureBase& batchTexture, int capacity)
	{
		ASSERT(m_spriteBatchCount < SPRITE_BATCH_MAX);

		// configure new batch
		TextureBatch& newBatch = m_spriteBatches[m_spriteBatchCount];
		newBatch.pTexture = &batchTexture;
		newBatch.capacity = capacity;
		newBatch.count = 0;

		// start offset
		if(m_spriteBatchCount > 0)
		{
			TextureBatch& prevBatch{ m_spriteBatches[m_spriteBatchCount - 1] };
			newBatch.indexOffset = prevBatch.indexOffset + prevBatch.capacity;
		}
		else
		{
			newBatch.indexOffset = 0;
		}

		ASSERT(newBatch.indexOffset + capacity < SPRITE_MAX);
		return m_spriteBatchCount++;
	}
	void ScreenRenderer::ClearSpriteBatches()
	{
		m_spriteBatchCount = 0;
	}

	const TextureBase* ScreenRenderer::GetBatchTextures(int batchId)
	{
		ASSERT(batchId >= 0 && batchId < m_spriteBatchCount);
		TextureBatch& batch = m_spriteBatches[batchId];

		ASSERT(batch.pTexture);
		return batch.pTexture;
	}

	int ScreenRenderer::CreateLineBatch(const TextureBase& batchTexture, int capacity, float lineWidth)
	{
		ASSERT(m_lineBatchCount < LINE_BATCH_MAX);

		// configure new batch
		LineBatch& newBatch = m_lineBatches[m_lineBatchCount];
		newBatch.pTexture = &batchTexture;
		newBatch.capacity = capacity;
		newBatch.count = 0;
		newBatch.lineWidth = lineWidth;

		// start offset
		if (m_lineBatchCount > 0)
		{
			LineBatch& prevBatch{m_lineBatches[m_lineBatchCount - 1]};
			newBatch.indexOffset = prevBatch.indexOffset + prevBatch.capacity;
		}
		else
		{
			newBatch.indexOffset = 0;
		}

		ASSERT(newBatch.indexOffset + capacity < LINE_MAX);
		return m_lineBatchCount++;
	}
	void ScreenRenderer::ClearLineBatches()
	{
		m_lineBatchCount = 0;
	}

	void ScreenRenderer::SetBackgroundTextureData(GLfloat* pDataRGB, int width, int height)
	{
		m_backgroundTexture.SetDataFromArrayRGB(pDataRGB, width, height);
	}

	void ScreenRenderer::EraseBackgroundTextureData()
	{
		m_backgroundTexture.FreeTexture();
	}

	void ScreenRenderer::AddTexturedLine(const LineData& line, int batchId)
	{
		ASSERT(batchId >= 0 && batchId < m_lineBatchCount);
		LineBatch& batch = m_lineBatches[batchId];
		ASSERT(batch.count < batch.capacity);

		const Vector2f startPos{Core::GetNormalizedDeviceCoordinate(line.start)};
		const Vector2f endPos{Core::GetNormalizedDeviceCoordinate(line.end)};

		int batchPosIndex{ LINE_FLOATS_PER_POS * batch.indexOffset };
		int linePosIndex{ batchPosIndex + (LINE_FLOATS_PER_POS * batch.count) };
		m_linePosData[linePosIndex + 0] = startPos.x;
		m_linePosData[linePosIndex + 1] = startPos.y;
		m_linePosData[linePosIndex + 2] = endPos.x;
		m_linePosData[linePosIndex + 3] = endPos.y;

		int batchUvIndex{ LINE_FLOATS_PER_UV * batch.indexOffset };
		int lineUvIndex{ batchUvIndex + (LINE_FLOATS_PER_UV * batch.count) };
		m_lineUVData[lineUvIndex + 0] = line.texPosX;
		m_lineUVData[lineUvIndex + 1] = line.texLayer;

		batch.count++;
	}

	void ScreenRenderer::AddRawLine(const LineData& line, Colour4f colour)
	{
		ASSERT(m_rawlineCount < RAWLINE_MAX);

		const Vector2f startPos{ Core::GetNormalizedDeviceCoordinate(line.start) };
		const Vector2f endPos{ Core::GetNormalizedDeviceCoordinate(line.end) };

		int linePosIndex{ RAWLINE_FLOATS_PER_POS * m_rawlineCount };
		m_rawlinePosData[linePosIndex + 0] = startPos.x;
		m_rawlinePosData[linePosIndex + 1] = startPos.y;
		m_rawlinePosData[linePosIndex + 2] = endPos.x;
		m_rawlinePosData[linePosIndex + 3] = endPos.y;

		int lineColIndex{RAWLINE_FLOATS_PER_COLOR * m_rawlineCount};
		m_rawLineColData[lineColIndex + 0] = colour.r;
		m_rawLineColData[lineColIndex + 1] = colour.g;
		m_rawLineColData[lineColIndex + 2] = colour.b;
		m_rawLineColData[lineColIndex + 3] = colour.a;

		m_rawlineCount++;;
	}

	void ScreenRenderer::AddLinePoly(const LinePolyData& poly)
	{
		// line, triangle, square, or bigger
		ASSERT(poly.segments > 1);

		// decompose into individual lines
		// not the most efficient approach memory-wise as vertices get duplicated
		// but line-polys will not be a common occurrence in maps
		float increment {(PI * 2) / poly.segments};
		for (int i = 0; i < poly.segments; i++)
		{
			int nextIndex{ (i == poly.segments - 1) ? 0 : i + 1 };

			LineData line;
			line.start = poly.pos + Vector2f(cos(poly.rotation + (increment * i)), sin(poly.rotation + (increment * i))) * poly.radius;
			line.end = poly.pos + Vector2f(cos(poly.rotation + (increment * nextIndex)), sin(poly.rotation + (increment * nextIndex))) * poly.radius;

			AddRawLine(line, poly.colour);
		}
	}

	void ScreenRenderer::AddSprite(const SpriteData& sprite, int batchId)
	{
		ASSERT(batchId >= 0 && batchId < m_spriteBatchCount);
		TextureBatch& batch = m_spriteBatches[batchId];
		ASSERT(batch.count < batch.capacity);

		const Vector2f screenPos{ Core::GetNormalizedDeviceCoordinate(sprite.pos) };

		int batchPosIndex{SPRITE_FLOATS_PER_POS * batch.indexOffset};
		int spritePosIndex{batchPosIndex + (SPRITE_FLOATS_PER_POS * batch.count)};
		m_spritePosData[spritePosIndex + 0] = screenPos.x;
		m_spritePosData[spritePosIndex + 1] = screenPos.y;
		m_spritePosData[spritePosIndex + 2] = sprite.size.x;
		m_spritePosData[spritePosIndex + 3] = sprite.size.y;

		int batchColIndex{SPRITE_FLOATS_PER_COLOR * batch.indexOffset};
		int spriteColIndex{batchColIndex + (SPRITE_FLOATS_PER_COLOR * batch.count)};
		m_spriteColData[spriteColIndex + 0] = sprite.opacity;
		m_spriteColData[spriteColIndex + 1] = sprite.texLayer;

		batch.count++;
	}

	void ScreenRenderer::AddText(const TextData& textObj, int batchId)
	{
		ASSERT(batchId < m_spriteBatchCount);

		const std::string& gylphs = TextureFont::GetSupportedGlyphs();
		GLuint glyphWidth{m_spriteBatches[batchId].pTexture->GetWidth()};
		GLuint glyphHeight{m_spriteBatches[batchId].pTexture->GetHeight() / 2};

		Vector2f charPos{ textObj.pos };
		switch (textObj.alignment)
		{
			case TEXT_ALIGN_MIDDLE:
				charPos.x -= (textObj.text.length() / 2) * glyphWidth;
				break;
			case TEXT_ALIGN_RIGHT:
				charPos.x -= (textObj.text.length()) * glyphWidth;
				break;
		}

		for (int i = 0; i < textObj.text.length(); i++)
		{
			if(textObj.text.at(i) != ' ')
			{
				SpriteData glyph;
				glyph.pos = charPos;
				glyph.texLayer = gylphs.find(textObj.text.at(i));;
				glyph.opacity = textObj.opacity;
				glyph.size = textObj.scale;
				AddSprite(glyph, batchId);
			}

			charPos.x += (glyphWidth * textObj.scale.x);
		}
	}

	void ScreenRenderer::Render()
	{
		// Enable blending for transparency... #Refactor?
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		if (m_backgroundTexture.Exists())
		{
			RenderBackground();
		}
		else
		{
			glClear(GL_COLOR_BUFFER_BIT);
		}

		RenderRawLines();
		RenderTexturedLines();
		RenderSprites();
	}

	void ScreenRenderer::RenderBackground()
	{
		glUseProgram(m_backgroundShader);
		glBindTexture(GL_TEXTURE_2D, m_backgroundTexture.GetTextureId());
		GLCheck(glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, 1));
	}

	void ScreenRenderer::RenderRawLines()
	{
		glUseProgram(m_lineShaderColour);
		glBindVertexArray(m_rawlineVAO);

		// position
		glBindBuffer(GL_ARRAY_BUFFER, m_rawlinePosBuffer);
		glBufferSubData(
			GL_ARRAY_BUFFER,
			0,
			sizeof(GLfloat) * m_rawlineCount * RAWLINE_FLOATS_PER_POS,
			m_rawlinePosData
		);

		// colour
		glBindBuffer(GL_ARRAY_BUFFER, m_rawlineColorBuffer);
		glBufferSubData(
			GL_ARRAY_BUFFER,
			0,
			sizeof(GLfloat) * m_rawlineCount * RAWLINE_FLOATS_PER_COLOR,
			m_rawLineColData
		);

		// line width
		GLint widthLoc = glGetUniformLocation(m_lineShaderTextured, "lineWidth");
		glUniform2f(widthLoc, 2.f / Core::GetWindowSize().x, 2.f / Core::GetWindowSize().y);

		// render
		GLCheck(glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, m_rawlineCount)); // 4 vertices per instance, m_lineCount instances
		m_rawlineCount = 0;
	}

	void ScreenRenderer::RenderTexturedLines()
	{
		// SETUP VAO
		glUseProgram(m_lineShaderTextured);
		glBindVertexArray(m_lineVAO);

		// UPLOAD + RENDER EACH BATCH
		for (int i = 0; i < m_lineBatchCount; i++)
		{
			LineBatch& batch = m_lineBatches[i];
			if (!batch.count)
				continue;

			// position
			glBindBuffer(GL_ARRAY_BUFFER, m_linePosBuffer);
			glBufferSubData(
				GL_ARRAY_BUFFER,
				0,
				sizeof(GLfloat) * batch.count * LINE_FLOATS_PER_POS,
				&m_linePosData[batch.indexOffset * LINE_FLOATS_PER_POS]
			);

			// uv
			glBindBuffer(GL_ARRAY_BUFFER, m_lineUVBuffer);
			glBufferSubData(
				GL_ARRAY_BUFFER,
				0,
				sizeof(GLfloat) * batch.count * LINE_FLOATS_PER_UV,
				&m_lineUVData[batch.indexOffset * LINE_FLOATS_PER_UV]
			);

			// texture
			glBindTexture(GL_TEXTURE_2D_ARRAY, batch.pTexture->GetTextureId());
			
			// const uniform data
			GLint texLoc = glGetUniformLocation(m_lineShaderTextured, "textureSampler");
			glUniform1i(texLoc, 0);
			GLint widthLoc = glGetUniformLocation(m_lineShaderTextured, "lineWidth");
			glUniform2f(widthLoc, batch.lineWidth / Core::GetWindowSize().x, batch.lineWidth / Core::GetWindowSize().y);

			// render
			GLCheck(glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, batch.count)); // 4 vertices per instance, m_lineCount instances
			batch.count = 0;
		}
	}

	void ScreenRenderer::RenderSprites()
	{
		// Set sprite shader + VAO
		glUseProgram(m_spriteShader);
		glBindVertexArray(m_spriteVAO);

		// UPLOAD + RENDER EACH BATCH
		for (int i = 0; i < m_spriteBatchCount; i++)
		{
			TextureBatch& batch = m_spriteBatches[i];
			if(!batch.count)
				continue;

			// position
			glBindBuffer(GL_ARRAY_BUFFER, m_spritePosBuffer);
			glBufferSubData(
				GL_ARRAY_BUFFER,
				0,
				sizeof(GLfloat) * batch.count * SPRITE_FLOATS_PER_POS,
				&m_spritePosData[batch.indexOffset * SPRITE_FLOATS_PER_POS]
			);

			// color
			glBindBuffer(GL_ARRAY_BUFFER, m_spriteColBuffer);
			glBufferSubData(
				GL_ARRAY_BUFFER,
				0,
				sizeof(GLfloat) * batch.count * SPRITE_FLOATS_PER_COLOR,
				&m_spriteColData[batch.indexOffset * SPRITE_FLOATS_PER_COLOR]
			);

			// texture
			glBindTexture(batch.pTexture->IsArray() ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D, batch.pTexture->GetTextureId());

			// const uniform data
			GLint sizeLoc = glGetUniformLocation(m_spriteShader, "spriteSize");
			glUniform2f(
				sizeLoc,
				(static_cast<float>(batch.pTexture->GetWidth()) / Core::GetWindowSize().x),
				(static_cast<float>(batch.pTexture->GetHeight()) / Core::GetWindowSize().y)
			);

			// render
			GLCheck(glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, batch.count)); // 4 vertices per instance, batch.count instances
			batch.count = 0;
		}
	}

	void ScreenRenderer::ReleaseAll()
	{
		ClearSpriteBatches();
		ClearLineBatches();
		EraseBackgroundTextureData();
	}
}