#pragma once
#include "TextureBase.h"

namespace Spear
{
	class TextureArray : public TextureBase
	{
	public:
		TextureArray();
		~TextureArray();

		// For automatically allocating an array from a directory (all files must have equal dimensions)
		bool InitialiseFromDirectory(const char* dir, int* out_totalSlots = nullptr);

		// For manually allocating an array slot-by-slot:
		void Allocate(GLuint width, GLuint height, GLuint slots);
		bool SetDataFromFile(GLuint slot, const char* filename);
		bool SetDataFromArrayRGBA(GLuint slot, float* pPixels, int width, int height);

		// TextureBase Overrides
		void FreeTexture() override;
		GLuint GetGpuTextureId() const override { return m_textureId; };
		GLuint GetWidth() const override { return m_textureWidth; };
		GLuint GetHeight() const override { return m_textureHeight; };
		GLuint GetDepth() const override { return m_textureDepth; };
		bool Exists() const override { return (m_textureId != 0); }
		bool IsArray() const override { return true; };
		const SDL_Surface* GetSDLSurface(int slot = 0) const override { return m_pSDLSurfaces.at(slot); };

	private:
		void FreeSDLSurfaces();
		std::vector<SDL_Surface*> m_pSDLSurfaces;
		GLuint m_textureId{ 0 };
		GLuint m_textureWidth{ 0 };
		GLuint m_textureHeight{ 0 };
		GLuint m_textureDepth{ 0 };
	};

}