#pragma once
#include <GameObject/GameObjectComponent.h>
#include <DataTypes/TextureProperty.h>
#include "GlobalTextureBatches.h"

struct RaycastSprite;

class RaycastSpriteComponent : public GameObjectComponent
{
public:
	SERIALIZABLE_DERIVED(RaycastSpriteComponent, GameObjectComponent, m_texture, m_localOffset, m_spriteSize, m_spriteHeight)

	virtual void OnDraw() const override;
	virtual void OnEditorDraw(const Vector3f& position, float zoom, float mapSpacing) override;

	float GetEditorHoverRadius(float zoom) const;

private:
	Spear::TextureProperty<GlobalTextureBatches::BATCH_SPRITESET_1> m_texture{};
	Vector2f m_localOffset{Vector2f::ZeroVector};
	Vector2f m_spriteSize{ 1, 1 };
	float m_spriteHeight{ 0 };
};

