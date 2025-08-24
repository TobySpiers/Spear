#include "Obj_RaycastSprite.h"
#include "Raycaster/Raycaster.h"
#include "Graphics/ScreenRenderer.h"
#include "Components/RaycastSpriteComponent.h"
#include "Collision/CollisionComponent2D.h"

GAMEOBJECT_REGISTER(Obj_RaycastSprite)

Obj_RaycastSprite::Obj_RaycastSprite()
{
	m_spriteComp = AddComponent<RaycastSpriteComponent>();

	m_collisionComp = AddComponent<CollisionComponentRadial>();
	m_collisionComp->ApplySetup(Collision::World, Collision::PROFILE_Characters, Collision::PROFILE_None, true);
}

void Obj_RaycastSprite::OnCreated()
{
	SetDrawEnabled(true);
}

float Obj_RaycastSprite::GetEditorHoverRadius(float zoom)
{
	return m_spriteComp->GetEditorHoverRadius(zoom);
}
