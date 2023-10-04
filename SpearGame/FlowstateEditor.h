#pragma once
#include "SpearEngine/FlowstateManager.h"
#include "SpearEngine/TextureArray.h"

constexpr int MAP_WIDTH_MAX_SUPPORTED{100};
constexpr int MAP_HEIGHT_MAX_SUPPORTED{100};

struct GridNode
{
	int texIdRoof{0};
	int texIdWall{0};
	int texIdFloor{0};

	int extendUp{ 0 };	// additional units for walls above
	int extendDown{ 0 };// additional units for walls below

	void Reset();
};

struct EditorMapData
{
	int gridWidth{10};
	int gridHeight{10};
	GridNode gridNodes[MAP_WIDTH_MAX_SUPPORTED * MAP_HEIGHT_MAX_SUPPORTED];

	void SetSize(int width, int height) {gridWidth = width; gridHeight = height;}
	GridNode& GetNode(int x, int y) {ASSERT(x < gridWidth && y < gridHeight && x >= 0 && y >= 0); return gridNodes[x + (y * MAP_WIDTH_MAX_SUPPORTED)]; }
	void ClearData();
	void Draw();
};

class FlowstateEditor : public Spear::Flowstate
{
	enum InputActions
	{
		INPUT_SELECT,
		INPUT_SELECT_ALT,
		INPUT_QUIT,

		INPUT_MODE_FLOOR,
		INPUT_MODE_WALL,
		INPUT_MODE_ROOF,

		INPUT_COUNT
	};

public:
	FlowstateEditor() {};
	virtual ~FlowstateEditor() {};

	// Called once when state begins
	void StateEnter() override;

	// Update game. Return a slot id to switch state, return -1 to remain in current state.
	int StateUpdate(float deltaTime) override;

	// Render game
	void StateRender() override;

	// Called once when state ends
	void StateExit() override;

	Spear::TextureArray m_menuTextures;
	EditorMapData m_map;
};
