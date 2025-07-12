#pragma once
#include "GameObject/GameObject.h"

struct RaycastSprite;

class Obj_RaycastSprite : public GameObject
{
	GAMEOBJECT_SERIALISABLE(Obj_RaycastSprite, GameObject, m_spriteBatch, m_spriteId, m_spriteHeight, m_spriteSize)

public:

	virtual void OnCreated() override;
	virtual void OnTick(float deltaTime) override;
	virtual void OnDestroy() override;

	virtual void OnEditorDraw(const Vector3f& position, float zoom) override;
	virtual float GetEditorHoverRadius(float zoom) override;

private:
	void RefreshSpriteData();

	RaycastSprite* m_raycastSprite{nullptr};
	Vector2f m_spriteSize{1, 1};
	int m_spriteBatch{0};
	int m_spriteId{0};
	int m_spriteHeight{0};
};