#pragma once
#include "TextureBase.h"

namespace Spear
{
	class Texture : public TextureBase
	{
	public:
		Texture();
		~Texture();

		bool SetDataFromFile(const char* filename);
		bool SetDataFromArrayRGB(float* pPixels, int width, int height);


		// TextureBase Overrides
		void FreeTexture() override;
		GLuint GetTextureId() const override{return m_textureId;};
		GLuint GetWidth() const override {return m_textureWidth;};
		GLuint GetHeight() const override {return m_textureHeight;};
		GLuint GetDepth() const override {return 0;};
		bool Exists() const override {return (m_textureId != 0); };
		bool IsArray() const override {return false;};
		const SDL_Surface* GetSDLSurface(int slot = 0) const override {return m_pSDLSurface;};

	private:
		void Allocate();
		SDL_Surface* m_pSDLSurface{nullptr};
		GLuint m_textureId{0};
		GLuint m_textureWidth{0};
		GLuint m_textureHeight{0};
	};

}