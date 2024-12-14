#pragma once
#include "LevelData.h"
#include "PanelRaycaster.h"

struct RaycasterConfig;

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
	static RaycasterConfig GetConfigCopy();
	static void ApplyConfig(const RaycasterConfig& config);
	static void ApplyFovModifier(float newFov);
	static Vector2i GetResolution();

	static void Draw2DGrid(const Vector2f& pos, const float angle);
	static void Draw3DGrid(const Vector2f& pos, float pitch, const float angle);

private:
	static void RecreateBackgroundArrays(int width, int height);
	static void ClearBackgroundArrays();

	// ImGui Panel
	static PanelRaycaster debugPanel;

	// Raycast Data
	static RaycasterConfig m_rayConfig;
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
		Vector2f	fovMinAngle;		// minimum ray angle based on fov (left edge of 'visual cone')
		Vector2f	fovMaxAngle;		// maximum ray angle based on fov (right edge of 'visual cone')

		float		fov;				// actual fov (calculated as sum of fovBase + fovModifier)
		float		fovWallMultiplier;	// height modifier applied to walls to prevent floor/roof separation when adjusting fov
	};
	static RaycastFrameData m_frame;
};