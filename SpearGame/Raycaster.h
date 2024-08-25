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
	int xResolution{900}; // internal resolution for raycaster
	int yResolution{600}; 
	int threads{15};
	int rayEncounterLimit{20}; // how many 'wall encounters' to allow per ray (used for rendering tall walls behind shorter walls)
	int XResolutionPerThread() { return xResolution / threads; }; // intentional integer division
	int YResolutionPerThread() { return yResolution / threads; };

	// Used only for 2D top-down rendering. Scale 1 = 1 tile : 1 pixel.
	float scale2D{ 75.f };

	// Debug settings
	bool highlightCorrectivePixels{ false }; // whether to render corrective pixels as BrightRed instead of using pixel-cloning
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
	static void Init(MapData& map);
	static void ApplyConfig(const RaycastParams& config);

	static void Draw2DLooseWalls(const Vector2f& pos, const float angle, RaycastWall* pWalls, int wallCount);
	static void Draw3DLooseWalls(const Vector2f& pos, const float angle, RaycastWall* pWalls, int wallCount);

	static void Draw2DGrid(const Vector2f& pos, const float angle);
	static void Draw3DGrid(const Vector2f& pos, float pitch, const float angle);

private:
	static void RecreateBackgroundArrays(int width, int height);
	static void ClearBackgroundArrays();

	// Raycast Data
	static RaycastParams m_rayConfig;
	static MapData* m_map;

	// Background Floor/Ceiling Render
	static GLfloat* m_bgTexDepth;
	static GLuint* m_bgTexRGBA;

	// Depth
	static GLfloat m_mapMaxDepth;

	// For storing internal per-frame data
	struct RaycastFrameData
	{
		float		viewPitch;		// 'slope' used for rays sampling floor/ceiling
		float		viewHeight;		// vertical position of camera
		Vector2f	viewPos;		// 2D position of camera (XY)
		Vector2f	viewForward;	// forward vector for camera view
		
		// screen plane (allows even distribution of ray endpoints, radial distribution caused warping due to uneven gap sizes between ray endpoints hitting flat surfaces)
		Vector2f	screenPlaneEdgePositionL;	// position of left edge of camera far clip
		Vector2f	screenPlaneEdgePositionR;	// position of right edge of camera far clip
		Vector2f	screenPlaneVector;			// direction from left to right

		Vector2f	raySpacingDir;		// direction to space rays horizontally for walls
		float		raySpacingLength;	// spacing to use between each ray

		// although we have similar data for wall-rays, we need to know Fov distribution for floor-rays because
		// rays sampled directly in front of us need to be bunched tightly together while distant rays are spread out
		Vector2f	fovMinAngle;	// minimum ray angle based on fov (left edge of 'visual cone')
		Vector2f	fovMaxAngle;	// maximum ray angle based on fov (right edge of 'visual cone')
	};
	static RaycastFrameData m_frame;
};