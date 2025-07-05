#pragma once
#include "LevelData.h"
#include "PanelRaycaster.h"

struct RaycasterConfig;

struct RaycastSprite
{
	Vector2f spritePos{ Vector2f::ZeroVector };
	Vector2i size{ 500, 500 };
	int height{ 0 };
	int textureId{ 0 };
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

	static constexpr int RAYCAST_SPRITE_LIMIT{ 100 };

public:
	//static void SubmitNewGrid(u8 width, u8 height, const s8* pWorldIds, const u8* pRoofIds);
	static void Init(MapData& map);
	static RaycasterConfig GetConfigCopy();
	static void ApplyConfig(const RaycasterConfig& config);
	static void ApplyFovModifier(float newFov);
	static Vector2i GetResolution();
	static MapData* GetMap();

	static RaycastSprite* CreateSprite(int textureId, const Vector2f& pos);
	static void ClearSprite(RaycastSprite* sprite);

	static void Draw2DGrid(const Vector2f& pos, const float angle);
	static void Draw3DGrid(const Vector2f& pos, float pitch, const float angle);

private:
	static void RecreateBackgroundArrays(int width, int height);
	static void ClearRaycasterArrays();

	static void Draw3DGridCPU(const Vector2f& pos, float pitch, const float angle);
	static void Draw3DGridCompute(const Vector2f& pos, float pitch, const float angle);

	static void Draw3DSprites(const Vector2f& pos, float pitch, const float angle);

	// ImGui Panel
	static PanelRaycaster debugPanel;

	// (INPUT) Raycast Data
	static RaycasterConfig m_rayConfig;
	static MapData* m_map;

	// (OUTPUT) Depth/Texture Data
	static GLfloat* m_bgTexDepth;
	static GLuint* m_bgTexRGBA;
	
	// (RENDERING SETTINGS)
	friend class PanelRaycaster;
	static bool m_bSoftwareRendering;
	static int m_softwareRenderingThreads;
	static int XResolutionPerThread();
	static int YResolutionPerThread();

	static RaycastSprite m_sprites[RAYCAST_SPRITE_LIMIT];
	static int m_spriteCount;

	// For storing internal per-frame data
	struct RaycastFrameData
	{
		// CAUTION - CHANGES MADE TO THIS STRUCT MUST BE REFLECTED IN RAYCASTER COMPUTE SHADER
		// ===================================================================================

		float		viewPitch;		// 'slope' used for rays sampling floor/ceiling
		float		viewHeight;		// vertical position of camera
		float		fov;				// actual fov (calculated as sum of fovBase + fovModifier)
		float		fovWallMultiplier;	// height modifier applied to walls to prevent floor/roof separation when adjusting fov

		Vector2f	viewPos;		// 2D position of camera (XY)
		Vector2f	viewForward;	// forward vector for camera view
		
		// screen plane (allows even distribution of ray endpoints, radial distribution caused warping due to uneven gap sizes between ray endpoints hitting flat surfaces)
		Vector2f	screenPlaneEdgePositionL;	// position of left edge of camera far clip
		Vector2f	screenPlaneEdgePositionR;	// position of right edge of camera far clip
		Vector2f	screenPlaneVector;			// direction from left to right

		// although we have similar data for wall-rays, we need to know Fov distribution for floor-rays because
		// rays sampled directly in front of us need to be bunched tightly together while distant rays are spread out
		Vector2f	fovMinAngle;		// minimum ray angle based on fov (left edge of 'visual cone')
		Vector2f	fovMaxAngle;		// maximum ray angle based on fov (right edge of 'visual cone')
		
		Vector2f	raySpacingDir;		// direction to space rays horizontally for walls
		float		raySpacingLength;	// spacing to use between each ray
		
		float		padding;
		Vector2f	planeHeights;	// height of inner and outer floor/roof planes
	};
	static RaycastFrameData m_frame;

	struct RaycastComputeShader
	{
		bool isInitialised{ false };

		GLuint gridnodesSSBO{ 0 }; // SSBO - Shader Storage Buffer Object
		GLuint rayconfigUBO{ 0 }; // UBO - Uniform Buffer Object
		GLuint framedataUBO{0};

		// 0 - Planes, 1 - Walls
		static const int programSize{2};
		GLuint program[programSize];
		GLuint gridDimensionsLoc[programSize];
		GLuint worldTexturesLoc[programSize];
	};
	static RaycastComputeShader m_computeShader;
};