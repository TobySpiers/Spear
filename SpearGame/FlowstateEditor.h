#pragma once
#include "SpearEngine/FlowstateManager.h"
#include "SpearEngine/TextureArray.h"
#include "SpearEngine/TextureFont.h"

constexpr int MAP_WIDTH_MAX_SUPPORTED{100};
constexpr int MAP_HEIGHT_MAX_SUPPORTED{100};

struct GridNode
{
	int texIdRoof{0};
	int texIdWall{0};
	int texIdFloor{0};

	int extendUp{ 0 };	// additional units for walls above
	int extendDown{ 0 };// additional units for walls below

	int collisionMask{0}; // 0 - no collision, 1 - player collision

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
};

class FlowstateEditor : public Spear::Flowstate
{
	enum InputActions
	{
		INPUT_APPLY,
		INPUT_CLEAR,
		INPUT_QUIT,

		INPUT_SCROLL_LEFT,
		INPUT_SCROLL_RIGHT,
		INPUT_SCROLL_UP,
		INPUT_SCROLL_DOWN,

		INPUT_ZOOM_IN,
		INPUT_ZOOM_OUT,

		INPUT_MODE_FLOOR,
		INPUT_MODE_WALL,
		INPUT_MODE_ROOF,

		INPUT_COUNT
	};

	enum EditorBatches
	{
		BATCH_GUI,
		BATCH_MAP,
		BATCH_TEXT
	};

	Vector2i MousePosToGridIndex();
	bool ValidGridIndex(const Vector2i& index);
	float MapSpacing(){return m_camZoom * (m_mapTextures.GetWidth() + 10.f);};
	float TileRadius(){return m_camZoom * (m_mapTextures.GetWidth() * 0.707f);};

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
	Spear::TextureArray m_mapTextures;
	Spear::TextureFont m_editorFont;
	EditorMapData m_map;
	Vector2f m_camOffset{0.f, 0.f};
	float m_camZoom{1.f};
};
