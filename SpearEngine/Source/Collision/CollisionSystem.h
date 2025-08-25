#pragma once
#include "Core/Core.h"
#include "Core/Serializer.h"
#include "DataTypes/QuadTree.h"
#include "CollisionTypes.h"

class CollisionComponent2D;

// Defines for enabling different collision methods
#define COLLISION2D_SWEEP 0
#define COLLISION2D_QUADTREE 1

namespace Collision
{
	struct CollisionInfo
	{
		float depth{ 0.f };
		Vector2f normal{ 0.f, 0.f };
		bool bBlockingHit{false};
	};

	class CollisionSystem2D
	{
	public:
		static CollisionSystem2D& Get();

		void RegisterComponent(CollisionComponent2D* coll);
		void DeregisterComponent(CollisionComponent2D* coll);

		// Responsibility is on caller not to submit the same component more than once per collision tick. This is not fatal but wastes performance.
		void AddComponentToDirtyList(CollisionComponent2D* coll);

		// WARNING -- Empties the tree!
		void ResizeQuadTree(const Vector2f& worldMin, const Vector2f& worldMax);

		void Tick();

	private:
		// Returns true on overlap or blocking hit (overlap or blocking is stored in outCollInfo)
		bool ProcessCollisionPair(CollisionComponent2D* compA, CollisionComponent2D* compB, CollisionInfo& outCollInfo) const;
		bool CanBlock(CollisionComponent2D* compA, CollisionComponent2D* compB) const;
		bool CanOverlap(CollisionComponent2D* compA, CollisionComponent2D* compB) const;

		// Note: This function makes an assumption the wider collision system never passes in a Static v. Static collision
		void DepenetrateComponents(CollisionComponent2D* compA, CollisionComponent2D* compB, const CollisionInfo& info) const;

		// Quick axis test for separation
		static bool CheckSeparation(float minA, float maxA, float minB, float maxB) {return (minA >= maxB || minB >= maxA); }

		// Compares two components for intersection and fills outInfo with normal & depth needed to depenetrate (functions assume X overlap is guaranteed due to sweep & prune)
		bool IntersectComponents_CircleCircle(CollisionComponent2D* circleA, CollisionComponent2D* circleB, CollisionInfo& outInfo) const;
		bool IntersectComponents_BoxBox(CollisionComponent2D* boxA, CollisionComponent2D* boxB, CollisionInfo& outInfo) const;
		bool IntersectComponents_CircleBox(CollisionComponent2D* circle, CollisionComponent2D* box, CollisionInfo& outInfo) const;

		// Quickly tests two components for overlap without calculating depenetration info (functions assume X overlap is guaranteed due to sweep & prune)
		bool OverlapComponents_CircleCircle(CollisionComponent2D* circleA, CollisionComponent2D* circleB) const;
		bool OverlapComponents_BoxBox(CollisionComponent2D* boxA, CollisionComponent2D* boxB) const;
		bool OverlapComponents_CircleBox(CollisionComponent2D* circle, CollisionComponent2D* box) const;

		// All registered collision components
		std::vector<CollisionComponent2D*> m_components;

#if COLLISION2D_QUADTREE
		// Quad tree storing all components grouped by 2D rects. Slow to iterate, fast to search.
		QuadTree<CollisionComponent2D*> m_quadTree;
		
		// Components which need their position in the quad tree updated
		std::vector<CollisionComponent2D*> m_componentsDirty;
#endif
	};
}