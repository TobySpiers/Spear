#pragma once

namespace Spear
{
	// a 2D map grid
	// Pros: very fast, efficient intersect calculations
	// Cons: only supports 'square tile' grid formations
	struct RaycastDDAGrid
	{
		u8* pValues{nullptr};
		u8 width{0};
		u8 height{0};
	};

	// a loose 2D wall
	// Pros: freely positionable, any angle
	// Cons: much slower
	// Note: potential for ray-reflections/mirrors...?
	struct RaycastWall
	{
		Colour colour;
		Vector2f origin;
		Vector2f vec;
	};

	struct RaycastParams
	{
		Vector2f pos{ 0.f, 0.f };
		float fieldOfView{ TO_RADIANS(75.f) };
		float rotation{ 0.f };
		float depthCorrection{0.75f};

		float nearClip{5};
		float farClip{50};
		int rayResolution{200};
		int texResolution{8};

		// 2D is rendered 1 pixel per tile... this zooms 2D rendering
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
		static void Draw2DWalls(const RaycastParams& param, RaycastWall* pWalls, int wallCount);
		static void Draw3DWalls(const RaycastParams& param, RaycastWall* pWalls, int wallCount);

		static void Draw2DGrid(const RaycastParams& param, RaycastDDAGrid* pGrid);
		static void Draw3DGrid(const RaycastParams& param, RaycastDDAGrid* pGrid);
	};
}