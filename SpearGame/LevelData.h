#pragma once
#include "SpearEngine/Core.h"

constexpr int MAP_WIDTH_MAX_SUPPORTED{ 40 };
constexpr int MAP_HEIGHT_MAX_SUPPORTED{ 40 };

enum eLevelTextures
{
	TEX_NONE = -1,
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

enum eCollisionMask : u8
{
	COLL_NONE = 0,

	COLL_WALL	= 1 << 0,
	COLL_SOLID	= 1 << 1,
};

struct GridNode
{
	int texIdRoof[2]{ TEX_NONE, TEX_NONE };
	int texIdWall{ TEX_NONE };
	int texIdFloor[2]{ TEX_NONE, TEX_NONE };
	u8 drawFlags{ DRAW_DEFAULT };

	int extendUp{ 0 };				// additional units for walls above... uses texIdRoof if set, otherwise uses texIdWall
	int extendDown{ 0 };			// additional units for walls below... uses texIdFloor if set, otherwise uses texIdWall

	int collisionMask{ COLL_NONE };

	void Reset();
};

struct EditorMapData
{
	Vector2i playerStart{ 5, 5 };
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

	Vector2i playerStart{ 5, 5 };
	int gridWidth{10};
	int gridHeight{10};
	GridNode* pNodes{nullptr};
};