#include "Core.h"
#include "ScreenRenderer.h"
#include "ShaderCompiler.h"
#include "TextureFont.h"
#include "FrameProfiler.h"

namespace Spear
{

	ScreenRenderer::ScreenRenderer()
	{
		// Create necessary buffers/vaos
		InitialiseBackgroundBuffers();
		InitialiseFrameBufferObject();
		InitialiseLineBuffers();
		InitialiseSpriteBuffers();
		InitialiseTextBuffers();

		// COMPILE SHADERS
		m_lineShader = ShaderCompiler::CreateShaderProgram("../Shaders/LineVS.glsl", "../Shaders/LineFS.glsl");
		m_spriteShader = ShaderCompiler::CreateShaderProgram("../Shaders/SpriteVS.glsl", "../Shaders/SpriteFS.glsl");
		m_textShader = ShaderCompiler::CreateShaderProgram("../Shaders/TextVS.glsl", "../Shaders/TextFS.glsl");
		m_backgroundShader = ShaderCompiler::CreateShaderProgram("../Shaders/BackgroundVS.glsl", "../Shaders/BackgroundFS.glsl");
		m_screenShader = ShaderCompiler::CreateShaderProgram("../Shaders/ScreenVS.glsl", "../Shaders/ScreenFS.glsl");

		// Set samplers (colour-texture/depth-texture) for the background shader
		GLint bgTexLoc = glGetUniformLocation(m_backgroundShader, "textureSampler");
		GLint bgDepthLoc = glGetUniformLocation(m_backgroundShader, "depthSampler");
		glUseProgram(m_backgroundShader);
		glUniform1i(bgTexLoc, 0);
		glUniform1i(bgDepthLoc, 1);
		glUseProgram(NULL);
	}

	ScreenRenderer::~ScreenRenderer()
	{
		ReleaseAll();
		glDeleteFramebuffers(1, &m_fbo);
		glDeleteTextures(1, &m_fboRenderTexture[0]);
		glDeleteTextures(1, &m_fboRenderTexture[1]);
		glDeleteTextures(1, &m_fboDepthTexture);
	}

	void ScreenRenderer::InitialiseFrameBufferObject()
	{
		// Create fbo
		glGenFramebuffers(1, &m_fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

		// Create fbo resources
		glGenTextures(1, &m_fboRenderTexture[0]);
		glGenTextures(1, &m_fboRenderTexture[1]);
		glGenTextures(1, &m_fboDepthTexture);
		SetInternalResolution(Core::GetWindowSize().x, Core::GetWindowSize().y);

		// Attach resources
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_fboRenderTexture[0], 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_fboDepthTexture, 0);

		// Unbind fbo
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void ScreenRenderer::SetInternalResolution(int width, int height)
	{
		// Initialise render texture 0
		glBindTexture(GL_TEXTURE_2D, m_fboRenderTexture[0]);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RGBA,
			width,
			height,
			0,
			GL_RGBA,
			GL_UNSIGNED_BYTE,
			NULL
		);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		// Initialise render texture 1
		glBindTexture(GL_TEXTURE_2D, m_fboRenderTexture[1]);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RGBA,
			width,
			height,
			0,
			GL_RGBA,
			GL_UNSIGNED_BYTE,
			NULL
		);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		// Initialise depth texture
		glBindTexture(GL_TEXTURE_2D, m_fboDepthTexture);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_DEPTH_COMPONENT,
			width,
			height,
			0,
			GL_DEPTH_COMPONENT,
			GL_FLOAT,
			NULL
		);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		m_fboResolution = Vector2i(width, height);
	}

	void ScreenRenderer::InitialiseBackgroundBuffers()
	{
		glGenTextures(1, &m_backgroundDepthBuffer);
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
			LINE_FLOATS_PER_POS,
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
			LINE_FLOATS_PER_COLOR, 
			GL_FLOAT,
			GL_FALSE,
			0,
			(GLvoid*)0
		);
		glVertexAttribDivisor(1, 1);

		// SETUP DEPTH BUFFER
		glGenBuffers(1, &m_lineDepthBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_lineDepthBuffer);
		glBufferData(
			GL_ARRAY_BUFFER,
			LINE_MAX * sizeof(GLfloat),
			nullptr,
			GL_STATIC_DRAW
		);

		// + DEPTH ATTRIB
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(
			2,
			1, // 1 float per depth
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
			SPRITE_FLOATS_PER_POS,
			GL_FLOAT,
			GL_FALSE,
			0,
			(GLvoid*)0
		);
		glVertexAttribDivisor(0, 1);

		// SETUP DRAW BUFFER
		glGenBuffers(1, &m_spriteColBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_spriteColBuffer);
		glBufferData(
			GL_ARRAY_BUFFER,
			SPRITE_DRAW_MAXBYTES * sizeof(GLfloat),
			nullptr,
			GL_STATIC_DRAW
		);

		// + DRAW ATTRIB
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(
			1,
			SPRITE_FLOATS_PER_DRAW, 
			GL_FLOAT,
			GL_FALSE,
			0,
			(GLvoid*)0
		);
		glVertexAttribDivisor(1, 1);
	}

	void ScreenRenderer::InitialiseTextBuffers()
	{
		// CREATE VAO
		glGenVertexArrays(1, &m_textVAO);
		glBindVertexArray(m_textVAO);

		// SETUP POS BUFFER
		glGenBuffers(1, &m_textPosBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_textPosBuffer);
		glBufferData(
			GL_ARRAY_BUFFER,
			TEXT_POS_MAXBYTES * sizeof(GLfloat),
			nullptr,
			GL_STATIC_DRAW
		);

		// + POS ATTRIB
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(
			0,
			TEXT_FLOATS_PER_POS,
			GL_FLOAT,
			GL_FALSE,
			0,
			NULL
		);
		glVertexAttribDivisor(0, 1);

		// SETUP COL BUFFER
		glGenBuffers(1, &m_textColBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_textColBuffer);
		glBufferData(
			GL_ARRAY_BUFFER,
			TEXT_COL_MAXBYTES * sizeof(GLfloat),
			nullptr,
			GL_STATIC_DRAW
		);

		// + COL ATTRIB
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(
			1,
			TEXT_FLOATS_PER_COLOR,
			GL_FLOAT,
			GL_FALSE,
			0,
			NULL
		);
		glVertexAttribDivisor(1, 1);

		// Load default font
		m_defaultFont.LoadFont(TEXT_DEFAULT_FONT);
		CreateTextBatch(m_defaultFont, 150);
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

	int ScreenRenderer::CreateTextBatch(const TextureBase& fontTexture, int capacity)
	{
		ASSERT(m_textBatchCount < TEXT_BATCH_MAX);

		// configure new batch
		TextureBatch& newBatch = m_textBatches[m_textBatchCount];
		newBatch.pTexture = &fontTexture;
		newBatch.capacity = capacity;
		newBatch.count = 0;

		// start offset
		if (m_textBatchCount > 0)
		{
			TextureBatch& prevBatch{ m_textBatches[m_textBatchCount - 1] };
			newBatch.indexOffset = prevBatch.indexOffset + prevBatch.capacity;
		}
		else
		{
			newBatch.indexOffset = 0;
		}

		ASSERT(newBatch.indexOffset + capacity < TEXT_CHAR_MAX);
		return m_textBatchCount++;
	}
	void ScreenRenderer::ClearTextBatches()
	{
		m_textBatchCount = 1; // batch 0 is always reserved for default font
	}

	void ScreenRenderer::SetBackgroundTextureDataRGBA(GLuint* pDataRGBA, GLfloat* pDataDepth, int width, int height)
	{
		START_PROFILE("Upload Background Array");
		m_backgroundTexture[m_backgroundTextureActive].SetDataFromArrayRGBA(pDataRGBA, width, height);
		END_PROFILE("Upload Background Array");

		START_PROFILE("Upload Depth Array");
		glBindTexture(GL_TEXTURE_2D, m_backgroundDepthBuffer);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_R8,
			width,
			height,
			0,
			GL_RED,
			GL_FLOAT,
			pDataDepth
		);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);
		END_PROFILE("Upload Depth Array");
	}

	void ScreenRenderer::EraseBackgroundTextureData()
	{
		m_backgroundTexture[0].FreeTexture();
		m_backgroundTexture[1].FreeTexture();
		m_backgroundTextureActive = 0;
	}

	void ScreenRenderer::AddLine(const LineData& line)
	{
		ASSERT(m_lineCount < LINE_MAX);

		const Vector2f startPos{ Core::GetNormalizedDeviceCoordinate(line.start) };
		const Vector2f endPos{ Core::GetNormalizedDeviceCoordinate(line.end) };

		int linePosIndex{ LINE_FLOATS_PER_POS * m_lineCount };
		m_linePosData[linePosIndex + 0] = startPos.x;
		m_linePosData[linePosIndex + 1] = startPos.y;
		m_linePosData[linePosIndex + 2] = endPos.x;
		m_linePosData[linePosIndex + 3] = endPos.y;

		int lineColIndex{LINE_FLOATS_PER_COLOR * m_lineCount};
		m_lineColData[lineColIndex + 0] = line.colour.r;
		m_lineColData[lineColIndex + 1] = line.colour.g;
		m_lineColData[lineColIndex + 2] = line.colour.b;
		m_lineColData[lineColIndex + 3] = line.colour.a;

		m_lineDepthData[m_lineCount] = line.depth;

		m_lineCount++;
	}

	void ScreenRenderer::AddLinePoly(const LinePolyData& poly)
	{
		// line, triangle, square, or bigger
		ASSERT(poly.segments > 1);

		// decompose into individual lines
		// not the most efficient approach memory-wise as data gets duplicated a whole bunch
		// but for current purposes these are used sparingly enough for it not to matter too much
		float increment {(PI * 2) / poly.segments};
		for (int i = 0; i < poly.segments; i++)
		{
			int nextIndex{ (i == poly.segments - 1) ? 0 : i + 1 };

			LineData line;
			line.start = poly.pos + Vector2f(cos(poly.rotation + (increment * i)), sin(poly.rotation + (increment * i))) * poly.radius;
			line.end = poly.pos + Vector2f(cos(poly.rotation + (increment * nextIndex)), sin(poly.rotation + (increment * nextIndex))) * poly.radius;
			line.colour = poly.colour;
			line.depth = poly.depth;

			AddLine(line);
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

		int batchDrawIndex{SPRITE_FLOATS_PER_DRAW * batch.indexOffset};
		int spriteDrawIndex{batchDrawIndex + (SPRITE_FLOATS_PER_DRAW * batch.count)};
		m_spriteDrawData[spriteDrawIndex + 0] = sprite.opacity;
		m_spriteDrawData[spriteDrawIndex + 1] = sprite.texLayer;
		m_spriteDrawData[spriteDrawIndex + 2] = sprite.depth;

		batch.count++;
	}

	void ScreenRenderer::AddText(const TextData& textObj, int fontBatchId)
	{
		ASSERT(fontBatchId < m_textBatchCount);

		TextureBatch& batch = m_textBatches[fontBatchId];
		ASSERT(batch.count + textObj.text.length() < batch.capacity);

		GLuint glyphWidth{ batch.pTexture->GetWidth()};
		GLuint glyphHeight{ batch.pTexture->GetHeight() / 2};

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

		const std::string& supportedGlyphs = TextureFont::GetSupportedGlyphs();
		for (int i = 0; i < textObj.text.length(); i++)
		{
			if(textObj.text.at(i) != ' ')
			{
				const Vector2f screenPos{ Core::GetNormalizedDeviceCoordinate(charPos) };

				int batchPosIndex{ TEXT_FLOATS_PER_POS * batch.indexOffset };
				int glyphPosIndex{ batchPosIndex + (TEXT_FLOATS_PER_POS * batch.count) };
				m_textPosData[glyphPosIndex + 0] = screenPos.x;
				m_textPosData[glyphPosIndex + 1] = screenPos.y;
				m_textPosData[glyphPosIndex + 2] = textObj.scale.x;
				m_textPosData[glyphPosIndex + 3] = textObj.scale.y;

				int batchColIndex{ TEXT_FLOATS_PER_COLOR * batch.indexOffset };
				int glyphColIndex{ batchColIndex + (TEXT_FLOATS_PER_COLOR * batch.count) };
				m_textColData[glyphColIndex + 0] = textObj.colour.r;
				m_textColData[glyphColIndex + 1] = textObj.colour.g;
				m_textColData[glyphColIndex + 2] = textObj.colour.b;
				m_textColData[glyphColIndex + 3] = supportedGlyphs.find(textObj.text.at(i)); // texture layer/depth

				batch.count++;
			}

			charPos.x += (glyphWidth * textObj.scale.x);
		}
	}

	void ScreenRenderer::Render()
	{
		START_PROFILE("ScreenRenderer_Total");

		// FIRST PASS (to FrameBuffer) ----------------------------------------
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
		GLenum result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		ASSERT(result == GL_FRAMEBUFFER_COMPLETE);

		// Prepare frame (alpha blending for sprites/text)
		glViewport(0, 0, m_fboResolution.x, m_fboResolution.y);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_fboRenderTexture[m_fboRenderTextureActive], 0);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		// Render internal scene 
		RenderBackground();
		RenderLines();
		RenderSprites();

		// SECOND PASS (to Screen) ----------------------------------------
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDisable(GL_DEPTH_TEST);
		glViewport(0, 0, Core::GetWindowSize().x, Core::GetWindowSize().y);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glUseProgram(m_screenShader);
		glBindTexture(GL_TEXTURE_2D, m_fboRenderTexture[m_fboRenderTextureActive]);
		GLCheck(glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, 1));
		m_fboRenderTextureActive = (m_fboRenderTextureActive == 0 ? 1 : 0); // swap buffer
		
		RenderText();

		END_PROFILE("ScreenRenderer_Total");
	}

	void ScreenRenderer::RenderBackground()
	{
		START_PROFILE("ScreenRenderer_Background");
		if (m_backgroundTexture[m_backgroundTextureActive].Exists())
		{
			glUseProgram(m_backgroundShader);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_backgroundTexture[m_backgroundTextureActive].GetTextureId());

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, m_backgroundDepthBuffer);

			GLCheck(glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, 1));

			// Unbind textures
			glBindTexture(GL_TEXTURE_2D, 0);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, 0);

			// swap active background texture
			m_backgroundTextureActive = (m_backgroundTextureActive == 0 ? 1 : 0);
		}
		END_PROFILE("ScreenRenderer_Background");
	}

	void ScreenRenderer::RenderLines()
	{
		START_PROFILE("ScreenRenderer_Lines");

		glUseProgram(m_lineShader);
		glBindVertexArray(m_lineVAO);

		// position
		glBindBuffer(GL_ARRAY_BUFFER, m_linePosBuffer);
		glBufferSubData(
			GL_ARRAY_BUFFER,
			0,
			sizeof(GLfloat) * m_lineCount * LINE_FLOATS_PER_POS,
			m_linePosData
		);

		// colour
		glBindBuffer(GL_ARRAY_BUFFER, m_lineColorBuffer);
		glBufferSubData(
			GL_ARRAY_BUFFER,
			0,
			sizeof(GLfloat) * m_lineCount * LINE_FLOATS_PER_COLOR,
			m_lineColData
		);

		// depth
		glBindBuffer(GL_ARRAY_BUFFER, m_lineDepthBuffer);
		glBufferSubData(
			GL_ARRAY_BUFFER,
			0,
			sizeof(GLfloat) * m_lineCount,
			m_lineDepthData
		);

		// line width
		GLint widthLoc = glGetUniformLocation(m_lineShader, "lineWidth");
		glUniform2f(widthLoc, 2.f / Core::GetWindowSize().x, 2.f / Core::GetWindowSize().y);

		// render
		GLCheck(glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, m_lineCount)); // 4 vertices per instance, m_lineCount instances
		m_lineCount = 0;
		
		END_PROFILE("ScreenRenderer_Lines");
	}

	void ScreenRenderer::RenderSprites()
	{
		START_PROFILE("ScreenRenderer_Sprites");

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
				sizeof(GLfloat) * batch.count * SPRITE_FLOATS_PER_DRAW,
				&m_spriteDrawData[batch.indexOffset * SPRITE_FLOATS_PER_DRAW]
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

		END_PROFILE("ScreenRenderer_Sprites");
	}

	void ScreenRenderer::RenderText()
	{
		START_PROFILE("ScreenRenderer_Text");

		// Set sprite shader + VAO
		glUseProgram(m_textShader);
		glBindVertexArray(m_textVAO);

		// UPLOAD + RENDER EACH BATCH
		for (int i = 0; i < m_textBatchCount; i++)
		{
			TextureBatch& batch = m_textBatches[i];
			if (!batch.count)
				continue;

			// position
			glBindBuffer(GL_ARRAY_BUFFER, m_textPosBuffer);
			glBufferSubData(
				GL_ARRAY_BUFFER,
				0,
				sizeof(GLfloat) * batch.count * TEXT_FLOATS_PER_POS,
				&m_textPosData[batch.indexOffset * TEXT_FLOATS_PER_POS]
			);

			// color
			glBindBuffer(GL_ARRAY_BUFFER, m_textColBuffer);
			glBufferSubData(
				GL_ARRAY_BUFFER,
				0,
				sizeof(GLfloat) * batch.count * TEXT_FLOATS_PER_COLOR,
				&m_textColData[batch.indexOffset * TEXT_FLOATS_PER_COLOR]
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

		END_PROFILE("ScreenRenderer_Text");
	}

	void ScreenRenderer::ReleaseAll()
	{
		ClearSpriteBatches();
		ClearTextBatches();
		EraseBackgroundTextureData();
	}
}