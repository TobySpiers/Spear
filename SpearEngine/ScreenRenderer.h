#pragma once
#include "Texture.h"
#include "TextureArray.h"

namespace Spear
{
	constexpr int LINE_MAX{ 2000 };
	constexpr int FLOATS_PER_POS{4};
	constexpr int FLOATS_PER_COLOR{4};
	constexpr int FLOATS_PER_UV{2}; // X pos, Z (array slot)
	constexpr int INSTANCE_POS_MAX{FLOATS_PER_POS * LINE_MAX};
	constexpr int INSTANCE_COL_MAX{FLOATS_PER_COLOR * LINE_MAX};
	constexpr int INSTANCE_UV_MAX{FLOATS_PER_UV * LINE_MAX};


	// Renders 2D data to the screen (no 3D polygons)
	class ScreenRenderer
	{
		NO_COPY(ScreenRenderer);
	public:
		
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
		void SetBackgroundData(GLfloat* pDataRGB, int width, int height);
		void AddLine(const LineData& line);
		void AddLinePoly(const LinePolyData& circle);
		void SetLineWidth(float width){m_lineWidth = width;};
		void Render();

	private:
		int PosDataSize(){return sizeof(GLfloat) * (m_lineCount * FLOATS_PER_POS);};
		int ColDataSize(){return sizeof(GLfloat) * (m_lineCount * FLOATS_PER_COLOR);};
		int UVDataSize(){return sizeof(GLfloat) * (m_lineCount * FLOATS_PER_UV); };

		// line data
		GLfloat m_instancePosData[INSTANCE_POS_MAX] = {};	// position data
		GLfloat m_instanceColorData[INSTANCE_COL_MAX] = {};	// color data
		GLfloat m_instanceUVData[INSTANCE_UV_MAX] = {};		// UV data
		int m_lineCount{0};

		// render data
		float m_lineWidth{1.f};
		GLuint m_vertexArrayObj{0};
		GLuint m_instancePosBuffer{0};
		GLuint m_instanceColorBuffer{0};
		GLuint m_instanceUVBuffer{0};
		GLuint m_shaderLine{ 0 };

		// background texture
		GLuint m_shaderBackground{0};
		Texture m_backgroundTexture;

		// temp: texture data for wall
		TextureArray m_textureArray;
		Texture m_texture;
	};

}