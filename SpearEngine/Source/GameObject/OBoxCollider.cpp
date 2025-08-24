#include "OBoxCollider.h"
#include "Collision/CollisionComponent2D.h"
#include "Collision/CollisionTypes.h"

OBoxCollider::OBoxCollider()
{
	m_collisionComp = AddComponent<CollisionComponentAABB>();
	m_collisionComp->ApplySetup(Collision::World, Collision::PROFILE_All, Collision::PROFILE_None, true);
}

void OBoxCollider::SetHalfExtent(const Vector2f& halfExtent)
{
	m_collisionComp->SetHalfExtent(halfExtent);
}

void OBoxCollider::ApplySetup(Collision::eCollisionType type, int blockingFlags, int overlapFlags, bool bStatic)
{
	m_collisionComp->ApplySetup(type, blockingFlags, overlapFlags, bStatic);
}