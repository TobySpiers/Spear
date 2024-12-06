#pragma once
#include "Core/FlowstateManager.h"
#include "Graphics/TextureArray.h"
#include "Graphics/TextureFont.h"
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

		INPUT_MODE_NEXT,
		INPUT_MODE_PREV,

		INPUT_FACE_NORTH,
		INPUT_FACE_EAST,
		INPUT_FACE_SOUTH,
		INPUT_FACE_WEST,

		INPUT_SAVE,
		INPUT_LOAD,

		INPUT_COUNT
	};

	enum EditorMode
	{
		MODE_PLAYERSTART,
		MODE_FLOOR2,
		MODE_FLOOR,
		MODE_WALL,
		MODE_ROOF,
		MODE_ROOF2,
		MODE_RISE,
		MODE_FALL,
		MODE_COLLISION,
		MODE_DRAW_DIRECTION,

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

	float m_menuTexturesScrollSpeed{ 20.f };
	Spear::TextureArray m_menuTextures;
	Spear::TextureArray m_mapTextures;

	std::vector<Spear::UiButton> m_textureButtons;

	Vector2f m_camOffset{400.f, 200.f};
	float m_camZoom{1.f};
	bool m_cursorInMenu{false};
	int m_curMode{EditorMode::MODE_WALL};
	int m_curTex{0};
	int m_clickCache{0};

	float m_saveCooldown{0.f};

	EditorMapData m_map;
};
