#pragma once
#include "Texture.h"
#include "TextureFont.h"

namespace Spear
{
	// SPRITE BATCHES
	constexpr int SPRITE_MAX{1000};
	constexpr int SPRITE_BATCH_MAX{10};
	constexpr int SPRITE_FLOATS_PER_POS{4}; // 4 values (2 for xy, 2 for width/height)
	constexpr int SPRITE_FLOATS_PER_DRAW{3}; // 2 values (x: texture depth, y: opacity, z: render depth)
	constexpr int SPRITE_POS_MAXBYTES{SPRITE_FLOATS_PER_POS * SPRITE_MAX};
	constexpr int SPRITE_DRAW_MAXBYTES{SPRITE_FLOATS_PER_DRAW * SPRITE_MAX};

	// TEXT BATCHES
	constexpr const char* TEXT_DEFAULT_FONT = "../Assets/FONTS/PublicPixelRegular24/PublicPixel";
	constexpr int TEXT_CHAR_MAX{200}; // eats into total allocated for SPRITE_MAX
	constexpr int TEXT_BATCH_MAX{2};
	constexpr int TEXT_FLOATS_PER_POS{4}; // 4 values (2 for xy, 2 for scale)
	constexpr int TEXT_FLOATS_PER_COLOR{4}; // RGBA
	constexpr int TEXT_POS_MAXBYTES{TEXT_FLOATS_PER_POS * TEXT_CHAR_MAX};
	constexpr int TEXT_COL_MAXBYTES{TEXT_FLOATS_PER_COLOR * TEXT_CHAR_MAX};

	// LINES (UNBATCHED)
	constexpr int LINE_MAX{ 10000 };
	constexpr int LINE_FLOATS_PER_POS{ 4 }; // 4 values (x1, y1, x2, y2) 
	constexpr int LINE_FLOATS_PER_COLOR{ 4 }; // 4 values (rgba)
	constexpr int LINE_POS_MAXBYTES{ LINE_FLOATS_PER_POS * LINE_MAX };
	constexpr int LINE_COL_MAXBYTES{ LINE_FLOATS_PER_COLOR * LINE_MAX };

	// System for rendering 2D images to the screen
	class ScreenRenderer
	{
		NO_COPY(ScreenRenderer);
	public:
		
		enum TextAlign
		{
			TEXT_ALIGN_LEFT,
			TEXT_ALIGN_MIDDLE,
			TEXT_ALIGN_RIGHT,
		};

		struct TextData
		{
			std::string text;
			Colour3f colour{1.f, 1.f, 1.f};
			Vector2f pos{0.f, 0.f};
			Vector2f scale{1.f, 1.f};
			TextAlign alignment{TEXT_ALIGN_LEFT};
		};

		struct SpriteData
		{
			Vector2f pos{0.f, 0.f};
			Vector2f size{1.f, 1.f};
			float opacity{1.f};
			float depth{0.f};
			int texLayer{0}; // for TextureArray batches
		};

		struct LineData
		{
			Colour4f colour{ 1.f, 1.f, 1.f, 1.f };
			Vector2f start{ 0.f, 0.f };
			Vector2f end{ 0.f, 0.f };
			float depth{0.f};
		};

		struct LinePolyData
		{
			Colour4f colour{0.f, 0.f, 0.f, 0.f};
			Vector2f pos{ 0.f, 0.f };
			float radius{ 0.f };
			float rotation{ 0.f };
			float depth{ 0.f };
			int segments{ 3 };
		};

		ScreenRenderer();
		~ScreenRenderer();

		// General
		void SetInternalResolution(int width, int height);
		Vector2i GetInternalResolution() {return m_fboResolution;};
		void Render();
		void ReleaseAll();

		// Sprite Batch System
		int CreateSpriteBatch(const TextureBase& batchTexture, int capacity);
		const TextureBase* GetBatchTextures(int batchId);
		void AddSprite(const SpriteData& sprite, int batchId);
		void ClearSpriteBatches();

		// Text Batch System
		int CreateTextBatch(const TextureBase& fontTexture, int capacity);
		void AddText(const TextData& text, int fontBatchId = 0);
		void ClearTextBatches();

		// Raw Lines (discrete batches not necessary as no textures used)
		void AddLine(const LineData& line);
		void AddLinePoly(const LinePolyData& circle);

		// Background
		void SetBackgroundTextureDataRGBA(GLuint* pDataRGBA, GLfloat* pDataDepth, int width, int height);
		void EraseBackgroundTextureData();

	private:
		void InitialiseFrameBufferObject();
		void InitialiseBackgroundBuffers();
		void InitialiseLineBuffers();
		void InitialiseSpriteBuffers();
		void InitialiseTextBuffers();

		void RenderBackground();
		void RenderLines();
		void RenderSprites();
		void RenderText();

		struct TextureBatch
		{
			int indexOffset{0};	// position of Item 0 in main array
			int capacity{0};	// maximum items allowed in batch
			int count{0};		// number of items currently in batch
			const TextureBase* pTexture{nullptr};
		};

		// frame buffer
		Vector2i m_fboResolution{0, 0};
		GLuint m_fbo{0};
		GLuint m_fboDepthTexture{0};
		GLuint m_fboRenderTexture[2] = {}; // double buffer so we can write next frame before finishing prev frame
		int m_fboRenderTextureActive{0};

		// screen background
		Texture m_backgroundTexture[2]; // double buffer prevents having to wait for last frame to finish
		int m_backgroundTextureActive{0};
		GLuint m_backgroundDepthBuffer{0};

		// sprite data
		GLuint m_spriteVAO{0};
		GLuint m_spritePosBuffer{0};
		GLuint m_spriteColBuffer{0};
		GLfloat m_spritePosData[SPRITE_POS_MAXBYTES] = {};
		GLfloat m_spriteDrawData[SPRITE_DRAW_MAXBYTES] = {};
		TextureBatch m_spriteBatches[SPRITE_BATCH_MAX] = {};
		int m_spriteBatchCount{0};

		// text data
		TextureFont m_defaultFont;
		GLuint m_textVAO{0};
		GLuint m_textPosBuffer{0};
		GLuint m_textColBuffer{0};
		GLfloat m_textPosData[TEXT_POS_MAXBYTES] = {};
		GLfloat m_textColData[TEXT_COL_MAXBYTES] = {};
		TextureBatch m_textBatches[TEXT_BATCH_MAX] = {};
		int m_textBatchCount{0};

		// line data
		GLuint m_lineVAO{ 0 };
		GLuint m_linePosBuffer{0};
		GLuint m_lineColorBuffer{0};
		GLuint m_lineDepthBuffer{0};
		GLfloat m_linePosData[LINE_POS_MAXBYTES] = {};
		GLfloat m_lineColData[LINE_COL_MAXBYTES] = {};
		GLfloat m_lineDepthData[LINE_MAX] = {};
		int m_lineCount{0};

		// shaders
		GLuint m_lineShader{0};
		GLuint m_spriteShader{0};
		GLuint m_textShader{0};
		GLuint m_backgroundShader{0};
		GLuint m_screenShader{0};
	};
}