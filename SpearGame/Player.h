#pragma once
#include "SpearEngine/Raycaster.h"

class Player
{
public:
	Player();

	void SetPos(const Vector2f& pos){m_pos = pos;};
	void SetSpeed(float speed){m_moveSpeed = speed;};
	void Update(float deltaTime);
	void Draw(bool perspective) const;

	void RegisterWalls(Spear::RaycastWall* pWalls, int size);
	void RegisterGrid(Spear::RaycastDDAGrid* pGrid){m_pRayGrid = pGrid;};

private:
	Spear::RaycastParams m_rayParams;
	Vector2f m_pos{0.f, 0.f};
	float m_moveSpeed{2.f};
	float m_rotation{TO_RADIANS(-90.f)};
	float m_turnSpeed{TO_RADIANS(5.f)};

	Spear::RaycastDDAGrid* m_pRayGrid{nullptr};

	Spear::RaycastWall* m_pWalls{nullptr};
	int m_wallCount{0};
};

