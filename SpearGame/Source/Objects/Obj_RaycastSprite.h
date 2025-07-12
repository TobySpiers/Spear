#pragma once
#include "GameObject/GameObject.h"
#include "GlobalTextureBatches.h"
#include <Graphics/TextureProperty.h>

struct RaycastSprite;

class Obj_RaycastSprite : public GameObject
{
	GAMEOBJECT_SERIALISABLE(Obj_RaycastSprite, GameObject, m_texture, m_spriteHeight, m_spriteSize)

public:

	virtual void OnCreated() override;
	virtual void OnTick(float deltaTime) override;
	virtual void OnDestroy() override;

	virtual void OnEditorDraw(const Vector3f& position, float zoom) override;
	virtual float GetEditorHoverRadius(float zoom) override;

private:
	void RefreshSpriteData();

	Spear::TextureProperty<GlobalTextureBatches::BATCH_SPRITESET_1> m_texture;

	RaycastSprite* m_raycastSprite{nullptr};
	Vector2f m_spriteSize{1, 1};
	float m_spriteHeight{0};
};