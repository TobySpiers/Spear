#pragma once
#include "Texture.h"
#include "TextureArray.h"

namespace Spear
{
	// LINE DATA SIZES
	constexpr int LINE_MAX{ 2000 };
	constexpr int LINE_FLOATS_PER_POS{4};
	constexpr int LINE_FLOATS_PER_COLOR{4};
	constexpr int LINE_FLOATS_PER_UV{2}; // X pos, Z (array slot)
	constexpr int LINE_POS_MAXBYTES{LINE_FLOATS_PER_POS * LINE_MAX};
	constexpr int LINE_COL_MAXBYTES{LINE_FLOATS_PER_COLOR * LINE_MAX};
	constexpr int LINE_UV_MAXBYTES{LINE_FLOATS_PER_UV * LINE_MAX};

	// SPRITE DATA SIZES
	constexpr int SPRITE_MAX{500};
	constexpr int SPRITE_FLOATS_PER_POS{4};
	constexpr int SPRITE_FLOATS_PER_COLOR{2};
	constexpr int SPRITE_POS_MAXBYTES{SPRITE_FLOATS_PER_POS * SPRITE_MAX};
	constexpr int SPRITE_COL_MAXBYTES{SPRITE_FLOATS_PER_COLOR * SPRITE_MAX};


	// Render 2D data to the screen (no 3D polygons)
	class ScreenRenderer
	{
		NO_COPY(ScreenRenderer);
	public:
		
		struct SpriteData
		{
			Vector2f pos{0.f, 0.f};
			Vector2f scale{1.f, 1.f};
			float opacity{1.f};
			int textureSlot{0};
		};

		struct LinePolyData
		{
			Colour4f colour;
			Vector2f pos{ 0.f, 0.f };
			float radius{ 0.f };
			float rotation{ 0.f };
			int segments{ 3 };
		};

		struct LineData
		{
			Colour4f colour;
			Vector2f start{ 0.f, 0.f };
			Vector2f end{ 0.f, 0.f };
			float texPosX{0.f};
			float texLayer{1.f};
		};

		ScreenRenderer();

		void SetLineWidth(float width){m_lineWidth = width;};
		void AddLine(const LineData& line);
		void AddLinePoly(const LinePolyData& circle);
		void AddSprite(const SpriteData& sprite);

		void SetTextureArrayData(const TextureArray& textureArray);
		void SetBackgroundTextureData(GLfloat* pDataRGB, int width, int height);
		void EraseBackgroundTextureData();

		void Render();

	private:
		void InitialiseLineBuffers();
		void InitialiseSpriteBuffers();

		void RenderBackground();
		void RenderLines();
		void RenderSprites();

		int LinePosSize(){return sizeof(GLfloat) * (m_lineCount * LINE_FLOATS_PER_POS);};
		int LineColSize(){return sizeof(GLfloat) * (m_lineCount * LINE_FLOATS_PER_COLOR);};
		int LineUVSize(){return sizeof(GLfloat) * (m_lineCount * LINE_FLOATS_PER_UV); };

		int SpritePosSize() {return sizeof(GLfloat) * (m_spriteCount * SPRITE_FLOATS_PER_POS); };
		int SpriteColSize() {return sizeof(GLfloat) * (m_spriteCount * SPRITE_FLOATS_PER_COLOR); };

		// background data
		Texture m_backgroundTexture;
		GLuint m_backgroundShader{0};

		// texture array in use by sprites/lines
		const TextureArray* m_pTextureArray{ nullptr };

		// sprite data
		GLfloat m_spritePosData[SPRITE_POS_MAXBYTES] = {};
		GLfloat m_spriteColData[SPRITE_COL_MAXBYTES] = {};
		int m_spriteCount{0};
		// sprite buffers/pipeline
		GLuint m_spriteVAO{0};
		GLuint m_spritePosBuffer{0};
		GLuint m_spriteColBuffer{0};
		GLuint m_spriteShader{0};

		// line data
		GLfloat m_linePosData[LINE_POS_MAXBYTES] = {};
		GLfloat m_lineColorData[LINE_COL_MAXBYTES] = {};
		GLfloat m_lineUVData[LINE_UV_MAXBYTES] = {};
		int m_lineCount{0};
		// line buffers/pipeline
		GLuint m_lineVAO{0};
		GLuint m_linePosBuffer{0};
		GLuint m_lineColorBuffer{0};
		GLuint m_lineUVBuffer{0};
		GLuint m_lineShader{ 0 };
		float m_lineWidth{1.f};

	};
}