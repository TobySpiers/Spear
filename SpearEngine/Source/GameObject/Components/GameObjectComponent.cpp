#include "GameObjectComponent.h"
#include "GameObject/GameObject.h"

void GameObjectComponent::Initialise(GameObject* owner)
{
	ASSERT(!m_owner.IsValid())
	m_owner = owner;
}

GameObject* GameObjectComponent::GetOwner() const
{
	return m_owner.GetRawPtr();
}
