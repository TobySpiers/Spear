#include "TextureArray.h"
#include "SDL_Image.h"
#include "Colour.h"

namespace Spear
{
	TextureArray::TextureArray()
	{}
	
	TextureArray::~TextureArray()
	{
		FreeTexture();
	}

	void TextureArray::Allocate(GLuint width, GLuint height, GLuint slots)
	{
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
			width,
			height,
			slots
		));

		glBindTexture(GL_TEXTURE_2D_ARRAY, 0); // unbind

		m_textureWidth = width;
		m_textureHeight = height;
		m_textureDepth = slots;
	}

	bool TextureArray::SetDataFromFile(GLuint slot, const char* filename)
	{
		SDL_Surface* pSurface = IMG_Load(filename);
		if (!pSurface)
		{
			LOG(std::string("Texture failed to load: ") + filename);
			return false;
		}
		ASSERT(pSurface->w == m_textureWidth && pSurface->h == m_textureHeight);

		if (pSurface->format->format != SDL_PIXELFORMAT_RGBA32 && pSurface->format->format != SDL_PIXELFORMAT_BGRA32)
		{
			LOG(std::string("\nWARNING: Converted image from non-suitable texture format: ") + filename);
			SDL_Surface* pConvertedSurface = SDL_ConvertSurfaceFormat(pSurface, SDL_PIXELFORMAT_RGBA32, 0);
			if (!pConvertedSurface)
			{
				LOG("\tABORT: Image conversion failed!");
				return false;
			}
			SDL_FreeSurface(pSurface);		// free up the old image
			pSurface = pConvertedSurface;	// update pointer to converted image
		}

		// bind THIS texture array
		glBindTexture(GL_TEXTURE_2D_ARRAY, m_textureId);

		GLCheck(glTexSubImage3D(
			GL_TEXTURE_2D_ARRAY,	// type we are assigning to
			0,						// no mipmap
			0,
			0,
			slot,					// 'index' into texture array
			m_textureWidth,			
			m_textureHeight,
			1,						// 'depth' of texture (always 1 for a single slice)
			GL_RGBA,
			GL_UNSIGNED_BYTE,
			pSurface->pixels
		));		

		// unbind texture
		glBindTexture(GL_TEXTURE_2D_ARRAY, NULL);

		SDL_FreeSurface(pSurface);
		return true;
	}

	bool TextureArray::SetDataFromArrayRGBA(GLuint slot, float* pPixels, int width, int height)
	{
		ASSERT(width == m_textureWidth && height == m_textureHeight);

		// bind THIS texture
		glBindTexture(GL_TEXTURE_2D_ARRAY, m_textureId);

		GLCheck(glTexSubImage3D(
			GL_TEXTURE_2D_ARRAY,
			0,	
			0,
			0,
			slot,
			m_textureWidth,
			m_textureHeight,
			1,
			GL_RGBA,
			GL_UNSIGNED_BYTE,
			pPixels
		));

		// unbind texture
		glBindTexture(GL_TEXTURE_2D_ARRAY, NULL);

		return true;
	}

	void TextureArray::FreeTexture()
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