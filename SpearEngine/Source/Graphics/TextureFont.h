#pragma once
#include "TextureBase.h"

static const std::string SUPPORTED_GLYPHS("!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~");

namespace Spear
{

	class TextureFont : public TextureBase
	{
	public:
		TextureFont();
		~TextureFont();

		void LoadFont(const char* fontPath);
		
		static const std::string& GetSupportedGlyphs() {return SUPPORTED_GLYPHS;};

		// TextureBase Overrides
		void FreeTexture() override;
		GLuint GetGpuTextureId() const override { return m_textureId; };
		GLuint GetWidth() const override { return m_textureWidth; };
		GLuint GetHeight() const override { return m_textureHeight; };
		GLuint GetDepth() const override { return m_textureDepth; };
		bool Exists() const override { return (m_textureId != 0); }
		bool IsArray() const override { return true; };
		const SDL_Surface* GetSDLSurface(int slot = 0) const override { return nullptr; };

	private:
		GLuint m_textureId{ 0 };
		GLuint m_textureWidth{ 0 };
		GLuint m_textureHeight{ 0 };
		GLuint m_textureDepth{ 0 };
	};
}