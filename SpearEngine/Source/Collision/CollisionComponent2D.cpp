#include "CollisionComponent2D.h"
#include "GameObject/GameObject.h"
#include "CollisionSystem.h"

void CollisionComponent2D::OnCreated()
{
    Collision::CollisionSystem2D::Get().RegisterComponent(this);
    m_onMoved = GetOwner().OnMoved.Register(this, &CollisionComponent2D::OnMoved);
}

void CollisionComponent2D::OnDestroy()
{
    GetOwner().OnMoved.Unregister(m_onMoved);
    Collision::CollisionSystem2D::Get().DeregisterComponent(this);
}

void CollisionComponent2D::OnMoved(GameObject* obj)
{
#if COLLISION2D_QUADTREE
    if (!m_bDirty)
    {
        m_bDirty = true;
        Collision::CollisionSystem2D::Get().AddComponentToDirtyList(this);
    }
#endif
}

Vector2f CollisionComponent2D::GetOrigin() const
{
    return GetOwner().GetPosition().XY();
}

void CollisionComponent2D::ApplySetup(Collision::eCollisionType type, int blockingFlags, int overlapFlags, bool bStatic)
{
    SetIsStatic(bStatic);
    SetObjectType(type);
    SetBlockingFlags(blockingFlags);
    SetOverlapFlags(overlapFlags);
}

void CollisionComponent2D::SetIsStatic(bool bStatic)
{
    m_bStatic = bStatic;
}

void CollisionComponent2D::SetObjectType(Collision::eCollisionType type)
{
    m_objectType = type;
}

void CollisionComponent2D::SetBlockingFlags(int blockingFlags)
{
    m_blockingFlags = blockingFlags;
}

void CollisionComponent2D::SetOverlapFlags(int overlapFlags)
{
    m_overlapFlags = overlapFlags;
}

void CollisionComponentRadial::SetRadius(float radius)
{
    m_radius = radius;
}

float CollisionComponentRadial::GetMinX() const
{
    return GetOwner().GetPosition().x - m_radius;
}

float CollisionComponentRadial::GetMaxX() const
{
    return GetOwner().GetPosition().x + m_radius;
}

float CollisionComponentRadial::GetMinY() const
{
    return GetOwner().GetPosition().y - m_radius;
}

float CollisionComponentRadial::GetMaxY() const
{
    return GetOwner().GetPosition().y + m_radius;
}

void CollisionComponentRadial::ProjectToAxis(const Vector2f& axis, float& outMin, float& outMax) const
{
    float projectedCenter = Dot(GetOwner().GetPosition().XY(), axis);
    outMin = projectedCenter - m_radius;
    outMax = projectedCenter + m_radius;
}

void CollisionComponentAABB::SetHalfExtent(const Vector2f& halfExtent)
{
    m_halfExtent = halfExtent;
}

float CollisionComponentAABB::GetMinX() const
{
    return GetOwner().GetPosition().x - m_halfExtent.x;
}

float CollisionComponentAABB::GetMaxX() const
{
    return GetOwner().GetPosition().x + m_halfExtent.x;
}

float CollisionComponentAABB::GetMinY() const
{
    return GetOwner().GetPosition().y - m_halfExtent.y;
}

float CollisionComponentAABB::GetMaxY() const
{
    return GetOwner().GetPosition().y + m_halfExtent.y;
}

void CollisionComponentAABB::ProjectToAxis(const Vector2f& axis, float& outMin, float& outMax) const
{
    outMin = FLT_MAX;
    outMax = -FLT_MAX;

    const Vector3f& origin = GetOwner().GetPosition();
    Vector2f v[4] = {
        {origin.x - m_halfExtent.x, origin.y - m_halfExtent.y},
        {origin.x - m_halfExtent.x, origin.y + m_halfExtent.y},
        {origin.x + m_halfExtent.x, origin.y - m_halfExtent.y},
        {origin.x + m_halfExtent.x, origin.y + m_halfExtent.y},
    };

    for (int i = 0; i < 4; i++)
    {
        float axisDistance = Dot(v[i], axis);
        outMin = std::min(outMin, axisDistance);
        outMax = std::max(outMax, axisDistance);
    }
}
