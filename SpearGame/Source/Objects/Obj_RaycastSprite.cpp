#include "Obj_RaycastSprite.h"
#include "Raycaster/Raycaster.h"

GAMEOBJECT_REGISTER(Obj_RaycastSprite)

void Obj_RaycastSprite::OnCreated()
{
	m_raycastSprite = Raycaster::CreateSprite(m_spriteId, GetPosition().XY());

	SetTickEnabled(true);
}

void Obj_RaycastSprite::OnTick(float deltaTime)
{
	m_raycastSprite->spritePos = GetPosition().XY();
	m_raycastSprite->height = m_spriteHeight * 500; // *500 makes it so that -1 equals FLOOR and +1 equals ROOF
	m_raycastSprite->size = (m_spriteSize * 1000).ToInt(); // *1000 makes it so that 1 == WallTile height, 0.5 == HalfHeight, etc.
}

void Obj_RaycastSprite::OnDestroy()
{
	Raycaster::ClearSprite(m_raycastSprite);
}
