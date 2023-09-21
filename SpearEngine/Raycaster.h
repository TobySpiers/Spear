#pragma once

namespace Spear
{
	// a 2D wall
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
		float viewDistance{ 1000 };
		float fieldOfView{ TO_RADIANS(360.f) };
		float rotation{ 0.f };
		int rayCount{ 800 };

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