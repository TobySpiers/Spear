#pragma once
#include "Core/Serializer.h"

namespace Collision
{
	enum eCollisionType
	{
		None = 0,
		World = 1 << 0,
		Player = 1 << 1,
		NPC = 1 << 2,
		Enemy = 1 << 3,
		Pickup = 1 << 4,
		Interaction = 1 << 5,

		PROFILE_None = 0,
		PROFILE_All = World | Player | NPC | Enemy | Pickup | Interaction,
		PROFILE_Characters = Player | NPC | Enemy,

		PROFILE_PlayerBlock = World | Player | NPC | Enemy,
		PROFILE_PlayerOverlap = Pickup | Interaction
	};
	EXPOSE_ENUM_FLAGS(eCollisionType, None, World, Player, NPC, Enemy, Pickup, Interaction)
}