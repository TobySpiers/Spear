#pragma once
#include "GameObjectComponent.h"
#include "Core/Serializer.h"

enum eCollisionType
{
	None = 0,
	WorldSolid = 1 << 0,
	WorldFlyable = 1 << 1,
	Player = 1 << 2,
	NPC = 1 << 3,
	Collectable = 1 << 4,
	Interaction = 1 << 5
};
EXPOSE_ENUM_FLAGS(eCollisionType, None, WorldSolid, WorldFlyable, Player, NPC, Collectable, Interaction)

class CollisionComponent_Radial : public GameObjectComponent
{
public:
	SERIALIZABLE_DERIVED(CollisionComponent_Radial, GameObjectComponent, radius, objectType, blockingFlags, overlapFlags)

private:
	float radius{1.f};
	eCollisionType_Wrapper objectType{None}; // type of this collider
	eCollisionType_WrapperFlags blockingFlags{None}; // types to block against
	eCollisionType_WrapperFlags overlapFlags{None}; // types to overlap against
};

