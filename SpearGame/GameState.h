#pragma once
#include "Player.h"

struct GameState
{
	static GameState& Get();
	static GameState* GetSafe();

	Player player;
	MapData mapData;
};
