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

	// movement input
	const float moveDistance{ (input.InputHold(INPUT_SPRINT) ? m_sprintSpeed : m_walkSpeed) * deltaTime };
	Vector2f moveDirection{ 0.f, 0.f };
	if (input.InputHold(INPUT_FORWARD))
	{
		moveDirection = Vector2f(cos(m_rotation), sin(m_rotation));
	}
	else if (input.InputHold(INPUT_BACKWARD))
	{
		moveDirection = Vector2f(cos(m_rotation), sin(m_rotation)) * -1;
	}
	if (input.InputHold(INPUT_STRAFE_LEFT))
	{
		moveDirection += Vector2f(-sin(m_rotation), cos(m_rotation)) * -1;
	}
	else if (input.InputHold(INPUT_STRAFE_RIGHT))
	{
		moveDirection += Vector2f(-sin(m_rotation), cos(m_rotation));
	}
	moveDirection = NormalizeNonZero(moveDirection);

	// Pre-checked movement
	if (moveDirection.x || moveDirection.y)
	{
		GameState& gameState = GameState::Get();

		const u8 collisionFlags = eCollisionMask::COLL_WALL | eCollisionMask::COLL_SOLID;
		if (!gameState.mapData.CollisionSearchDDA(m_pos + Vector2f(0.f, m_collBox), Vector2f((moveDirection.x * moveDistance) + (Sign(moveDirection.x) * m_collBox), 0.f), collisionFlags)
		&& !gameState.mapData.CollisionSearchDDA(m_pos - Vector2f(0.f, m_collBox), Vector2f((moveDirection.x * moveDistance) + (Sign(moveDirection.x) * m_collBox), 0.f), collisionFlags))
		{
			m_pos.x += moveDirection.x * moveDistance;
		}
		if (!gameState.mapData.CollisionSearchDDA(m_pos + Vector2f(m_collBox, 0.f), Vector2f(0.f, (moveDirection.y * moveDistance) + (Sign(moveDirection.y) * m_collBox)), collisionFlags)
		&& !gameState.mapData.CollisionSearchDDA(m_pos - Vector2f(m_collBox, 0.f), Vector2f(0.f, (moveDirection.y * moveDistance) + (Sign(moveDirection.y) * m_collBox)), collisionFlags))
		{
			m_pos.y += moveDirection.y * moveDistance;
		}
	}

	// mouse look
	m_rotation += (input.GetMouseAxis().x * 0.005f);
 	m_pitch += (input.GetMouseAxis().y * 0.004f);
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