#pragma once
#include "TextureBase.h"

namespace Spear
{
	class TextureArray : public TextureBase
	{
	public:
		TextureArray();
		~TextureArray();

		void Allocate(GLuint width, GLuint height, GLuint slots);
		bool SetDataFromFile(GLuint slot, const char* filename);
		bool SetDataFromArrayRGBA(GLuint slot, float* pPixels, int width, int height);

		// TextureBase Overrides
		void FreeTexture() override;
		GLuint GetTextureId() const override { return m_textureId; };
		GLuint GetWidth() const override { return m_textureWidth; };
		GLuint GetHeight() const override { return m_textureHeight; };
		GLuint GetDepth() const override { return m_textureDepth; };

	private:
		GLuint m_textureId{ 0 };
		GLuint m_textureWidth{ 0 };
		GLuint m_textureHeight{ 0 };
		GLuint m_textureDepth{ 0 };
	};

}