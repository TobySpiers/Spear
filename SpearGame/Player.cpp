#include "SpearEngine/Core.h"
#include "SpearEngine/ServiceLocator.h"
#include "SpearEngine/InputManager.h"
#include "SpearEngine/LineRenderer.h"

#include "PlayFlowstate.h"
#include "Player.h"

Player::Player()
{
}

void Player::Update(float deltaTime)
{
	Spear::InputManager& input = Spear::ServiceLocator::GetInputManager();

	if (input.InputHold(INPUT_UP))
	{
		m_pos = m_pos + Vector2f(cos(m_rotation), sin(m_rotation)) * m_moveSpeed * deltaTime;
	}
	else if (input.InputHold(INPUT_DOWN))
	{
		m_pos = m_pos - Vector2f(cos(m_rotation), sin(m_rotation)) * m_moveSpeed * deltaTime;
	}
	if (input.InputHold(INPUT_LEFT))
	{
		m_pos = m_pos - Vector2f(-sin(m_rotation), cos(m_rotation)) * m_moveSpeed * deltaTime;
	}
	else if (input.InputHold(INPUT_RIGHT))
	{
		m_pos = m_pos + Vector2f(-sin(m_rotation), cos(m_rotation)) * m_moveSpeed * deltaTime;
	}

	if (input.InputHold(INPUT_ROTATE_LEFT))
	{
		m_rotation -= m_turnSpeed;
	}
	else if (input.InputHold(INPUT_ROTATE_RIGHT))
	{
		m_rotation += m_turnSpeed;
	}

	m_rayParams.pos = m_pos;
	m_rayParams.rotation = m_rotation;
}

void Player::Draw(bool perspective) const
{
	if (perspective)
	{
		// raycasting render
		//Spear::Raycaster::Draw3DWalls(m_rayParams, m_pWalls, m_wallCount);

		Spear::Raycaster::Draw3DGrid(m_rayParams, m_pRayGrid);
	}
	else
	{
		// draw 2D
		//Spear::Raycaster::Draw2DWalls(m_rayParams, m_pWalls, m_wallCount);
		Spear::Raycaster::Draw2DGrid(m_rayParams, m_pRayGrid);
	}	
}

void Player::RegisterWalls(Spear::RaycastWall* walls, int size)
{
	m_pWalls = walls;
	m_wallCount = size;
}