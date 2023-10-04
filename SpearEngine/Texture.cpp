#include "Texture.h"
#include "SDL_Image.h"
#include "Colour.h"

namespace Spear
{
	Texture::Texture()
	{
	}

	Texture::~Texture()
	{
		FreeTexture();
	}

	void Texture::Allocate()
	{
		// Create new texture slot and get ID
		glGenTextures(1, &m_textureId);
	}

	bool Texture::SetDataFromFile(const char* filename)
	{
		if (m_textureId == 0)
		{
			Allocate();
		}

		SDL_Surface* pSurface = IMG_Load(filename);
		if (!pSurface)
		{
			LOG(std::string("Texture failed to load: ") + filename);
			return false;
		}

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

		// bind THIS texture
		glBindTexture(GL_TEXTURE_2D, m_textureId);

		// load pixelData into texture slot
		GLCheck(glTexImage2D(
			GL_TEXTURE_2D,						// type of texture we are assigning to
			0,									// mip map level
			GL_RGBA,							// pixel format of how texture should be stored 
			pSurface->w,
			pSurface->h,
			0,									// texture border width
			GL_RGBA,							// format of the data BEING assigned
			GL_UNSIGNED_BYTE,					// data type of the pixel data being assigned
			pSurface->pixels
		));

		// nearest pixel filtering
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		// repeat wrap
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// unbind texture
		glBindTexture(GL_TEXTURE_2D, NULL);

		SDL_FreeSurface(pSurface);
		return true;
	}

	bool Texture::SetDataFromArrayRGB(float* pPixels, int width, int height)
	{
		if (m_textureId == 0)
		{
			Allocate();
		}

		// bind THIS texture
		glBindTexture(GL_TEXTURE_2D, m_textureId);

		// load pixelData into texture slot
		GLCheck(glTexImage2D(
			GL_TEXTURE_2D,						// type of texture we are assigning to
			0,									// mip map level
			GL_RGB,							// pixel format of how texture should be stored 
			width,
			height,
			0,									// texture border width
			GL_RGB,							// format of the data BEING assigned
			GL_FLOAT,					// data type of the pixel data being assigned
			pPixels
		));

		// nearest pixel filtering
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		// repeat wrap
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		// unbind texture
		glBindTexture(GL_TEXTURE_2D, NULL);

		m_textureWidth = width;
		m_textureHeight = height;
		return true;
	}

	void Texture::FreeTexture()
	{
		if (m_textureId != 0)
		{
			glDeleteTextures(1, &m_textureId);
			m_textureId = 0;
		}

		m_textureWidth = 0;
		m_textureHeight = 0;
	}
}