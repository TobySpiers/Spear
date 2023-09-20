#pragma once

namespace Spear
{
	class Texture
	{
	public:
		Texture(){};
		~Texture();

		bool LoadTextureFromFile(const char* filename);
		void FreeTexture();

		GLuint GetTextureId(){return m_textureId;};
		GLuint GetWidth(){return m_textureWidth;};
		GLuint GetHeight(){return m_textureHeight;};

	private:
		GLuint m_textureId{0};
		GLuint m_textureWidth{0};
		GLuint m_textureHeight{0};
	};

}