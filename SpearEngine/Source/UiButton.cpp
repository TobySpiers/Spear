#include "UiButton.h"
#include "Core/ServiceLocator.h"
#include "Core/InputManager.h"
#include "Core/WindowManager.h"
#include "Graphics/TextureBase.h"

namespace Spear
{
	void UiButton::Initialise(const TextureBase& buttonTexture)
	{
		m_widthHeight = Vector2f(buttonTexture.GetWidth(), buttonTexture.GetHeight());
	}
	
	void UiButton::SetClickCallback(void (*function)(void))
	{
		m_callbackFunc = function;
	}

	bool UiButton::MouseOver()
	{
		const float spriteSizeX{ (m_widthHeight.x * m_sprite.size.x) / 2};
		const float spriteSizeY{ (m_widthHeight.y * m_sprite.size.y) / 2};
		
		Vector2i mouse{ServiceLocator::GetInputManager().GetMousePos()};
		return (mouse.x < m_sprite.pos.x + spriteSizeX
			&& mouse.x > m_sprite.pos.x - spriteSizeX
			&& mouse.y < m_sprite.pos.y + spriteSizeY
			&& mouse.y > m_sprite.pos.y - spriteSizeY);
	}

	bool UiButton::Clicked()
	{
		return (MouseOver() && ServiceLocator::GetInputManager().ClickRelease());
	}

	bool UiButton::RightClicked()
	{
		return (MouseOver() && ServiceLocator::GetInputManager().RightClickRelease());
	}

	void UiButton::Update()
	{
		constexpr float opacityDefault = 1.0f;
		constexpr float opacityHighlight = 0.9f;
		constexpr float opacityClick = 0.8f;
		


		if (MouseOver())
		{
			if (ServiceLocator::GetInputManager().ClickHold() || ServiceLocator::GetInputManager().RightClickHold())
			{
				m_sprite.opacity = opacityClick;
			}
			else
			{
				m_sprite.opacity = opacityHighlight;
			}

			if (ServiceLocator::GetInputManager().ClickRelease())
			{
				if(m_callbackFunc)
				{
					m_callbackFunc();
				}
			}
		}
		else
		{
			m_sprite.opacity = opacityDefault;
		}
	}

	void UiButton::Draw(int batchId)
	{
		Spear::ServiceLocator::GetScreenRenderer().AddSprite(m_sprite, batchId);
	}
}