#pragma once
#include "GameObject/GameObject.h"

class CollisionComponentRadial;

class Player : public GameObject
{
public:
	Player();

	virtual void OnCreated() override;
	virtual void OnTick(float deltaTime) override;

	float GetLookRotation() const {return m_rotation;}
	float GetLookPitch() const {return m_pitch;}

private:
	CollisionComponentRadial* m_collisionComp{nullptr};

	float m_walkSpeed{1.5f};
	float m_sprintSpeed{ 4.f };
	float m_lookSpeed{ 0.0035f };
	float m_pitch{0.f}; // vertical look
	float m_pitchLimit{ 0.5f };
	float m_rotation{TO_RADIANS(-90.f)};
	float m_turnSpeed{TO_RADIANS(2.f)};
};

