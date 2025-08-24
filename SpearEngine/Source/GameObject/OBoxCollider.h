#pragma once
#include "GameObject.h"
#include "Collision/CollisionTypes.h"

class CollisionComponentAABB;

class OBoxCollider : public GameObject
{
	GAMEOBJECT_DEFINITION(OBoxCollider, GameObject)

public:
	OBoxCollider();

	void SetHalfExtent(const Vector2f& halfExtent);
	void ApplySetup(Collision::eCollisionType type, int blockingFlags, int overlapFlags, bool bStatic);

private:
	CollisionComponentAABB* m_collisionComp{nullptr};
};