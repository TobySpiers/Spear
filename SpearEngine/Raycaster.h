#pragma once

namespace Spear
{
	// a 2D map grid
	// Pros: very fast, efficient intersect calculations
	// Cons: only supports static tile grid formations
	struct RaycastDDAGrid
	{
		u8* pRoofIds{nullptr};
		s8* pWorldIds{nullptr};	// positive = wall+floor, negative = floor, 0 = empty
		u8 width{0};
		u8 height{0};
	};

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
		int xResolution{480}; // 480. affects performance for walls + floors + ceilings.
		int yResolution{270}; // 270. affects performance for floors + ceilings. wall y-resolution is faked for aesthetic only.

		// Used only in 2D rendering. Scale 1 = 1 tile : 1 pixel.
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
		static void SubmitNewGrid(u8 width, u8 height, const s8* pWorldIds, const u8* pRoofIds);
		static void ApplyConfig(const RaycastParams& config);

		static void Draw2DWalls(const Vector2f& pos, const float angle, RaycastWall* pWalls, int wallCount);
		static void Draw3DWalls(const Vector2f& pos, const float angle, RaycastWall* pWalls, int wallCount);

		static void Draw2DGrid(const Vector2f& pos, const float angle);
		static void Draw3DGrid(const Vector2f& pos, const float angle);

	private:
		static void RecreateBackgroundArray(int width, int height);
		static void ClearBackgroundArray();

		// Raycast Data
		static RaycastDDAGrid m_ddaGrid;
		static RaycastParams m_rayConfig;

		// Background Floor/Ceiling Render
		static GLfloat* m_bgTexPixels;
		static const int m_bgTexPixelSize{3}; // 3 values: R,G,B
	};
}