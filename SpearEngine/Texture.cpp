#include "Texture.h"
#include "SDL_Image.h"
#include "Colour.h"

#if _DEBUG
#include "FrameProfiler.h"
#endif

namespace Spear
{
	Texture::Texture()
	{
	}

	Texture::~Texture()
	{
		FreeTexture();
	}

	void Texture::Allocate(int width, int height)
	{
		m_textureWidth = width;
		m_textureHeight = height;
		
		// Create new texture slot and get ID
		glGenTextures(1, &m_textureId);
		glBindTexture(GL_TEXTURE_2D, m_textureId);

		// Create the texture
		GLCheck(glTexImage2D(
			GL_TEXTURE_2D,						// type of texture we are assigning to
			0,									// mip map level
			GL_RGBA,							// pixel format of how texture should be stored 
			width,
			height,
			0,									// texture border width
			GL_RGBA,							// format of the data BEING assigned
			GL_UNSIGNED_INT_8_8_8_8_REV,		// data type of the pixel data being assigned
			NULL
		));
		glBindTexture(GL_TEXTURE_2D, NULL);
	}

	bool Texture::SetDataFromFile(const char* filename)
	{
		m_pSDLSurface = IMG_Load(filename);
		if (!m_pSDLSurface)
		{
			LOG(std::string("Texture failed to load: ") + filename);
			return false;
		}

		if (m_pSDLSurface->format->format != SDL_PIXELFORMAT_RGBA32 && m_pSDLSurface->format->format != SDL_PIXELFORMAT_BGRA32)
		{
			LOG(std::string("\nWARNING: Converted image from non-suitable texture format: ") + filename);
			SDL_Surface* pConvertedSurface = SDL_ConvertSurfaceFormat(m_pSDLSurface, SDL_PIXELFORMAT_RGBA32, 0);
			if (!pConvertedSurface)
			{
				LOG("\tABORT: Image conversion failed!");
				return false;
			}
			SDL_FreeSurface(m_pSDLSurface);		// free up the old image
			m_pSDLSurface = pConvertedSurface;		// update pointer to converted image
		}

		if (m_textureId == 0 || m_textureWidth != m_pSDLSurface->w || m_textureHeight != m_pSDLSurface->h)
		{
			Allocate(m_pSDLSurface->w, m_pSDLSurface->h);
		}

		// bind THIS texture
		glBindTexture(GL_TEXTURE_2D, m_textureId);

		// load pixelData into texture slot
		glTexSubImage2D(
			GL_TEXTURE_2D, 
			0, 
			0, 
			0, 
			m_pSDLSurface->w,
			m_pSDLSurface->h,
			GL_RGBA, 
			GL_UNSIGNED_BYTE,
			m_pSDLSurface->pixels
		);

		// nearest pixel filtering
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		// repeat wrap
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// unbind texture
		glBindTexture(GL_TEXTURE_2D, NULL);

		return true;
	}

	bool Texture::SetDataFromArrayRGBA(GLuint* pPixels, int width, int height)
	{
		if (m_textureId == 0 || m_textureWidth != width || m_textureHeight != height)
		{
			Allocate(width, height);
		}

		// bind THIS texture
		glBindTexture(GL_TEXTURE_2D, m_textureId);

		// update data in previously allocated memory
		glTexSubImage2D(
			GL_TEXTURE_2D,
			0,
			0,
			0,
			width,
			height,
			GL_RGBA,
			GL_UNSIGNED_BYTE,
			pPixels
		);

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

		SDL_FreeSurface(m_pSDLSurface);
	}
}