#pragma once
#include "Core/Core.h"

constexpr int MAP_WIDTH_MAX_SUPPORTED{ 40 };
constexpr int MAP_HEIGHT_MAX_SUPPORTED{ 40 };

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

enum eCollisionMask : int
{
	COLL_NONE = 0,

	COLL_WALL	= 1 << 0,
	COLL_SOLID	= 1 << 1,
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

	int collisionMask{ COLL_NONE };

	void Reset();

	// Returns true if textures assigned to each node match
	bool CompareNodeByTexture(const GridNode& other);
};

struct EditorMapData
{
	EditorMapData(const char* name) : mapName(name) {};

	std::string mapName{ "Untitled" };
	Vector2i playerStart{ 5, 5 };
	int gridWidth{ 10 };
	int gridHeight{ 10 };
	float planeHeights[PLANE_HEIGHTS_TOTAL] = { 2.f, 1.f };
	GridNode gridNodes[MAP_WIDTH_MAX_SUPPORTED * MAP_HEIGHT_MAX_SUPPORTED];

	void SetSize(int width, int height);
	GridNode& GetNode(int x, int y) { ASSERT(x < gridWidth && y < gridHeight && x >= 0 && y >= 0); return gridNodes[x + (y * MAP_WIDTH_MAX_SUPPORTED)]; }
	GridNode& GetNode(const Vector2i& pos) { return GetNode(pos.x, pos.y); }
};

struct MapData
{
	const int TotalNodes() const;
	const GridNode* GetNode(Vector2i index) const;
	const GridNode* GetNode(int x, int y) const;
	bool CollisionSearchDDA(const Vector2f& start, const Vector2f& trajectory, u8 collisionTestMask, Vector2f* out_hitPos = nullptr, bool* out_bVerticalHit = nullptr) const;

	std::string mapName{ "Untitled" };
	Vector2i playerStart{ 5, 5 };
	int gridWidth{10};
	int gridHeight{10};
	float planeHeights[PLANE_HEIGHTS_TOTAL] = { 2.f, 1.f };
	GridNode* pNodes{nullptr};
};