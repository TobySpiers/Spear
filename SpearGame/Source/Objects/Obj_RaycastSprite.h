#pragma once
#include "GameObject/GameObject.h"

struct RaycastSprite;

class Obj_RaycastSprite : public GameObject
{
	GAMEOBJECT_SERIALISABLE(Obj_RaycastSprite, GameObject, m_spriteId, m_spriteHeight, m_spriteSize)

public:

	virtual void OnCreated() override;
	virtual void OnTick(float deltaTime) override;
	virtual void OnDestroy() override;


private:
	RaycastSprite* m_raycastSprite{nullptr};
	Vector2f m_spriteSize{1, 1};
	int m_spriteId{0};
	int m_spriteHeight{0};
};