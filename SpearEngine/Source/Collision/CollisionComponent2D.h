#pragma once
#include "GameObject/GameObjectComponent.h"
#include "CollisionSystem.h"
#include "CollisionTypes.h"
#include <unordered_set>

enum class eCollisionShape2D
{
	Radial,
	AABB
};

class CollisionComponent2D : public GameObjectComponent
{
public:
	SERIALIZABLE_DERIVED(CollisionComponent2D, GameObjectComponent, m_bStatic, m_objectType, m_blockingFlags, m_overlapFlags)
	
	friend class Collision::CollisionSystem2D;

	virtual void OnCreated() override;
	virtual void OnDestroy() override;
	void OnMoved(GameObject* obj);

	virtual eCollisionShape2D GetShape() const = 0;

	virtual float GetMinX() const = 0;
	virtual float GetMaxX() const = 0;
	virtual float GetMinY() const = 0;
	virtual float GetMaxY() const = 0;
	virtual float GetRadius() const {return 0.f;}

	// Returns TopLeft corner for AABB containing collision shape
	Vector2f GetAABBOrigin() const {return Vector2f(GetMinX(), GetMinY()); }
	// Returns extent from TopLeft to BottomRight for AABB containing collision shape
	Vector2f GetAABBExtent() const {return Vector2f(GetMaxX() - GetMinX(), GetMaxY() - GetMinY()); }

	virtual void ProjectToAxis(const Vector2f& axis, float& outMin, float& outMax) const = 0;

	Vector2f GetOrigin() const;
	bool IsStatic() const { return m_bStatic; }
	int GetObjectType() const { return m_objectType.GetVal(); }
	int GetBlockingFlags() const { return m_blockingFlags.GetVal(); }
	int GetOverlapFlags() const { return m_overlapFlags.GetVal(); }

	void ApplySetup(Collision::eCollisionType type, int blockingFlags, int overlapFlags, bool bStatic = false);
	void SetIsStatic(bool bStatic);
	void SetObjectType(Collision::eCollisionType type);
	void SetBlockingFlags(int blockingFlags);
	void SetOverlapFlags(int overlapFlags);

private:

	bool m_bStatic{false}; // if true, owner cannot be moved by collisions and tests against other static components will be skipped
	Collision::eCollisionType_Wrapper m_objectType{Collision::World};
	Collision::eCollisionType_WrapperFlags m_blockingFlags{Collision::PROFILE_None};
	Collision::eCollisionType_WrapperFlags m_overlapFlags{Collision::PROFILE_None};

	std::unordered_set<CollisionComponent2D*> m_overlapping;

	EventID m_onMoved{0};

#if COLLISION2D_SWEEP
	std::unordered_set<CollisionComponent2D*> m_oldOverlaps;
#elif COLLISION2D_QUADTREE
	std::list<QuadTreeItem<CollisionComponent2D*>>::iterator m_quadTreeLocation;
	bool m_bDirty{false}; // whether collision quadtree needs to be refreshed for this component
#endif
};

class CollisionComponentRadial : public CollisionComponent2D
{
public:
	SERIALIZABLE_DERIVED(CollisionComponentRadial, CollisionComponent2D, m_radius)

	void SetRadius(float radius);

	virtual eCollisionShape2D GetShape() const {return eCollisionShape2D::Radial;}

	virtual float GetMinX() const override;
	virtual float GetMaxX() const override;
	virtual float GetMinY() const override;
	virtual float GetMaxY() const override;
	virtual float GetRadius() const override { return m_radius; }

	virtual void ProjectToAxis(const Vector2f& axis, float& outMin, float& outMax) const override;

private:
	float m_radius{0.25f};
};

class CollisionComponentAABB : public CollisionComponent2D
{
public:
	SERIALIZABLE_DERIVED(CollisionComponentAABB, CollisionComponent2D, m_halfExtent)

	void SetHalfExtent(const Vector2f& halfExtent);

	virtual eCollisionShape2D GetShape() const { return eCollisionShape2D::AABB; }

	virtual float GetMinX() const override;
	virtual float GetMaxX() const override;
	virtual float GetMinY() const override;
	virtual float GetMaxY() const override;

	virtual void ProjectToAxis(const Vector2f& axis, float& outMin, float& outMax) const override;

private:
	Vector2f m_halfExtent{ 1.f, 1.f };
};