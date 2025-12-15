#pragma once
#include "Core/Core.h"
#include <filesystem>

constexpr int MAP_WIDTH_MAX_SUPPORTED{ 40 };
constexpr int MAP_HEIGHT_MAX_SUPPORTED{ 40 };

class CollisionComponent2D;

enum eLevelTextures
{
	TEX_MULTI = -2,
	TEX_NONE = -1
};

enum eDrawFlags : u8
{
	// default mode: draws any side facing camera
	DRAW_DEFAULT = 0,

	// manual flags: only draws enabled sides facing camera
	DRAW_N = 1 << 0, 
	DRAW_E = 1 << 1,
	DRAW_S = 1 << 2,
	DRAW_W = 1 << 3,
};

enum eSpecialMapFlags : u8
{
	SPECIAL_NONE = 0,
	
	SPECIAL_MIRROR,
	SPECIAL_MIRROR_PORTAL,
	SPECIAL_MIRROR_PORTAL_CONJOINED,
	
	SPECIAL_TOTAL
};

enum ePlaneHeight : u8
{
	PLANE_HEIGHT_OUTER,
	PLANE_HEIGHT_INNER,

	PLANE_HEIGHTS_TOTAL
};

struct GridNode
{
	// CAUTION - CHANGES MADE TO THIS STRUCT MUST BE REFLECTED IN RAYCASTER COMPUTE SHADER FILES
	int texIdRoof[2]{ TEX_NONE, TEX_NONE };
	int texIdWall{ TEX_NONE };
	int texIdFloor[2]{ TEX_NONE, TEX_NONE };
	int drawFlags{ DRAW_DEFAULT };

	int extendUp{ 0 };				// additional units for walls above... uses texIdRoof if set, otherwise uses texIdWall
	int extendDown{ 0 };			// additional units for walls below... uses texIdFloor if set, otherwise uses texIdWall

	int collisionMask{ 0 };
	int specialFlag{ SPECIAL_NONE };

	void Reset();

	// Returns true if textures assigned to each node match
	bool CompareNodeByTexture(const GridNode& other);
};

struct MapDataBase
{
	MapDataBase() {}
	MapDataBase(const char* name) : mapName(name) {}

	std::string mapName{ "Untitled" };
	std::filesystem::directory_entry spriteDirectory{ "Null "};
	std::filesystem::directory_entry tileDirectory{ "Null "};

	Vector2i playerStart{ 5, 5 };
	int gridWidth{ 10 };
	int gridHeight{ 10 };
	float planeHeights[PLANE_HEIGHTS_TOTAL] = { 2.f, 1.f };

	float darkness{3.f};
};

struct EditorMapData : public MapDataBase
{
	EditorMapData(const char* name) : MapDataBase(name) {}

	void SetSize(int width, int height);
	GridNode& GetNode(int x, int y) { ASSERT(x < gridWidth && y < gridHeight && x >= 0 && y >= 0); return gridNodes[x + (y * MAP_WIDTH_MAX_SUPPORTED)]; }
	GridNode& GetNode(const Vector2i& pos) { return GetNode(pos.x, pos.y); }

	GridNode gridNodes[MAP_WIDTH_MAX_SUPPORTED * MAP_HEIGHT_MAX_SUPPORTED];
};


struct LineSearchData
{
	const GridNode* node{nullptr}; // the node which was hit
	Vector2i tile{Vector2i::ZeroVector}; // the tilegrid location of the node which was hit
	Vector2f hitPos{Vector2f::ZeroVector}; // the exact location of the hit
	bool bVerticalHit{false}; // whether the node was hit along a Y-facing edge
	float percentComplete{0.f}; // percentage (0.0 to 1.0) along length of ray when hit occurred
};

struct MapData : public MapDataBase
{
	const int TotalNodes() const;
	const GridNode* GetNode(Vector2i index) const;
	const GridNode* GetNode(int x, int y) const;
	Vector2i GetExitTileForConjoinedPortal(Vector2i entryTile, bool bScanY) const;

	Vector2f PreCheckedMovement(const Vector2f& start, const Vector2f& trajectory, CollisionComponent2D* collisionComp, float& outRotationOffset) const;
	Vector2f PreCheckedMovement(const Vector2f& start, const Vector2f& traceTrajectory, const Vector2f& AABB, u8 collisionMask, float& outRotationOffset) const;
	
	GridNode* pNodes{nullptr};
	
	// Returns true if tile is encountered for which predicate returns true while performing DDA traversal. Returns false if end is reached with no encounter.
	template <typename Predicate>
	bool LineSearchDDA(const Vector2f& start, const Vector2f& end, Predicate predicate, LineSearchData* outSearchData = nullptr) const
	{
		const Vector2f trajectory = end - start;
		const float distanceLimit{ trajectory.Length() };
		if (distanceLimit <= 0.f)
		{
			return false;
		}
		
		const Vector2f direction = Normalize(trajectory);
		const Vector2f stepSize{sqrt(1 + (direction.y / direction.x) * (direction.y / direction.x)),	// length required to travel 1 X unit in Ray Direction
							sqrt(1 + (direction.x / direction.y) * (direction.x / direction.y)) };	// length required to travel 1 Y unit in Ray Direction

		Vector2i mapCheck = start.ToInt(); // truncation will 'snap' position to tile
		Vector2f testProgression; // tracks accumulating length of 'test' ray: via x units, via y units

		// 1. Calculate initial direction/distance values
		Vector2i step;
		if (direction.x < 0)
		{
			step.x = -1;
			testProgression.x = (start.x - static_cast<float>(mapCheck.x)) * stepSize.x;
		}
		else
		{
			step.x = 1;
			testProgression.x = (static_cast<float>(mapCheck.x + 1) - start.x) * stepSize.x;

		}
		if (direction.y < 0)
		{
			step.y = -1;
			testProgression.y = (start.y - static_cast<float>(mapCheck.y)) * stepSize.y;
		}
		else
		{
			step.y = 1;
			testProgression.y = (static_cast<float>(mapCheck.y + 1) - start.y) * stepSize.y;
		}

		// 2. Search step-by-step for a collision
		while (true)
		{
			bool bVerticalHit;
			float distance{ 0.f };
			if (testProgression.x < testProgression.y)
			{
				// X distance is currently shortest, increase X
				mapCheck.x += step.x;
				distance = testProgression.x;
				testProgression.x += stepSize.x; // increase ray by 1 X unit
				bVerticalHit = false;
			}
			else
			{
				// Y distance is currently shortest, increase Y
				mapCheck.y += step.y;
				distance = testProgression.y;
				testProgression.y += stepSize.y; // increase ray by 1 Y unit
				bVerticalHit = true;
			}

			// If we've reached our distance limit without finding a collision, then ray does not collide. This also means outPercentComplete is always less than 1.f if we return a hit.
			if (distance >= distanceLimit)
			{
				return false;
			}

			// Check position is within range of array
			if (const GridNode* node = GetNode(mapCheck))
			{				
				// Compare node using predicate function supplied by caller
				if (predicate(*node))
				{
					if (outSearchData)
					{						
						outSearchData->bVerticalHit = bVerticalHit;
						outSearchData->hitPos = start + (direction * distance);
						outSearchData->tile = mapCheck;
						outSearchData->node = node;
						outSearchData->percentComplete = distance / distanceLimit;
					}
					return true;
				}
			}
			else
			{
				// If we've left the grid, exit early
				return false;
			}
		}
	}
};