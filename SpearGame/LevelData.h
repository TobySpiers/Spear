#pragma once
#include "SpearEngine/Core.h"

constexpr int MAP_WIDTH_MAX_SUPPORTED{ 40 };
constexpr int MAP_HEIGHT_MAX_SUPPORTED{ 40 };

enum eLevelTextures
{
	TEX_NONE = -1,
};

enum eCollisionMask : u8
{
	COLL_NONE = 0,

	COLL_WALL	= 1 << 0,
	COLL_SOLID	= 1 << 1,
};

enum class eCollisionSearch
{
	WALL_TEXTURE,
	COLLISION
};

struct GridNode
{
	int texIdRoof{ TEX_NONE };
	int texIdWall{ TEX_NONE };
	int texIdFloor{ TEX_NONE };

	int extendUp{ 0 };				// additional units for walls above... uses texIdRoof if set, otherwise uses texIdWall
	int extendDown{ 0 };			// additional units for walls below... uses texIdFloor if set, otherwise uses texIdWall

	int collisionMask{ COLL_NONE };

	void Reset();
};

struct EditorMapData
{
	int gridWidth{ 10 };
	int gridHeight{ 10 };
	GridNode gridNodes[MAP_WIDTH_MAX_SUPPORTED * MAP_HEIGHT_MAX_SUPPORTED];

	void SetSize(int width, int height) { gridWidth = width; gridHeight = height; }
	GridNode& GetNode(int x, int y) { ASSERT(x < gridWidth && y < gridHeight && x >= 0 && y >= 0); return gridNodes[x + (y * MAP_WIDTH_MAX_SUPPORTED)]; }
	void ClearData();
};


struct MapData
{
	const GridNode* GetNode(Vector2i index);
	const GridNode* GetNode(int x, int y);

	bool CollisionSearchDDA(const Vector2f& start, const Vector2f& trajectory, u8 collisionTestMask, Vector2f* out_hitPos = nullptr, bool* out_bVerticalHit = nullptr);

	int gridWidth{10};
	int gridHeight{10};
	GridNode* pNodes{nullptr};
};