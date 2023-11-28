#pragma once
#include "LevelData.h"

// a loose 2D wall
// Pros: freely positionable, any angle, supports dynamic movement/rotation
// Cons: much slower
// Note: potential for ray-reflections/mirrors...?
struct RaycastWall
{
	Colour4f colour;
	Vector2f origin;
	Vector2f vec;
};

struct RaycastParams
{
	float fieldOfView{ TO_RADIANS(75.f) };
	float farClip{50};
	int xResolution{300}; // affects performance for walls + floors + ceilings.
	int yResolution{250}; // affects performance for floors + ceilings.

	// Used only for 2D top-down rendering. Scale 1 = 1 tile : 1 pixel.
	float scale2D{ 75.f };
};

// class to cast and render rays
class Raycaster
{
	NO_CONSTRUCT(Raycaster);

	enum eRayHit
	{
		RAY_NOHIT = 0,
		RAY_HIT_FRONT,
		RAY_HIT_SIDE
	};

public:
	//static void SubmitNewGrid(u8 width, u8 height, const s8* pWorldIds, const u8* pRoofIds);
	static void LoadLevel(const char* filename);
	static void ApplyConfig(const RaycastParams& config);

	static void Draw2DWalls(const Vector2f& pos, const float angle, RaycastWall* pWalls, int wallCount);
	static void Draw3DWalls(const Vector2f& pos, const float angle, RaycastWall* pWalls, int wallCount);

	static void Draw2DGrid(const Vector2f& pos, const float angle);
	static void Draw3DGrid(const Vector2f& pos, float pitch, const float angle);

private:
	static void RecreateBackgroundArrays(int width, int height);
	static void ClearBackgroundArrays();

	// Raycast Data
	static MapData m_map;
	static RaycastParams m_rayConfig;

	// Background Floor/Ceiling Render
	static GLfloat* m_bgTexDepth;
	static GLuint* m_bgTexRGBA;

	// Depth
	static GLfloat m_mapMaxDepth;
};