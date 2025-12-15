#include "Core/Core.h"
#include "Core/ServiceLocator.h"
#include "Core/InputManager.h"
#include "Flowstates/FlowstateGame.h"
#include <Collision/CollisionComponent2D.h>
#include "Player.h"

Player::Player()
{
	m_collisionComp = AddComponent<CollisionComponentRadial>();
	m_collisionComp->ApplySetup(Collision::Player, Collision::PROFILE_PlayerBlock, Collision::PROFILE_PlayerOverlap);
}

void Player::OnCreated()
{
	SetTickEnabled(true);
}

void Player::OnTick(float deltaTime)
{
	Spear::InputManager& input = Spear::ServiceLocator::GetInputManager();

	// player sprint
	const float moveDistance{ (input.InputHold(INPUT_SPRINT) ? m_sprintSpeed : m_walkSpeed) * deltaTime };

	// movement input
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

	// Pre-checked movement for player prevents tunneling during low FPS spikes and guarantees smooth sliding against tiled AABBs, but is probably overkill for objects other than the player
	// Some scenarios are smoother without pre-checks (squeezing through a slightly-too-small gap between a tile and a SolidObject), but without this we get caught on corners when sliding along tiled AABBs
	// The pre-checked movement also handles traversing portals
	if (moveDirection.x || moveDirection.y)
	{
		float rotationOffset;
		SetPosition(GameState::Get().mapData.PreCheckedMovement(GetPosition().XY(), moveDirection * moveDistance, m_collisionComp, rotationOffset));
		m_rotation += rotationOffset;
	}

	// mouse look
	m_rotation += (input.GetMouseAxis().x * m_lookSpeed);
	m_pitch += (input.GetMouseAxis().y * m_lookSpeed);
	m_pitch = std::min(std::max(m_pitch, -m_pitchLimit), m_pitchLimit); // cap pitch at half-up and half-down (greater angles reveal psuedo-3d-ness)

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