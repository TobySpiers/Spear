#pragma once
#include "Graphics/ScreenRenderer.h"

namespace Spear
{
	class UiButton
	{
	public:
		UiButton(){};
		UiButton(const TextureBase& buttonTexture) {Initialise(buttonTexture); };
		void Initialise(const TextureBase& buttonTexture);
		void Initialise(const float imageWidth, const float imageHeight) {m_widthHeight = Vector2f(imageWidth, imageHeight); }

		void SetClickCallback(void (*function)(void));

		bool MouseOver();
		bool Clicked();
		bool RightClicked();

		void Update();
		void Draw(int batchId);
		
		ScreenRenderer::SpriteData m_sprite;

	private:
		Vector2f m_widthHeight{1.f, 1.f};
		void (*m_callbackFunc)(void) = nullptr;
	};
}