#include "OLevelLink.h"
#include "Collision/CollisionComponent2D.h"
#include "GameState.h"

GAMEOBJECT_REGISTER(OLevelLink)

OLevelLink::OLevelLink()
{
	m_collisionComp = AddComponent<CollisionComponentRadial>();
	m_collisionComp->ApplySetup(Collision::World, Collision::PROFILE_None, Collision::PROFILE_Characters, true);
}

void OLevelLink::OnOverlapBegin(CollisionComponent2D* other)
{
	// Since we can only overlap against Player channel, the player must have entered our space - request level transition
	if(m_linkedLevel.IsValid())
	{
		GameState::Get().QueueLevelTransition(m_linkedLevel.GetFilePath().path().filename().string().c_str(), m_linkId, m_seamless ? other->GetOwner().GetPosition().XY() - GetPosition().XY() : Vector2f::ZeroVector);
	}
}
