#pragma once
#include "Player.h"
#include "Raycaster/Raycaster.h"

struct LevelTransition
{
	std::string levelName{""}; // the level file to transition to.
	int linkId{0}; // the linkId to position the player against inside the new level.
	Vector2f offset{Vector2f::ZeroVector}; // for 'seamless' transitions between levels with matching exit/entry points, we can maintain the same offset to uphold the illusion.
};

struct GameState
{
	static GameState& Get();
	static GameState* GetSafe();

	void QueueLevelTransition(const char* levelName, int linkId = 0, Vector2f offset = Vector2f::ZeroVector);
	const LevelTransition& GetPendingLevelTransition() const {return levelTransition;}
	void ClearPendingLevelTransition() {levelTransition.levelName = ""; }

	float deltaTime{ 0.f };
	float gameTime{ 0.f };

	Player* player{nullptr};
	MapData mapData;

	LevelTransition levelTransition;
};