#include "RaycastSpriteComponent.h"
#include "GameObject\GameObject.h"
#include "Raycaster\Raycaster.h"

void RaycastSpriteComponent::OnDraw() const
{
	RaycastSprite& sprite = Raycaster::MakeSprite();
	sprite.spritePos = GetOwner()->GetPosition().XY() + m_localOffset;
	sprite.height = m_spriteHeight * 500; // *500 makes it so that -1 sets origin to FLOOR and +1 sets origin to ROOF
	sprite.size = m_spriteSize * Vector2f(1.f, 2.f); // (1, 2) makes it so that size (1,1) is equal to 1 Square Wall Tile
}

void RaycastSpriteComponent::OnEditorDraw(const Vector3f& position, float zoom, float mapSpacing)
{
	Spear::Renderer::SpriteData sprite;
	sprite.pos = position.XY() + (mapSpacing * m_localOffset);
	sprite.size = Vector2f(zoom, zoom) * m_spriteSize;
	sprite.texLayer = m_texture.SpriteId();
	sprite.depth = position.z;
	Spear::Renderer::Get().AddSprite(sprite, GlobalTextureBatches::BATCH_SPRITESET_1);
}

float RaycastSpriteComponent::GetEditorHoverRadius(float zoom) const
{
	return (zoom * 50) * std::max(m_spriteSize.x, m_spriteSize.y);
}
