#pragma once
#include "Texture.h"
#include "TextureFont.h"

namespace Spear
{
	// SPRITE BATCHES
	constexpr int SPRITE_MAX{1000};
	constexpr int SPRITE_BATCH_MAX{10};
	constexpr int SPRITE_FLOATS_PER_POS{4}; // 4 values (2 for xy, 2 for width/height)
	constexpr int SPRITE_FLOATS_PER_COLOR{2}; // 2 values (x: texture depth, y: opacity)
	constexpr int SPRITE_POS_MAXBYTES{SPRITE_FLOATS_PER_POS * SPRITE_MAX};
	constexpr int SPRITE_COL_MAXBYTES{SPRITE_FLOATS_PER_COLOR * SPRITE_MAX};

	// TEXT BATCHES
	constexpr const char* TEXT_DEFAULT_FONT = "../Assets/FONTS/PublicPixelRegular24/PublicPixel";
	constexpr int TEXT_CHAR_MAX{200}; // eats into total allocated for SPRITE_MAX
	constexpr int TEXT_BATCH_MAX{2};
	constexpr int TEXT_FLOATS_PER_POS{4}; // 4 values (2 for xy, 2 for scale)
	constexpr int TEXT_FLOATS_PER_COLOR{4}; // RGBA
	constexpr int TEXT_POS_MAXBYTES{TEXT_FLOATS_PER_POS * TEXT_CHAR_MAX};
	constexpr int TEXT_COL_MAXBYTES{TEXT_FLOATS_PER_COLOR * TEXT_CHAR_MAX};

	// TEXTURED LINE BATCHES
	constexpr int LINE_MAX{2000};
	constexpr int LINE_BATCH_MAX{2};
	constexpr int LINE_FLOATS_PER_POS{4}; // 0,1: startXY, 2,3: endXY
	constexpr int LINE_FLOATS_PER_INFO{3}; // 0: x pos, 1: texture slot, 2: depth
	constexpr int LINE_POS_MAXBYTES{LINE_FLOATS_PER_POS * LINE_MAX};
	constexpr int LINE_INFO_MAXBYTES{LINE_FLOATS_PER_INFO * LINE_MAX};

	// UNTEXTURED LINES (NO BATCHES)
	constexpr int RAWLINE_MAX{ 10000 };
	constexpr int RAWLINE_FLOATS_PER_POS{ 4 }; 
	constexpr int RAWLINE_FLOATS_PER_COLOR{ 4 }; // 4 values (rgba)
	constexpr int RAWLINE_POS_MAXBYTES{ RAWLINE_FLOATS_PER_POS * RAWLINE_MAX };
	constexpr int RAWLINE_COL_MAXBYTES{ RAWLINE_FLOATS_PER_COLOR * RAWLINE_MAX };

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
			Vector2f start{ 0.f, 0.f };
			Vector2f end{ 0.f, 0.f };
			float texPosX{0.f};
			float depth{0.f};
			int texLayer{0};
		};

		struct LinePolyData
		{
			Colour4f colour{0.f, 0.f, 0.f, 0.f};
			Vector2f pos{ 0.f, 0.f };
			float radius{ 0.f };
			float rotation{ 0.f };
			int segments{ 3 };
		};

		ScreenRenderer();
		~ScreenRenderer();

		// Sprite Batch System
		int CreateSpriteBatch(const TextureBase& batchTexture, int capacity);
		const TextureBase* GetBatchTextures(int batchId);
		void AddSprite(const SpriteData& sprite, int batchId);
		void ClearSpriteBatches();

		// Text Batch System
		int CreateTextBatch(const TextureBase& fontTexture, int capacity);
		void AddText(const TextData& text, int fontBatchId = 0);
		void ClearTextBatches();

		// Line Batch System
		int CreateLineBatch(const TextureBase& batchTexture, int capacity, float lineWidth = 1.f);
		void SetBatchLineWidth(int batchId, float width){m_lineBatches[batchId].lineWidth = width; };
		void AddTexturedLine(const LineData& line, int batchId);
		void ClearLineBatches();

		// Raw Lines (discrete batches not necessary as no textures used)
		void AddRawLine(const LineData& line, Colour4f colour);
		void AddLinePoly(const LinePolyData& circle);

		// Background
		void SetBackgroundTextureDataRGBA(GLuint* pDataRGBA, GLfloat* pDataDepth, int width, int height);
		void EraseBackgroundTextureData();

		void Render();
		void ReleaseAll();

	private:
		void InitialiseFrameBufferObject();
		void InitialiseBackgroundBuffers();
		void InitialiseRawLineBuffers();
		void InitialiseTexturedLineBuffers();
		void InitialiseSpriteBuffers();
		void InitialiseTextBuffers();

		void RenderBackground();
		void RenderRawLines();
		void RenderTexturedLines();
		void RenderSprites();
		void RenderText();

		struct TextureBatch
		{
			int indexOffset{0};	// position of Item 0 in main array
			int capacity{0};	// maximum items allowed in batch
			int count{0};		// number of items currently in batch
			const TextureBase* pTexture{nullptr};
		};
		struct LineBatch : TextureBatch
		{
			float lineWidth{1.f};
		};

		// frame buffer
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
		GLfloat m_spriteColData[SPRITE_COL_MAXBYTES] = {};
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

		// textured line data
		GLuint m_lineVAO{0};
		GLuint m_linePosBuffer{0};
		GLuint m_lineUVBuffer{0};
		GLfloat m_linePosData[LINE_POS_MAXBYTES] = {};
		GLfloat m_lineUVData[LINE_INFO_MAXBYTES] = {};
		LineBatch m_lineBatches[LINE_BATCH_MAX] = {};
		int m_lineBatchCount{0};

		// untextured line data
		GLuint m_rawlineVAO{ 0 };
		GLuint m_rawlinePosBuffer{0};
		GLuint m_rawlineColorBuffer{0};
		GLfloat m_rawlinePosData[RAWLINE_POS_MAXBYTES] = {};
		GLfloat m_rawLineColData[RAWLINE_COL_MAXBYTES] = {};
		int m_rawlineCount{0};

		// shaders
		GLuint m_lineShaderTextures{0};
		GLuint m_lineShaderColour{0};
		GLuint m_spriteShader{0};
		GLuint m_textShader{0};
		GLuint m_backgroundShader{0};
		GLuint m_screenShader{0};
	};
}