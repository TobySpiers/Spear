#pragma once
#include "GameObject/GameObject.h"
#include "GlobalTextureBatches.h"
#include <DataTypes/TextureProperty.h>

struct RaycastSprite;
class RaycastSpriteComponent;
class CollisionComponentRadial;

class Obj_RaycastSprite : public GameObject
{
	GAMEOBJECT_DEFINITION(Obj_RaycastSprite, GameObject)

public:
	Obj_RaycastSprite();

	virtual void OnCreated() override;

	virtual float GetEditorHoverRadius(float zoom) override;

private:
	RaycastSpriteComponent* m_spriteComp{nullptr};
	CollisionComponentRadial* m_collisionComp{ nullptr };
};