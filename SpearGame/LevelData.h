#pragma once
#include "SpearEngine/Core.h"

constexpr int MAP_WIDTH_MAX_SUPPORTED{ 40 };
constexpr int MAP_HEIGHT_MAX_SUPPORTED{ 40 };

enum eLevelTextures
{
	TEX_NONE = -1,

	TEX_STONE,
	TEX_WOOD,

	TEX_TOTAL
};

enum eCollisionType
{
	COLL_NONE,
	COLL_SOLID,
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
	int gridWidth{10};
	int gridHeight{10};
	GridNode* pNodes{nullptr};
};

//struct RaycastDDAGrid
//{
//	u8* pRoofIds{ nullptr };
//	s8* pWorldIds{ nullptr };	// positive = wall+floor, negative = floor, 0 = empty
//	u8 width{ 0 };
//	u8 height{ 0 };
//};
