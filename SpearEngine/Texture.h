#pragma once

namespace Spear
{
	class Texture
	{
	public:
		Texture();
		~Texture();

		bool SetDataFromFile(const char* filename);
		bool SetDataFromArrayRGB(float* pPixels, int width, int height);
		void FreeTexture();

		GLuint GetTextureId() const{return m_textureId;};
		GLuint GetWidth() const{return m_textureWidth;};
		GLuint GetHeight() const{return m_textureHeight;};

	private:
		GLuint m_textureId{0};
		GLuint m_textureWidth{0};
		GLuint m_textureHeight{0};
	};

}