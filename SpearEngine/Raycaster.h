#pragma once

namespace Spear
{
	// a 2D map grid
	// Pros: very fast, efficient intersect calculations
	// Cons: only supports 'square tile' grid formations
	struct RaycastDDAGrid
	{

	};

	// a loose 2D wall
	// Pros: freely positionable, any angle
	// Cons: much slower
	// Note: potential for ray-reflections/mirrors...?
	struct RaycastWall
	{
		Colour colour;
		Vector2D origin;
		Vector2D vec;

		void Draw();
	};

	struct RaycastParams
	{
		Vector2D pos{ 0.f, 0.f };
		float fieldOfView{ TO_RADIANS(360.f) };
		float rotation{ 0.f };
		float depthCorrection{0.005f};

		float nearClip{5};
		float farClip{ 5000 };
		int resolution{ 200 };

		RaycastWall* pWalls{ nullptr };
		int wallCount{ 0 };
	};

	// class to cast and render rays
	class Raycaster
	{
		NO_CONSTRUCT(Raycaster);

	public:
		static void Draw2D(const RaycastParams& param);
		static void Draw3D(const RaycastParams& param);
	};
}