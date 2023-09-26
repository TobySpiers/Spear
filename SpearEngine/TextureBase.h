#pragma once
#include "Core.h"

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
	};

}