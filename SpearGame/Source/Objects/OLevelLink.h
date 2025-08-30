#pragma once
#include "GameObject/GameObject.h"
#include <DataTypes/AssetProperty.h>

class CollisionComponentRadial;

class OLevelLink : public GameObject
{
	GAMEOBJECT_DEFINITION_DATA(OLevelLink, GameObject, m_linkedLevel, m_linkId, m_seamless)

public:
	OLevelLink();

	virtual void OnOverlapBegin(CollisionComponent2D* other) override;

	int GetLinkId() const {return m_linkId;}

private:
	CollisionComponentRadial* m_collisionComp{ nullptr };

	Spear::AssetProperty m_linkedLevel{"../Assets/MAPS/", ".level"};
	int m_linkId{0};
	bool m_seamless{false};
};

