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

		INPUT_SCROLL_LEFT,
		INPUT_SCROLL_RIGHT,
		INPUT_SCROLL_UP,
		INPUT_SCROLL_DOWN,

		INPUT_ZOOM_IN,
		INPUT_ZOOM_OUT,

		INPUT_MODE_FLOOR,
		INPUT_MODE_WALL,
		INPUT_MODE_ROOF,

		INPUT_SAVE,
		INPUT_LOAD,

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
	Spear::TextureFont m_editorFont;

	Spear::UiButton m_textureButtons[eLevelTextures::TEX_TOTAL];

	Vector2f m_camOffset{0.f, 0.f};
	float m_camZoom{1.f};
	int m_curTex;

	EditorMapData m_map;
};
