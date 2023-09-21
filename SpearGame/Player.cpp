#include "SpearEngine/Core.h"
#include "SpearEngine/ServiceLocator.h"
#include "SpearEngine/InputManager.h"
#include "SpearEngine/LineRenderer.h"

#include "PlayFlowstate.h"
#include "Player.h"

Player::Player()
{
	m_rayParams.fieldOfView = TO_RADIANS(90.f);
}

void Player::SetPos(const Vector2D& pos)
{
	m_pos = pos;
}

void Player::Update()
{
	Spear::InputManager& input = Spear::ServiceLocator::GetInputManager();

	if (input.InputHold(INPUT_UP))
	{
		m_pos = m_pos + Vector2D(sin(m_rotation), cos(m_rotation)) * m_moveSpeed;
	}
	else if (input.InputHold(INPUT_DOWN))
	{
		m_pos = m_pos - Vector2D(sin(m_rotation), cos(m_rotation)) * m_moveSpeed;
	}
	if (input.InputHold(INPUT_LEFT))
	{
		m_pos = m_pos - Vector2D(-cos(m_rotation), sin(m_rotation)) * m_moveSpeed;
	}
	else if (input.InputHold(INPUT_RIGHT))
	{
		m_pos = m_pos + Vector2D(-cos(m_rotation), sin(m_rotation)) * m_moveSpeed;
	}

	if (input.InputHold(INPUT_ROTATE_LEFT))
	{
		m_rotation += m_turnSpeed;
	}
	else if (input.InputHold(INPUT_ROTATE_RIGHT))
	{
		m_rotation -= m_turnSpeed;
	}

	m_rayParams.pos = m_pos;
	m_rayParams.rotation = m_rotation - TO_RADIANS(45.f);
}

void Player::Draw(bool perspective) const
{
	if (perspective)
	{
		Spear::Raycaster::Draw3D(m_rayParams);
	}
	else
	{
		// draw rays
		Spear::Raycaster::Draw2D(m_rayParams);

		// draw player on top
		Spear::LinePolyData poly;
		poly.colour = Red;
		poly.radius = 10.f;
		poly.segments = 3;
		poly.pos = m_pos;
		poly.rotation = m_rotation + TO_RADIANS(45.f);
		Spear::ServiceLocator::GetLineRenderer().AddLinePoly(poly);
	}	
}

void Player::RegisterWalls(Spear::RaycastWall* walls, int size)
{
	m_rayParams.pWalls = walls;
	m_rayParams.wallCount = size;
}