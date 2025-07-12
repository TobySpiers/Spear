#include "Obj_RaycastSprite.h"
#include "Raycaster/Raycaster.h"
#include "Graphics/ScreenRenderer.h"

GAMEOBJECT_REGISTER(Obj_RaycastSprite)

void Obj_RaycastSprite::OnCreated()
{
	m_raycastSprite = Raycaster::CreateSprite(m_texture.SpriteId(), GetPosition().XY());
	RefreshSpriteData();

	SetTickEnabled(true);
}

void Obj_RaycastSprite::OnTick(float deltaTime)
{
	RefreshSpriteData();
}

void Obj_RaycastSprite::OnDestroy()
{
	Raycaster::ClearSprite(m_raycastSprite);
	m_raycastSprite = nullptr;
}

void Obj_RaycastSprite::OnEditorDraw(const Vector3f& position, float zoom)
{
	Spear::Renderer::SpriteData sprite;
	sprite.pos = position.XY();
	sprite.size = Vector2f(zoom, zoom) * m_spriteSize;
	sprite.texLayer = m_texture.SpriteId();
	sprite.depth = position.z;
	Spear::Renderer::Get().AddSprite(sprite, GlobalTextureBatches::BATCH_SPRITESET_1);
}

float Obj_RaycastSprite::GetEditorHoverRadius(float zoom)
{
	return (zoom * 50) * std::max(m_spriteSize.x, m_spriteSize.y);
}

void Obj_RaycastSprite::RefreshSpriteData()
{
	m_raycastSprite->spritePos = GetPosition().XY();
	m_raycastSprite->height = m_spriteHeight * 500; // *500 makes it so that -1 sets origin to FLOOR and +1 sets origin to ROOF
	m_raycastSprite->size = m_spriteSize * Vector2f(1.f, 2.f); // (1, 2) makes it so that size (1,1) is equal to 1 Square Wall Tile
}
