#include "Core.h"
#include "Texture.h"
#include "SDL_Image.h"

namespace Spear
{
	Texture::~Texture()
	{
		FreeTexture();
	}

	bool Texture::LoadTextureFromFile(const char* filename)
	{
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

		// create and bind fresh texture slot
		glGenTextures(1, &m_textureId);
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