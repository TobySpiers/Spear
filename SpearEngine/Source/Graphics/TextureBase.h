#pragma once
#include "Core/Core.h"

namespace Spear
{
	class TextureBase
	{
	public:
		virtual void FreeTexture() = 0;

		virtual GLuint GetTextureId() const = 0;
		virtual GLuint GetWidth() const = 0;
		virtual GLuint GetHeight() const = 0;
		virtual GLuint GetDepth() const = 0;
		virtual bool Exists() const = 0;
		virtual bool IsArray() const = 0;
		virtual const SDL_Surface* GetSDLSurface(int slot = 0) const = 0;
	};

}