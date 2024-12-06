#include "TextureFont.h"
#include "SDL_Image.h"
#include <sstream>
#include <iomanip>

namespace Spear
{
	TextureFont::TextureFont()
	{}

	TextureFont::~TextureFont()
	{
		FreeTexture();
	}

	void TextureFont::LoadFont(const char* fontPath)
	{
		std::string filename(std::string(fontPath) + "_000.png");
		SDL_Surface* pSurface = IMG_Load(filename.c_str());
		ASSERT(pSurface);
		GLuint glyphWidth = pSurface->w;
		GLuint glyphHeight = pSurface->h;
		SDL_FreeSurface(pSurface);


		m_textureWidth = glyphWidth;
		m_textureHeight = glyphHeight;
		m_textureDepth = SUPPORTED_GLYPHS.length();

		// Create new texture array
		glGenTextures(1, &m_textureId);
		glBindTexture(GL_TEXTURE_2D_ARRAY, m_textureId);
		// nearest pixel filtering
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		// repeat wrap
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		// initialise format
		GLCheck(glTexStorage3D(
			GL_TEXTURE_2D_ARRAY,
			1,
			GL_RGBA8,
			m_textureWidth,
			m_textureHeight,
			m_textureDepth
		));

		// Upload Glyph images to TextureArray
		for (int i = 0; i < m_textureDepth; i++)
		{
			std::stringstream formattedSequenceNum;
			formattedSequenceNum << std::setw(3) << std::setfill('0') << (i);

			std::string filename(std::string(fontPath) + "_" + formattedSequenceNum.str() + ".png");

			SDL_Surface* pSurface = IMG_Load(filename.c_str());
			ASSERT(pSurface);
			ASSERT(pSurface->w == m_textureWidth && pSurface->h == m_textureHeight);

			if (pSurface->format->format != SDL_PIXELFORMAT_RGBA32 && pSurface->format->format != SDL_PIXELFORMAT_BGRA32)
			{
				LOG(std::string("\nWARNING: Converted image from non-suitable texture format: ") + filename);
				SDL_Surface* pConvertedSurface = SDL_ConvertSurfaceFormat(pSurface, SDL_PIXELFORMAT_RGBA32, 0);
				ASSERT(pConvertedSurface)
				SDL_FreeSurface(pSurface);		// free up the old image
				pSurface = pConvertedSurface;	// update pointer to converted image
			}

			GLCheck(glTexSubImage3D(
				GL_TEXTURE_2D_ARRAY,	// type we are assigning to
				0,						// no mipmap
				0,
				0,
				i,					// 'index' into texture array
				m_textureWidth,
				m_textureHeight,
				1,						// 'depth' of texture (always 1 for a single slice)
				GL_RGBA,
				GL_UNSIGNED_BYTE,
				pSurface->pixels
			));

			SDL_FreeSurface(pSurface);
		}

		// unbind texture
		glBindTexture(GL_TEXTURE_2D_ARRAY, NULL);
	}

	void TextureFont::FreeTexture()
	{
		if (m_textureId != 0)
		{
			glDeleteTextures(1, &m_textureId);
			m_textureId = 0;
		}

		m_textureWidth = 0;
		m_textureHeight = 0;
		m_textureDepth = 0;
	}
}