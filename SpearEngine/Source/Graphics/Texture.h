#pragma once
#include "TextureBase.h"

namespace Spear
{
	class Texture : public TextureBase
	{
	public:
		Texture();
		~Texture();

		bool SetDataFromArrayRGBA(GLuint* pPixels, int width, int height);
		void Resize(int width, int height);

		// TextureBase Overrides
		void FreeTexture() override;
		GLuint GetGpuTextureId() const override{return m_textureId;};
		GLuint GetWidth() const override {return m_textureWidth;};
		GLuint GetHeight() const override {return m_textureHeight;};
		GLuint GetDepth() const override {return 0;};
		bool Exists() const override {return (m_textureId != 0); };
		bool IsArray() const override {return false;};
		const SDL_Surface* GetSDLSurface(int slot = 0) const override {return nullptr;};

	private:
		void Allocate(int width, int height);
		GLuint m_textureId{0};
		GLuint m_textureWidth{0};
		GLuint m_textureHeight{0};
	};

}