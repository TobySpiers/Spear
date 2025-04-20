#pragma once
#include "Core/FlowstateManager.h"
#include "Graphics/TextureArray.h"
#include "Graphics/TextureFont.h"
#include "LevelData.h"

#include <unordered_set>

class GameObject;
class EditorActionBase;

class FlowstateEditor : public Spear::Flowstate
{
	enum EditorMode
	{
		MODE_TILES,
		MODE_OBJECTS,

		EDITORMODE_COUNT
	};

	enum InputActions
	{
		INPUT_CTRL,
		INPUT_ALT,
		INPUT_SHIFT,

		INPUT_Z,
		INPUT_Y,

		INPUT_APPLY,
		INPUT_CLEAR,
		INPUT_QUIT,
		INPUT_MODIFIER,

		INPUT_DELETE,

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


	enum DrawFlags
	{
		DRAW_OBJECTS = 1 << 0,
		DRAW_FLOOR2 = 1 << 1,
		DRAW_FLOOR = 1 << 2,
		DRAW_WALL = 1 << 3,
		DRAW_ROOF = 1 << 4,
		DRAW_ROOF2 = 1 << 5,
		DRAW_RISE = 1 << 6,
		DRAW_FALL = 1 << 7,
		DRAW_COLLISION = 1 << 8,
		DRAW_DIRECTION = 1 << 9,

		DRAW_DEFAULT = DRAW_FLOOR2 | DRAW_FLOOR | DRAW_WALL | DRAW_ROOF | DRAW_ROOF2 | DRAW_COLLISION | DRAW_OBJECTS,
		DRAW_NONE = 0,
		DRAW_ALL = (1 << 10) - 1,
	};
	int m_visibilityFlags = DRAW_DEFAULT;

	enum EditorBatches
	{
		BATCH_MAP,
	};

	enum OngoingClick
	{
		CLICK_NONE,
		CLICK_DEFAULT,
		CLICK_CTRL,
		CLICK_SHIFT,
		CLICK_ALT
	};

	enum SelectionMode
	{
		SELECTION_ADDITIVE,
		SELECTION_SUBTRACTIVE
	};

	Vector2f m_dragOrigin{ 0.f, 0.f };
	Vector2i m_clickOrigin{ 0, 0 };
	SelectionMode m_ongoingSelectionMode{ SELECTION_ADDITIVE };
	OngoingClick m_clickType{CLICK_NONE};
	std::unordered_set<HashableVector2i> m_ongoingClickSelection;

	void MakePanel_Editor_Tiles();
	void MakePanel_Editor_Objects();
	void MakePanel_Visibility();
	void MakePanel_Details_Tile();
	void MakePanel_Details_Object();
	void MakePanel_UndoRedo();

	bool MakePopup_TextureSelect(int& outValue, const char* popupId);

	const char* GetModeText(DrawFlags flag);

	// Tile Helpers
	Vector2i MousePosToGridIndex();
	bool ValidTile(const Vector2i& index);
	float MapSpacing() { return m_camZoom * m_mapTextures.GetWidth(); };
	float TileRadius(){return m_camZoom * (m_mapTextures.GetWidth() * 0.5f);};

	// Object Helpers
	Vector2f MousePosToWorldPos();
	Vector2f ScreenPosToWorldPos(const Vector2f& screenPos);
	Vector2f WorldPosToScreenPos(const Vector2f& worldPos);
	GameObject* MousePosToObject();

	void ProcessInput();
	bool ProcessInput_DragView();
	bool ProcessInput_UndoRedo();

	// Tiles - Editor Input
	bool ProcessInput_Tiles_FloodSelect();
	bool ProcessInput_Tiles_Select();

	// Objects - Editor Input
	bool ProcessInput_Objects_Select();
	bool ProcessInput_Objects_Create();
	bool ProcessInput_Objects_Delete();

	void StartSelection();
	void FloodSelect(const Vector2i& origin);
	void CommitSelection();

public:
	FlowstateEditor() {};
	virtual ~FlowstateEditor() {};

	void ResetEditor();
	void InitialiseMap(const char* mapName);

	// --- Flowstate Interface ---
	// Called once when state begins
	void StateEnter() override;

	// Update game. Return a slot id to switch state, return -1 to remain in current state.
	int StateUpdate(float deltaTime) override;

	// Render game
	void StateRender() override;

	// Called once when state ends
	void StateExit() override;

	// --- Editor Functions ---
	// Discards existing data and creates new level with supplied name
	void NewLevel(const char* levelName = "Untitled");

	// Saves existing data with the current level name
	void SaveLevel();

	// Saves existing data using supplied level name
	void SaveLevel(const char* levelName);

	// Opens saved data using supplied level name
	void OpenLevel(const char* levelName);

	// Commits an EditorAction to the Undo/Redo pipeline
	void CommitAction(EditorActionBase* action);

	// Undoes the requested number of EditorActions
	void UndoAction(int steps = 1);

	// Redoes the requested number of EditorActions
	void RedoAction(int steps = 1);

	void ClearUndoActions();
	void ClearRedoActions();
private:
	// State
	int m_editorMode{ EditorMode::MODE_TILES };
	std::vector<EditorActionBase*> m_undoActions;
	std::vector<EditorActionBase*> m_redoActions;

	// File
	EditorMapData* m_map{nullptr};
	std::string m_levelName{ "Untitled" };
	float m_saveCooldown{0.f};

	// Camera
	Vector2f m_camOffset{400.f, 200.f};
	float m_camZoom{1.f};

	// Hud
	float m_menuTexturesScrollSpeed{ 20.f };
	Spear::TextureArray m_mapTextures;

	// Colours
	const float m_fadedTileOpacity = 0.5f;
	const Colour4f m_colourDefault = Colour4f(1.f, 1.f, 1.f, 0.2f);
	const Colour4f m_colourCollision = Colour4f::Red();
	const Colour4f m_colourHovered = Colour4f::White();
	const Colour4f m_colourSelectingAdd = Colour4f::Cyan();
	const Colour4f m_colourSelectingSubtract = Colour4f::Red();
	const Colour4f m_colourSelected = Colour4f::Cyan();

	// Editor - Tiles
	std::unordered_set<HashableVector2i> m_selectedTiles;
	Vector2i m_hoveredTile{ -1, -1 };
	bool m_hoveredTileValid{ false };

	// Editor - Objects
	int m_objectConstructorId{0};
	std::unordered_set<GameObject*> m_selectedObjects;
	GameObject* m_hoveredObject{nullptr};
	GameObject* m_draggingObject{nullptr};
	Vector2f m_draggedObjectDeltaToMouse{0.f, 0.f};
	const float m_hoverRadiusSqr{.1f};
	bool m_dragSelectingObjects{false};

};
