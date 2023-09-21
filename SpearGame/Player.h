#pragma once
#include "SpearEngine/Raycaster.h"

class Player
{
public:
	Player();

	void SetPos(const Vector2D& pos);
	void Update();
	void Draw() const;
	void RegisterWalls(Spear::RaycastWall* pWalls, int size);

private:
	Spear::RaycastParams m_rayParams;
	Vector2D m_pos{0.f, 0.f};
	float m_rotation{TO_RADIANS(180.f)};
	float m_moveSpeed{10.f};
	float m_turnSpeed{TO_RADIANS(5.f)};
};

