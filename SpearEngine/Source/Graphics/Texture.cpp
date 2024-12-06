#include "Texture.h"
#include "SDL_Image.h"
#include "Core/Colour.h"

#if _DEBUG
#include "Core/FrameProfiler.h"
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
	}
}