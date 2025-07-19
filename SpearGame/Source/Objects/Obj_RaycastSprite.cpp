#include "Obj_RaycastSprite.h"
#include "Raycaster/Raycaster.h"
#include "Graphics/ScreenRenderer.h"
#include "Components/RaycastSpriteComponent.h"

GAMEOBJECT_REGISTER(Obj_RaycastSprite)

Obj_RaycastSprite::Obj_RaycastSprite()
{
	GameObject* ptr = this;
	m_spriteComp = AddComponent<RaycastSpriteComponent>();
	m_collisionComp = AddComponent<CollisionComponent_Radial>();
}

void Obj_RaycastSprite::OnCreated()
{
	SetDrawEnabled(true);
}

float Obj_RaycastSprite::GetEditorHoverRadius(float zoom)
{
	return m_spriteComp->GetEditorHoverRadius(zoom);
}
