#pragma once
#include "Player.h"

struct GameState
{
	static GameState& Get();
	static GameState* GetSafe();

	float deltaTime{ 0.f };
	float gameTime{ 0.f };

	Player player;
	MapData mapData;
};
