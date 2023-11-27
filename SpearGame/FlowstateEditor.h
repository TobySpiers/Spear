#pragma once
#include "SpearEngine/FlowstateManager.h"
#include "SpearEngine/TextureArray.h"
#include "SpearEngine/TextureFont.h"
#include "LevelData.h"

class FlowstateEditor : public Spear::Flowstate
{
	enum InputActions
	{
		INPUT_APPLY,
		INPUT_CLEAR,
		INPUT_QUIT,
		INPUT_MODIFIER,

		INPUT_INCREASE_MAPSIZE,
		INPUT_DECREASE_MAPSIZE,

		INPUT_SCROLL_LEFT,
		INPUT_SCROLL_RIGHT,
		INPUT_SCROLL_UP,
		INPUT_SCROLL_DOWN,

		INPUT_ZOOM_IN,
		INPUT_ZOOM_OUT,

		INPUT_CYCLE_MODE,

		INPUT_SAVE,
		INPUT_LOAD,

		INPUT_COUNT
	};

	enum EditorMode
	{
		MODE_FLOOR,
		MODE_WALL,
		MODE_ROOF,
		MODE_RISE,
		MODE_FALL,
		MODE_COLLISION,

		MODE_TOTAL
	};

	enum EditorBatches
	{
		BATCH_GUI,
		BATCH_MAP,
	};

	const char* GetModeText();
	Vector2i MousePosToGridIndex();
	bool ValidGridIndex(const Vector2i& index);
	float MapSpacing(){return m_camZoom * (m_mapTextures.GetWidth() + 10.f);};
	float TileRadius(){return m_camZoom * (m_mapTextures.GetWidth() * 0.707f);};
	void SaveLevel();
	void LoadLevel();
	

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

	Spear::UiButton m_textureButtons[eLevelTextures::TEX_TOTAL];

	Vector2f m_camOffset{400.f, 200.f};
	float m_camZoom{1.f};
	bool m_cursorInMenu{false};
	int m_curMode{EditorMode::MODE_WALL};
	int m_curTex{eLevelTextures::TEX_STONE};
	int m_clickCache{0};

	float m_saveCooldown{0.f};

	EditorMapData m_map;
};
