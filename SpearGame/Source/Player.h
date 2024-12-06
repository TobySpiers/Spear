#pragma once

class Player
{
public:
	Player();

	void SetPos(const Vector2f& pos){m_pos = pos;};
	void SetSpeed(float speed){m_walkSpeed = speed;};
	void Update(float deltaTime);
	void Draw(bool perspective) const;

private:
	Vector2f m_pos{0.f, 0.f};
	float m_walkSpeed{1.5f};
	float m_sprintSpeed{ 4.f };
	float m_lookSpeed{ 0.0035f };
	float m_pitch{0.f}; // vertical look
	float m_pitchLimit{ 0.5f };
	float m_rotation{TO_RADIANS(-90.f)};
	float m_turnSpeed{TO_RADIANS(2.f)};
	float m_collBox{ 0.12f }; // world-collisions only
};

