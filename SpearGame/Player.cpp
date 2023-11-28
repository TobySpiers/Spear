#include "SpearEngine/Core.h"
#include "SpearEngine/ServiceLocator.h"
#include "SpearEngine/InputManager.h"
#include "SpearEngine/ScreenRenderer.h"

#include "FlowstateGame.h"
#include "Player.h"

Player::Player()
{
}

void Player::Update(float deltaTime)
{
	Spear::InputManager& input = Spear::ServiceLocator::GetInputManager();

	// movement
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

	// mouse look
	m_rotation += (input.GetMouseAxis().x * 0.005f);
 	m_pitch += (input.GetMouseAxis().y * 0.005f);
	m_pitch = std::min(std::max(m_pitch, -.5f), .5f); // cap pitch at half-up and half-down (greater angles reveal psuedo-3d-ness)

	// retro look
	if (input.InputHold(INPUT_ROTATE_LEFT))
	{
		m_rotation -= m_turnSpeed;
	}
	else if (input.InputHold(INPUT_ROTATE_RIGHT))
	{
		m_rotation += m_turnSpeed;
	}
}

void Player::Draw(bool perspective) const
{
	if (perspective)
	{
		//Spear::Raycaster::Draw3DWalls(m_rayParams, m_pWalls, m_wallCount);
		Raycaster::Draw3DGrid(m_pos, m_pitch, m_rotation);
	}
	else
	{
		//Spear::Raycaster::Draw2DWalls(m_rayParams, m_pWalls, m_wallCount);
		Raycaster::Draw2DGrid(m_pos, m_rotation);
	}	
}