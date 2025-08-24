#include "CollisionSystem.h"
#include "GameObject/GameObject.h"
#include "CollisionComponent2D.h"
#include "Core/ServiceLocator.h"
#include <algorithm>
#include <deque>
#include <Core/FrameProfiler.h>

namespace Collision
{
	CollisionSystem2D& CollisionSystem2D::Get()
	{
		return Spear::ServiceLocator::GetCollisionSystem();
	}

	void CollisionSystem2D::RegisterComponent(CollisionComponent2D* coll)
	{
		ASSERT(std::find(m_components.begin(), m_components.end(), coll) == m_components.end());
		m_components.emplace_back(coll);

#if COLLISION2D_QUADTREE
		coll->m_quadTreeLocation = m_quadTree.Insert(coll, QuadTreeRect(coll->GetAABBOrigin(), coll->GetAABBExtent()));
#endif
	}

	void CollisionSystem2D::DeregisterComponent(CollisionComponent2D* coll)
	{
		auto it = std::find(m_components.begin(), m_components.end(), coll);
#if COLLISION2D_SWEEP
		// No point swap-removing since we'd have to resort the whole vector again
		if (it != m_components.end())
		{
			m_components.erase(it);
		}
#elif COLLISION2D_QUADTREE
		if (it != m_components.end())
		{
			*it = m_components.back();
			m_components.pop_back();
		}
		m_quadTree.Remove(coll->m_quadTreeLocation);
#endif
	}

	void CollisionSystem2D::AddComponentToDirtyList(CollisionComponent2D* coll)
	{
#if COLLISION2D_QUADTREE
		m_componentsDirty.push_back(coll);
#endif
	}

	void CollisionSystem2D::ResizeQuadTree(const Vector2f& worldMin, const Vector2f& worldMax)
	{
#if COLLISION2D_QUADTREE
		m_quadTree.Resize(QuadTreeRect(worldMin, worldMax - worldMin));
#endif
	}

	void CollisionSystem2D::Tick()
	{
		START_PROFILE("CollisionSystem2D");

		enum eCollisionEventType
		{
			OverlapBegin,
			OverlapEnd,
			BlockingHit
		};
		struct CollisionEvent
		{
			eCollisionEventType eventType;
			CollisionComponent2D* compA;
			CollisionComponent2D* compB;
		};
		std::vector<CollisionEvent> collisionEvents;
		CollisionInfo collInfo;

#if COLLISION2D_SWEEP
		std::sort(m_components.begin(), m_components.end(), [](const CollisionComponent2D* a, const CollisionComponent2D* b)
		{
			return a->GetMinX() < b->GetMinX();
		});

		// Dual queues allows us to naturally skip static v static pairs since these shouldn't interact anyway
		std::deque<CollisionComponent2D*> staticSweep;
		std::deque<CollisionComponent2D*> nonstaticSweep;

		// Find likely pairs
		for (CollisionComponent2D* compA : m_components)
		{
			compA->m_oldOverlaps = std::move(compA->m_overlapping);
			compA->m_overlapping = {};

			while (!staticSweep.empty() && staticSweep.front()->GetMaxX() < compA->GetMinX())
			{
				CollisionComponent2D* compExitingSweep = staticSweep.front();
				for (CollisionComponent2D* oldOverlap : compExitingSweep->m_oldOverlaps)
				{
					collisionEvents.emplace_back(eCollisionEventType::OverlapEnd, compExitingSweep, oldOverlap);
				}

				staticSweep.pop_front();
			}
			while (!nonstaticSweep.empty() && nonstaticSweep.front()->GetMaxX() < compA->GetMinX())
			{
				CollisionComponent2D* compExitingSweep = nonstaticSweep.front();
				for (CollisionComponent2D* oldOverlap : compExitingSweep->m_oldOverlaps)
				{
					collisionEvents.emplace_back(eCollisionEventType::OverlapEnd, compExitingSweep, oldOverlap);
				}

				nonstaticSweep.pop_front();
			}

			// all objects should always test against swept nonstatic objects
			for (int q = 0; q < nonstaticSweep.size(); q++)
			{
				CollisionComponent2D* compB = nonstaticSweep[q];
				if(ProcessCollisionPair(compA, compB, collInfo))
				{
					if (collInfo.bBlockingHit)
					{
						collisionEvents.emplace_back(eCollisionEventType::BlockingHit, compA, compB);
					}
					else
					{
						compA->m_overlapping.insert(compB);
						compB->m_overlapping.insert(compA);
						if (!compA->m_oldOverlaps.erase(compB) && !compB->m_oldOverlaps.erase(compA))
						{
							collisionEvents.emplace_back(eCollisionEventType::OverlapBegin, compA, compB);
						}
					}
				}
			}

			if (compA->IsStatic())
			{
				staticSweep.push_back(compA);
			}
			else
			{
				// non-static objects should also test against swept static objects
				for (int q = 0; q < staticSweep.size(); q++)
				{
					CollisionComponent2D* compB = staticSweep[q];
					if (ProcessCollisionPair(compA, compB, collInfo))
					{
						if (collInfo.bBlockingHit)
						{
							collisionEvents.emplace_back(eCollisionEventType::BlockingHit, compA, compB);
						}
						else
						{
							compA->m_overlapping.insert(compB);
							compB->m_overlapping.insert(compA);
							if (!compA->m_oldOverlaps.erase(compB) && !compB->m_oldOverlaps.erase(compA))
							{
								collisionEvents.emplace_back(eCollisionEventType::OverlapBegin, compA, compB);
							}
						}
					}
				}

				nonstaticSweep.push_back(compA);
			}
		}

#elif COLLISION2D_QUADTREE
		// m_componentsDirty contains all CollisionComponents which have moved since the last collision Tick
		// We first relocate these within the QuadTree, then process only these elements (since we do not need to compare two elements which have not moved)
		// If any component is moved as a result of depenetration, their OnMoved callback will add them back into the m_componentsDirty list, meaning they will be processed during the next CollisionTick
		// If we find we need greater accuracy, we could run multiple CollisionTicks per frame, lerping components along their delta since last frame rather than testing their final position
		for (CollisionComponent2D* comp : m_componentsDirty)
		{
			m_quadTree.Relocate(comp->m_quadTreeLocation, QuadTreeRect(comp->GetAABBOrigin(), comp->GetAABBExtent()));
			comp->m_bDirty = false; // setting false ensures any movement applied to the component from here onward triggers its readdition to the m_componentsDirty list
		}

		// Move contents of m_componentsDirty into compsToProcess, then reinitialise m_componentsDirty. 
		// This ensures m_componentsDirty is empty and can be safely added to by any components dirtied during depenetration
		std::vector<CollisionComponent2D*> compsToProcess = std::move(m_componentsDirty);
		m_componentsDirty = {};

		// Test possible pairs
		for (CollisionComponent2D* comp : compsToProcess)
		{
			std::unordered_set<CollisionComponent2D*> oldOverlaps = std::move(comp->m_overlapping);
			comp->m_overlapping = {};

			// Grab all nearby CollisionComponents we need to consider for this component
			std::vector<std::list<QuadTreeItem<CollisionComponent2D*>>::iterator> searchResults;
			m_quadTree.Search(QuadTreeRect(comp->GetAABBOrigin(), comp->GetAABBExtent()), searchResults);

			// Since we don't support Static v Static collision, static comps can skip other static comps
			if (comp->IsStatic())
			{
				for (auto& search : searchResults)
				{
					CollisionComponent2D* nearbyComp = search->item;
					if (nearbyComp == comp)
					{
						continue;
					}

					if(!nearbyComp->IsStatic())
					{
						if (ProcessCollisionPair(comp, nearbyComp, collInfo))
						{
							if (collInfo.bBlockingHit)
							{
								collisionEvents.emplace_back(eCollisionEventType::BlockingHit, comp, nearbyComp);
							}
							else
							{
								comp->m_overlapping.insert(nearbyComp);
								nearbyComp->m_overlapping.insert(comp);
								if (!oldOverlaps.erase(nearbyComp))
								{
									collisionEvents.emplace_back(eCollisionEventType::OverlapBegin, comp, nearbyComp);
								}
							}
						}
					}
				}
			}
			else // non-static comps have to test against both static & non-static
			{
				for (auto& search : searchResults)
				{
					CollisionComponent2D* nearbyComp = search->item;
					if (nearbyComp == comp)
					{
						continue;
					}

					if (ProcessCollisionPair(comp, nearbyComp, collInfo))
					{
						if (collInfo.bBlockingHit)
						{
							collisionEvents.emplace_back(eCollisionEventType::BlockingHit, comp, nearbyComp);
						}
						else
						{
							comp->m_overlapping.insert(nearbyComp);
							nearbyComp->m_overlapping.insert(comp);
							if (!oldOverlaps.erase(nearbyComp))
							{
								collisionEvents.emplace_back(eCollisionEventType::OverlapBegin, comp, nearbyComp);
							}
						}
					}
				}
			}

			for (CollisionComponent2D* oldOverlap : oldOverlaps)
			{
				collisionEvents.emplace_back(eCollisionEventType::OverlapEnd, comp, oldOverlap);
			}
		}
#endif
		END_PROFILE("CollisionSystem2D");

		START_PROFILE("CollisionEvents");
		for (CollisionEvent& event : collisionEvents)
		{
			switch (event.eventType)
			{
				case eCollisionEventType::OverlapBegin:
					event.compA->GetOwner().OnOverlapBegin(event.compB);
					event.compB->GetOwner().OnOverlapBegin(event.compA);
					break;

				case eCollisionEventType::OverlapEnd:
					event.compA->GetOwner().OnOverlapEnd(event.compB);
					event.compB->GetOwner().OnOverlapEnd(event.compA);
					break;

				case eCollisionEventType::BlockingHit:
					event.compA->GetOwner().OnBlockingHit(event.compB);
					event.compB->GetOwner().OnBlockingHit(event.compA);
					break;
			}
		}
		END_PROFILE("CollisionEvents");
	}

	bool CollisionSystem2D::CanBlock(CollisionComponent2D* compA, CollisionComponent2D* compB) const
	{
		return (compA->GetObjectType() & compB->GetBlockingFlags() && compB->GetObjectType() & compA->GetBlockingFlags());
	}

	bool CollisionSystem2D::CanOverlap(CollisionComponent2D* compA, CollisionComponent2D* compB) const
	{
		return (compA->GetObjectType() & compB->GetOverlapFlags() && compB->GetObjectType() & compA->GetOverlapFlags());
	}

	bool CollisionSystem2D::ProcessCollisionPair(CollisionComponent2D* compA, CollisionComponent2D* compB, CollisionInfo& collInfo) const
	{
		if(CanBlock(compA, compB))
		{
			bool bBlocking = false;
			if (compA->GetShape() == eCollisionShape2D::Radial)
			{
				if (compB->GetShape() == eCollisionShape2D::Radial)
				{
					bBlocking = IntersectComponents_CircleCircle(compA, compB, collInfo);
				}
				else
				{
					bBlocking = IntersectComponents_CircleBox(compA, compB, collInfo);
				}
			}
			else
			{
				if (compB->GetShape() == eCollisionShape2D::Radial)
				{
					// compB and compA are switched, so we have to flip outInfo.normal
					bBlocking = IntersectComponents_CircleBox(compB, compA, collInfo);
					collInfo.normal *= -1;
				}
				else
				{
					bBlocking = IntersectComponents_BoxBox(compA, compB, collInfo);
				}
			}
			if (bBlocking)
			{
				collInfo.bBlockingHit = true;
				DepenetrateComponents(compA, compB, collInfo);
			}
			return bBlocking;
		}
		else if (CanOverlap(compA, compB))
		{
			bool bOverlapping = false;
			if (compA->GetShape() == eCollisionShape2D::Radial)
			{
				if (compB->GetShape() == eCollisionShape2D::Radial)
				{
					bOverlapping = OverlapComponents_CircleCircle(compA, compB);
				}
				else
				{
					bOverlapping = OverlapComponents_CircleBox(compA, compB);
				}
			}
			else
			{
				if (compB->GetShape() == eCollisionShape2D::Radial)
				{
					bOverlapping = OverlapComponents_CircleBox(compB, compA);
				}
				else
				{
					bOverlapping = OverlapComponents_BoxBox(compA, compB);
				}
			}
			if (bOverlapping)
			{
				collInfo.bBlockingHit = false;
			}
			return bOverlapping;
		}
		return false;
	}

	bool CollisionSystem2D::IntersectComponents_CircleCircle(CollisionComponent2D* circleA, CollisionComponent2D* circleB, CollisionInfo& outInfo) const
	{
		// Test for separation in the Y axis
		if (CheckSeparation(circleA->GetMinY(), circleA->GetMaxY(), circleB->GetMinY(), circleB->GetMaxY()))
		{
			return false;
		}

#if !COLLISION2D_SWEEP
		// SortAndSweep already tests X separation in the broad phase, other methods need to test it now
		if (CheckSeparation(circleA->GetMinX(), circleA->GetMaxX(), circleB->GetMinX(), circleB->GetMaxX()))
		{
			return false;
		}
#endif

		Vector2f delta = circleB->GetOrigin() - circleA->GetOrigin();
		float separation = delta.Length();
		float radii = circleA->GetRadius() + circleB->GetRadius();
		outInfo.depth = radii - separation;
		if (outInfo.depth > 0)
		{
			outInfo.normal = Normalize(delta);
			return true;
		}
		return false;
	}

	bool CollisionSystem2D::IntersectComponents_BoxBox(CollisionComponent2D* boxA, CollisionComponent2D* boxB, CollisionInfo& outInfo) const
	{
		float aMinY = boxA->GetMinY();
		float aMaxY = boxA->GetMaxY();
		float bMinY = boxB->GetMinY();
		float bMaxY = boxB->GetMaxY();

		// Test for separation in the Y axis
		if (CheckSeparation(aMinY, aMaxY, bMinY, bMaxY))
		{
			return false;
		}

#if !COLLISION2D_SWEEP
		// SortAndSweep tests X separation in the broad phase, other methods need to test it now
		if (CheckSeparation(boxA->GetMinX(), boxA->GetMaxX(), boxB->GetMinX(), boxB->GetMaxX()))
		{
			return false;
		}
#endif

		// Calculate penetration depths
		float penetrationY1 = bMaxY - aMinY;
		float penetrationY2 = aMaxY - bMinY;
		float penetrationY = std::min(penetrationY1, penetrationY2);

		float penetrationX1 = boxB->GetMaxX() - boxA->GetMinX();
		float penetrationX2 = boxA->GetMaxX() - boxB->GetMinX();
		float penetrationX = std::min(penetrationX1, penetrationX2);

		// Return smallest depth & normal able to separate boxes
		if (penetrationX < penetrationY)
		{
			outInfo.normal = Vector2f(penetrationX1 < penetrationX2 ? -1 : 1, 0);
			outInfo.depth = penetrationX;
		}
		else
		{
			outInfo.normal = Vector2f(0, penetrationY1 < penetrationY2 ? -1 : 1);
			outInfo.depth = penetrationY;
		}
		return true;
	}

	bool CollisionSystem2D::IntersectComponents_CircleBox(CollisionComponent2D* circle, CollisionComponent2D* box, CollisionInfo& outInfo) const
	{
		float aMinY = circle->GetMinY();
		float aMaxY = circle->GetMaxY();
		float bMinY = box->GetMinY();
		float bMaxY = box->GetMaxY();

		// Test for separation in the Y axis
		if (CheckSeparation(aMinY, aMaxY, bMinY, bMaxY))
		{
			return false;
		}

#if !COLLISION2D_SWEEP
		// SortAndSweep tests X separation in the broad phase, other methods need to test it now
		if (CheckSeparation(circle->GetMinX(), circle->GetMaxX(), box->GetMinX(), box->GetMaxX()))
		{
			return false;
		}
#endif

		// Check for separating axis between circle & box
		{
			Vector2f axis = Normalize(box->GetOrigin() - circle->GetOrigin());

			float circleMin, circleMax;
			circle->ProjectToAxis(axis, circleMin, circleMax);

			float boxMin, boxMax;
			box->ProjectToAxis(axis, boxMin, boxMax);

			// If separation exists, shapes do not intersect
			if (circleMin >= boxMax || boxMin >= circleMax)
			{
				return false;
			}

			float penetrationA = boxMax - circleMin;
			float penetrationB = circleMax - boxMin;
			float penetration = std::min(penetrationA, penetrationB);

			outInfo.depth = penetration;
			outInfo.normal = axis * (penetrationA < penetrationB ? -1 : 1);
		}

		// Check whether X or Y axes offer smaller separation
		{
			float penetrationY1 = bMaxY - aMinY;
			float penetrationY2 = aMaxY - bMinY;
			float penetrationY = std::min(penetrationY1, penetrationY2);

			float penetrationX1 = box->GetMaxX() - circle->GetMinX();
			float penetrationX2 = circle->GetMaxX() - box->GetMinX();
			float penetrationX = std::min(penetrationX1, penetrationX2);

			// Return smallest depth & normal able to separate boxes
			if (penetrationX < penetrationY)
			{
				if(penetrationX < outInfo.depth)
				{
					outInfo.normal = Vector2f(penetrationX1 < penetrationX2 ? -1 : 1, 0);
					outInfo.depth = penetrationX;
				}
			}
			else // implicit: if (penetrationY <= penetrationX)
			{
				if(penetrationY < outInfo.depth)
				{
					outInfo.normal = Vector2f(0, penetrationY1 < penetrationY2 ? -1 : 1);
					outInfo.depth = penetrationY;
				}
			}
		}

		return true;
	}

	bool CollisionSystem2D::OverlapComponents_CircleCircle(CollisionComponent2D* circleA, CollisionComponent2D* circleB) const
	{
		// Test for separation in the Y axis
		if (CheckSeparation(circleA->GetMinY(), circleA->GetMaxY(), circleB->GetMinY(), circleB->GetMaxY()))
		{
			return false;
		}

#if !COLLISION2D_SWEEP
		// SortAndSweep tests X separation in the broad phase, other methods need to test it now
		if (CheckSeparation(circleA->GetMinX(), circleA->GetMaxX(), circleB->GetMinX(), circleB->GetMaxX()))
		{
			return false;
		}
#endif

		Vector2f delta = circleB->GetOrigin() - circleA->GetOrigin();
		float separation = delta.Length();
		float radii = circleA->GetRadius() + circleB->GetRadius();
		return separation < radii;
	}

	bool CollisionSystem2D::OverlapComponents_BoxBox(CollisionComponent2D* boxA, CollisionComponent2D* boxB) const
	{
		// Test for separation in the Y axis
		if (CheckSeparation(boxA->GetMinY(), boxA->GetMaxY(), boxB->GetMinY(), boxB->GetMaxY()))
		{
			return false;
		}

#if !COLLISION2D_SWEEP
		// SortAndSweep tests X separation in the broad phase, other methods need to test it now
		if (CheckSeparation(boxA->GetMinX(), boxA->GetMaxX(), boxB->GetMinX(), boxB->GetMaxX()))
		{
			return false;
		}
#endif

		return true;
	}

	bool CollisionSystem2D::OverlapComponents_CircleBox(CollisionComponent2D* circle, CollisionComponent2D* box) const
	{
		// Test for separation in the Y axis
		if (CheckSeparation(circle->GetMinY(), circle->GetMaxY(), box->GetMinY(), box->GetMaxY()))
		{
			return false;
		}

#if !COLLISION2D_SWEEP
		// SortAndSweep tests X separation in the broad phase, other methods need to test it now
		if (CheckSeparation(circle->GetMinX(), circle->GetMaxX(), box->GetMinX(), box->GetMaxX()))
		{
			return false;
		}
#endif

		// Check for separating axis between circle & box
		{
			Vector2f axis = Normalize(box->GetOrigin() - circle->GetOrigin());

			float circleMin, circleMax;
			circle->ProjectToAxis(axis, circleMin, circleMax);

			float boxMin, boxMax;
			box->ProjectToAxis(axis, boxMin, boxMax);

			// If separation exists, shapes do not intersect
			if (circleMin >= boxMax || boxMin >= circleMax)
			{
				return false;
			}
		}
		return true;
	}

	void CollisionSystem2D::DepenetrateComponents(CollisionComponent2D* compA, CollisionComponent2D* compB, const CollisionInfo& info) const
	{
		// We never want to depenetrate two static objects, earlier tests in the system should avoid this
		ASSERT(!(compA->IsStatic() && compB->IsStatic()));

		if (!compA->IsStatic() && !compB->IsStatic())
		{
			compA->GetOwner().MovePosition(-info.normal * info.depth * 0.5f);
			compB->GetOwner().MovePosition(info.normal * info.depth * 0.5f);

		}
		else if (compA->IsStatic())
		{
			compB->GetOwner().MovePosition(info.normal * info.depth);
		}
		else
		{
			compA->GetOwner().MovePosition(-info.normal * info.depth);
		}
	}
}