#include "LevelData.h"
#include <Collision/CollisionComponent2D.h>

const int MapData::TotalNodes() const
{
	return gridWidth * gridHeight;
}

const GridNode* MapData::GetNode(Vector2i index) const
{
	return GetNode(index.x, index.y);
}

const GridNode* MapData::GetNode(int x, int y) const
{
	if (x >= 0 && x < gridWidth && y >= 0 && y < gridHeight)
	{
		return &pNodes[x + (y * gridWidth)];
	}
	return nullptr;
}

Vector2i MapData::GetExitTileForConjoinedPortal(Vector2i entryTile, bool bScanY) const
{
	// If multiple MirrorPortalConjoined are touching, combine them into a single inverted mirror so the image isn't split up
	int portalLength = 0;
	int entryTilePosition = 0;
	Vector2i nextConjoinedPortalStep = bScanY ? Vector2i(0, 1) : Vector2i(1, 0);
								
	// Discover how big this conjoined portal is, and where the tile our ray has hit lies within it
	Vector2i mapCheck = entryTile;
	mapCheck -= nextConjoinedPortalStep;
	while (const GridNode* portalNode = GetNode(mapCheck))
	{
		if (portalNode->specialFlag == SPECIAL_MIRROR_PORTAL_CONJOINED)
		{
			entryTilePosition++;
			mapCheck -= nextConjoinedPortalStep;
		}
		else break;
	}
	portalLength += entryTilePosition;
	mapCheck = entryTile;
	mapCheck += nextConjoinedPortalStep;
	while (const GridNode* portalNode = GetNode(mapCheck))
	{
		if (portalNode->specialFlag == SPECIAL_MIRROR_PORTAL_CONJOINED)
		{
			portalLength++;
			mapCheck += nextConjoinedPortalStep;
		}
		else break;
	}
	
	// portalLength tells us how many portals are touching which we should consider a single mirror
	// entryTilePosition tells us the index into those portals of the entryTile, we use this to invert the exit: if we entered at position 0/5, we would exit at position 5/5. 1/5 -> 4/5, etc.
	int portalOutPosition = portalLength - entryTilePosition;
	mapCheck = entryTile;
	bScanY ? mapCheck.y = (mapCheck.y - entryTilePosition) + portalOutPosition : mapCheck.x = (mapCheck.x - entryTilePosition) + portalOutPosition;
	return mapCheck;
}

Vector2f MapData::PreCheckedMovement(const Vector2f& start, const Vector2f& trajectory, CollisionComponent2D* collisionComp, float& outRotationOffset) const
{
	return PreCheckedMovement(start, trajectory, collisionComp->GetAABBExtent(), collisionComp->GetBlockingFlags(), outRotationOffset);
}

Vector2f MapData::PreCheckedMovement(const Vector2f& start, const Vector2f& trajectory, const Vector2f& AABB, u8 collisionMask, float& outRotationOffset) const
{
	outRotationOffset = 0.f;
	
	Vector2f traceTrajectory{trajectory};
	Vector2f AABBHalfX = Vector2f(AABB.x / 2.f, 0.f);
	Vector2f AABBHalfY = Vector2f(0.f, AABB.y / 2.f);
	
	auto portalPredicate = [](const GridNode& node) {return node.specialFlag == SPECIAL_MIRROR_PORTAL || node.specialFlag == SPECIAL_MIRROR_PORTAL_CONJOINED;};
	auto collisionPredicate = [collisionMask](const GridNode& node) { return node.collisionMask & collisionMask;};
	
	Vector2f curPosition{start};
	Vector2f nextPosition{start};
	
	LineSearchData search;
	bool bCrossedPortal;
	do
	{
		// Find the position our AABB is able to reach.
		Vector2f trajectoryX = Vector2f(traceTrajectory.x + (Sign(traceTrajectory.x) * AABBHalfX.x), 0.f);
		if (!LineSearchDDA(curPosition + AABBHalfY, curPosition + AABBHalfY + trajectoryX, collisionPredicate)
		&& !LineSearchDDA(curPosition - AABBHalfY, curPosition - AABBHalfY + trajectoryX, collisionPredicate))
		{
			nextPosition.x += traceTrajectory.x;
		}
		
		Vector2f trajectoryY = Vector2f(0.f, traceTrajectory.y + (Sign(traceTrajectory.y) * AABBHalfY.y));
		if (!LineSearchDDA(curPosition + AABBHalfX, curPosition + AABBHalfX + trajectoryY, collisionPredicate)
		&& !LineSearchDDA(curPosition - AABBHalfX, curPosition - AABBHalfX + trajectoryY, collisionPredicate))
		{
			nextPosition.y += traceTrajectory.y;
		}
	
		// Trace center-of-AABB until destination to check if it crossed a portal threshold.
		bCrossedPortal = false;
		if (LineSearchDDA(curPosition, nextPosition, portalPredicate, &search))
		{
			bCrossedPortal = true;
			
			// There was a portal in our path. Prepare for the next phase of the trace. Reflect our ray.
			outRotationOffset += PI;
			traceTrajectory *= (1.f - search.percentComplete);
	
			if (search.node->specialFlag == SPECIAL_MIRROR_PORTAL_CONJOINED)
			{
				const Vector2i exitTile = GetExitTileForConjoinedPortal(search.tile, !search.bVerticalHit);
				search.hitPos += (exitTile - search.tile).ToFloat(); // update hitPos to represent the exit tile instead of the entry tile
			}
			if (search.bVerticalHit)
			{
				traceTrajectory.y *= -1;
				int truncX = static_cast<int>(search.hitPos.x);
				search.hitPos.x = truncX + (1 - (search.hitPos.x - truncX)); // mirror portals are inverted, so we flip positioning such that 0.25 becomes 0.75, 0.66 becomes 0.33, etc.
			}
			else
			{
				traceTrajectory.x *= -1;
				int truncY = static_cast<int>(search.hitPos.y);
				search.hitPos.y = truncY + (1 - (search.hitPos.y - truncY));
			}
			
			curPosition = search.hitPos;
			nextPosition = curPosition;
		}
	}
	while (bCrossedPortal);

	return nextPosition;
}

void GridNode::Reset()
{
	texIdRoof[0] = eLevelTextures::TEX_NONE;
	texIdRoof[1] = eLevelTextures::TEX_NONE;
	texIdWall = eLevelTextures::TEX_NONE;
	texIdFloor[0] = eLevelTextures::TEX_NONE;
	texIdFloor[1] = eLevelTextures::TEX_NONE;
	extendUp = 0;
	extendDown = 0;
	collisionMask = 0;
}

bool GridNode::CompareNodeByTexture(const GridNode& other)
{
	return texIdWall == other.texIdWall
		&& texIdFloor[0] == other.texIdFloor[0]
		&& texIdFloor[1] == other.texIdFloor[1]
		&& texIdRoof[0] == other.texIdRoof[0]
		&& texIdRoof[1] == other.texIdRoof[1];
}

void EditorMapData::SetSize(int width, int height)
{
	gridWidth = std::max(3, width); 
	gridHeight = std::max(3, height);
}
