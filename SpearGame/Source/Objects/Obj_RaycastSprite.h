#pragma once
#include "GameObject/GameObject.h"
#include "GlobalTextureBatches.h"
#include <Graphics/TextureProperty.h>
#include <GameObject/Components/CollisionComponent_Radial.h>

struct RaycastSprite;
class RaycastSpriteComponent;

class Obj_RaycastSprite : public GameObject
{
	GAMEOBJECT_DEFINITION(Obj_RaycastSprite, GameObject)

public:
	Obj_RaycastSprite();

	virtual void OnCreated() override;

	virtual float GetEditorHoverRadius(float zoom) override;

private:
	RaycastSpriteComponent* m_spriteComp{nullptr};
	CollisionComponent_Radial* m_collisionComp{nullptr};
};