#include "Core/Core.h"
#include "Core/ServiceLocator.h"
#include "Core/InputManager.h"
#include "Core/ImguiManager.h"
#include "Core/WindowManager.h"
#include "Graphics/ScreenRenderer.h"
#include "GameObject/GameObject.h"
#include "UiButton.h"

#include "eFlowstate.h"
#include "FlowstateEditor.h"
#include "LevelFileManager.h"
#include <imgui.h>
#include <imgui_stdlib.h>
#include <filesystem>
#include <imgui_internal.h>
#include "Editor/EditorAction_ModifyProperties.h"
#include "Editor/EditorAction_CreateObject.h"
#include "Editor/EditorAction_DeleteObjects.h"
#include "Editor/EditorAction_DragReposition.h"


void FlowstateEditor::ResetEditor()
{
	ClearRedoActions();
	ClearUndoActions();

	GameObject::GlobalDestroy();
	m_selectedObjects.clear();
	m_selectedTiles.clear();
	m_camOffset = { 400.f, 200.f };
	m_camZoom = 1.f;

	delete m_map;
	m_map = nullptr;
}

void FlowstateEditor::InitialiseMap(const char* mapName)
{
	ASSERT(m_map == nullptr);
	m_map = new EditorMapData(mapName);
}

void FlowstateEditor::StateEnter()
{
	NewLevel("Untitled");

	// Configure Inputs
	int config[INPUT_COUNT];

	config[INPUT_CTRL] = SDL_SCANCODE_LCTRL;
	config[INPUT_ALT] = SDL_SCANCODE_LALT;
	config[INPUT_SHIFT] = SDL_SCANCODE_LSHIFT;

	config[INPUT_N] = SDL_SCANCODE_N;
	config[INPUT_S] = SDL_SCANCODE_S;
	config[INPUT_Z] = SDL_SCANCODE_Z;
	config[INPUT_Y] = SDL_SCANCODE_Y;

	config[INPUT_APPLY] = SDL_BUTTON_LEFT;
	config[INPUT_CLEAR] = SDL_BUTTON_RIGHT;
	config[INPUT_QUIT] = SDL_SCANCODE_ESCAPE;

	config[INPUT_DELETE] = SDL_SCANCODE_DELETE;

	config[INPUT_SCROLL_LEFT] = SDL_SCANCODE_A;
	config[INPUT_SCROLL_RIGHT] = SDL_SCANCODE_D;
	config[INPUT_SCROLL_UP] = SDL_SCANCODE_W;
	config[INPUT_SCROLL_DOWN] = SDL_SCANCODE_S;
	config[INPUT_ZOOM_IN] = SDL_SCANCODE_E;
	config[INPUT_ZOOM_OUT] = SDL_SCANCODE_Q;

	config[INPUT_MODE_NEXT] = SDL_SCANCODE_RIGHTBRACKET;
	config[INPUT_MODE_PREV] = SDL_SCANCODE_LEFTBRACKET;

	config[INPUT_FACE_NORTH] = SDL_SCANCODE_UP;
	config[INPUT_FACE_EAST] = SDL_SCANCODE_RIGHT;
	config[INPUT_FACE_SOUTH] = SDL_SCANCODE_DOWN;
	config[INPUT_FACE_WEST] = SDL_SCANCODE_LEFT;

	Spear::ServiceLocator::GetInputManager().ConfigureInputBindings(config, INPUT_COUNT);
	Spear::ServiceLocator::GetInputManager().AllowImguiKeyboardConsumption(false);

	// Set background colour
	glClearColor(0.5f, 0.5f, 0.5f, 1.f);

	// Load map textures
	m_mapTextures.InitialiseFromDirectory("../Assets/TILESETS/64");
	Spear::Renderer::Get().CreateSpriteBatch(m_mapTextures, 64 * 64);

	// Load sprite textures
	m_spriteTextures.InitialiseFromDirectory("../Assets/SPRITES/SpriteSet1");
	Spear::Renderer::Get().CreateSpriteBatch(m_spriteTextures, 500);

	// Disable automatic ImGui handling - Editor handles ImGui manually
	Spear::ImguiManager& imguiManager = Spear::ImguiManager::Get();
	imguiManager.SetPanelsEnabled(false);

	Spear::WindowManager::Get().SetWindowTitleOverride("*Untitled - Spear");
}

int FlowstateEditor::StateUpdate(float deltaTime)
{
	Spear::InputManager& input = Spear::ServiceLocator::GetInputManager();

	bool bRequestQuit = false;

	if(m_saveCooldown > 0.f)
	{
		m_saveCooldown -= deltaTime;
	}

	const char* modal_NewLevel{ "Create New Level" };
	const char* modal_OpenLevel{ "Open Level" };
	const char* modal_SaveAs{ "Save As..." };

	static std::string modal_dataString{ "" };

	const char* selectedModal{ nullptr };
	if (ImGui::BeginMainMenuBar())
	{
		// Demo window menu
		if (ImGui::BeginMenu("File"))
		{
			bool test = false;
			if (ImGui::MenuItem("New", "Ctrl+N"))
			{
				selectedModal = modal_NewLevel;
			}
			if (ImGui::BeginMenu("Open", "Ctrl+O"))
			{
				for (const std::filesystem::directory_entry& filepath : std::filesystem::directory_iterator("../Assets/MAPS/"))
				{
					if (filepath.path().extension() == ".level")
					{
						if (ImGui::MenuItem(filepath.path().filename().string().c_str()))
						{
							selectedModal = modal_OpenLevel;
							modal_dataString = filepath.path().filename().string().c_str();
						}
					}
				}
				ImGui::EndMenu();
			}
			if (ImGui::MenuItem("Save", "Ctrl+S"))
			{
				SaveLevel();
			}
			if (ImGui::MenuItem("Save As...", "Ctrl+Alt+S"))
			{
				selectedModal = modal_SaveAs;
			}
			if (ImGui::MenuItem("Exit", "Escape"))
			{
				bRequestQuit = true;
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Undo", "Ctrl+Z", nullptr, m_undoActions.size() > 0))
			{
				UndoAction();
			}
			if (ImGui::MenuItem("Redo", "Ctrl+Y", nullptr, m_redoActions.size() > 0))
			{
				RedoAction();
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
	if (selectedModal)
	{
		ImGui::OpenPopup(selectedModal);
	}
	else if (input.InputHold(INPUT_CTRL))
	{
		// Modal menu keyboard shortcuts
		if (input.InputRelease(INPUT_N))
		{
			ImGui::OpenPopup(modal_NewLevel);
		}
		else if (input.InputRelease(INPUT_S))
		{
			if (input.InputHold(INPUT_ALT))
			{
				ImGui::OpenPopup(modal_SaveAs);
			}
			else
			{
				SaveLevel();
			}
		}
	}

	if (ImGui::BeginPopupModal(modal_NewLevel, NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Unsaved changes will be lost.\nThis operation cannot be undone!");
		ImGui::Separator();

		static std::string newLevelName{ "Untitled" };
		if(ImGui::InputText(".level", &newLevelName, ImGuiInputTextFlags_EnterReturnsTrue)
		|| ImGui::Button("Create", ImVec2(120, 0)))
		{
			ImGui::CloseCurrentPopup();
			NewLevel(newLevelName.c_str());
			newLevelName = "Untitled";
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0)))
		{
			ImGui::CloseCurrentPopup();
			newLevelName = "Untitled";
		}
		ImGui::EndPopup();
	}

	if (ImGui::BeginPopupModal(modal_OpenLevel, NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Unsaved changes will be lost.\nThis operation cannot be undone!");
		ImGui::Separator();
		ImGui::Text(modal_dataString.c_str());

		if (ImGui::Button("Open", ImVec2(120, 0)))
		{
			ImGui::CloseCurrentPopup();
			OpenLevel(modal_dataString.c_str());
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0)))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	if (ImGui::BeginPopupModal(modal_SaveAs, NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		static std::string levelName{ m_levelName };
		if (ImGui::InputText(".level", &levelName, ImGuiInputTextFlags_EnterReturnsTrue)
		|| ImGui::Button("Save As", ImVec2(120, 0)))
		{
			ImGui::CloseCurrentPopup();
			SaveLevel(levelName.c_str());
			levelName = m_levelName;
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0)))
		{
			ImGui::CloseCurrentPopup();
			levelName = m_levelName;
		}
		ImGui::EndPopup();
	}

	ImGui::Begin("Editor");
	ImGui::Text("Mode: ");
	ImGui::SameLine();
	if (ImGui::SmallButton("Map"))
	{
		m_selectedObjects.clear();
		m_editorMode = MODE_TILES;
	}
	ImGui::SameLine();
	if (ImGui::SmallButton("Object"))
	{
		m_selectedTiles.clear();
		m_editorMode = MODE_OBJECTS;
	}
	ImGui::NewLine();

	switch (m_editorMode)
	{
	case MODE_TILES:
		MakePanel_Editor_Tiles();
		break;
	case MODE_OBJECTS:
		MakePanel_Editor_Objects();
		break;
	}
	ImGui::End();

	MakePanel_Visibility();
	MakePanel_UndoRedo();

	ImGui::Begin("Details");
	switch (m_editorMode)
	{
	case MODE_TILES:
		MakePanel_Details_Tile();
		break;
	case MODE_OBJECTS:
		MakePanel_Details_Object();
		break;
	}	
	ImGui::End();

	// Cache mouse info
	m_hoveredTile = MousePosToGridIndex();
	m_hoveredTileValid = ValidTile(m_hoveredTile);
	m_hoveredObject = MousePosToObject();
	// Handle input
	ProcessInput();

	// ObjectClass tooltips
	if (m_editorMode == MODE_OBJECTS)
	{
		if(m_hoveredObject && !m_draggingObject)
		{
			ImGui::SetNextWindowPos(input.GetMousePos() + Vector2i(10, 10));
			ImGui::BeginTooltip();
			ImGui::Text(m_hoveredObject->GetClassName());
			ImGui::EndTooltip();
		}
	}

	// Keyboard camera scroll
	if (!input.InputHold(INPUT_CTRL))
	{
		float scrollSpeed{ 1000.f };
		if (input.InputHold(INPUT_SCROLL_LEFT))
		{
			m_camOffset.x += scrollSpeed * deltaTime;
		}
		if (input.InputHold(INPUT_SCROLL_RIGHT))
		{
			m_camOffset.x -= scrollSpeed * deltaTime;
		}
		if (input.InputHold(INPUT_SCROLL_UP))
		{
			m_camOffset.y += scrollSpeed * deltaTime;
		}
		if (input.InputHold(INPUT_SCROLL_DOWN))
		{
			m_camOffset.y -= scrollSpeed * deltaTime;
		}
	}

	// Handle zoom
	const int mouseWheel = input.Wheel();
	if (input.InputHold(INPUT_ZOOM_IN) || mouseWheel > 0)
	{
		ModifyZoom(1.1f);
	}
	else if (input.InputHold(INPUT_ZOOM_OUT) || mouseWheel < 0)
	{
		ModifyZoom(0.9f);
	}

	// Quit
	if (input.InputRelease(INPUT_QUIT))
	{
		bRequestQuit = true;
	}
	return bRequestQuit ? static_cast<int>(eFlowstate::STATE_MENU) : static_cast<int>(eFlowstate::STATE_THIS);
}

Vector2i FlowstateEditor::MousePosToGridIndex()
{
	Vector2f mousePos{ Spear::ServiceLocator::GetInputManager().GetMousePos().ToFloat()};
	mousePos -= m_camOffset;

	const Vector2i result{ static_cast<int>(std::round(mousePos.x / MapSpacing())), static_cast<int>(std::round(mousePos.y / MapSpacing())) };
	return result;
}

bool FlowstateEditor::ValidTile(const Vector2i& index)
{
	return (index.x >= 0 && index.x < m_map->gridWidth
		 && index.y >= 0 && index.y < m_map->gridHeight);
}

Vector2f FlowstateEditor::MousePosToWorldPos()
{	
	return ScreenPosToWorldPos(Spear::InputManager::Get().GetMousePos().ToFloat());
}

Vector2f FlowstateEditor::ScreenPosToWorldPos(const Vector2f& screenPos)
{
	return (screenPos - m_camOffset) / MapSpacing();
}

Vector2f FlowstateEditor::WorldPosToScreenPos(const Vector2f& worldPos)
{
	return m_camOffset + (worldPos * MapSpacing());
}

GameObject* FlowstateEditor::MousePosToObject()
{
	const Vector2f worldPos = MousePosToWorldPos() + GameObjectWorldPosOffset();

	const std::vector<GameObject*>& gameObjects = GameObject::GetAllObjects();
	float bestProximity{FLT_MAX};
	GameObject* result{nullptr};
	for (GameObject* object : gameObjects)
	{
		if (object->IsDestroyedInEditor())
		{
			continue;
		}

		const float proximity = (object->GetPosition().XY() - worldPos).LengthSqr();
		float requiredProximity = object->GetEditorHoverRadius(m_camZoom) / MapSpacing();
		if (proximity < requiredProximity && proximity < bestProximity)
		{
			result = object;
			bestProximity = proximity;
		}
	}
	return result;
}

void FlowstateEditor::ProcessInput()
{
	if (ProcessInput_DragView())
	{
		return;
	}

	if (ProcessInput_UndoRedo())
	{
		return;
	}

	if(m_editorMode == EditorMode::MODE_TILES)
	{
		if (ProcessInput_Tiles_FloodSelect())
		{
			return;
		}

		if (ProcessInput_Tiles_Select())
		{
			return;
		}
	}
	else if (m_editorMode == EditorMode::MODE_OBJECTS)
	{
		if (ProcessInput_Objects_Select())
		{
			return;
		}

		ProcessInput_Objects_Create();
		ProcessInput_Objects_Delete();
	}
}

bool FlowstateEditor::ProcessInput_DragView()
{
	Spear::InputManager& input = Spear::ServiceLocator::GetInputManager();
	if (input.RightClickStart())
	{
		m_clickOrigin = input.GetMousePos();
		m_dragOrigin = m_camOffset;
	}
	else if (input.RightClickHold())
	{
		m_camOffset = m_dragOrigin + (input.GetMousePos() - m_clickOrigin).ToFloat();
	}
	else
	{
		return false;
	}

	return true;
}

bool FlowstateEditor::ProcessInput_UndoRedo()
{
	Spear::InputManager& input = Spear::ServiceLocator::GetInputManager();

	if (input.InputHold(INPUT_CTRL))
	{
		if (input.InputRelease(INPUT_Z))
		{
			UndoAction();
			return true;
		}
		else if (input.InputRelease(INPUT_Y))
		{
			RedoAction();
			return true;
		}
	}
	return false;
}

bool FlowstateEditor::ProcessInput_Tiles_FloodSelect()
{
	Spear::InputManager& input = Spear::ServiceLocator::GetInputManager();
	if (input.MiddleClickStart())
	{
		StartSelection();
		FloodSelect(m_clickOrigin);
	}
	else if (input.MiddleClickHold())
	{
		if (m_clickOrigin != m_hoveredTile)
		{
			m_clickOrigin = m_hoveredTile;
			m_ongoingClickSelection.clear();
			FloodSelect(m_clickOrigin);
		}
	}
	else if (input.MiddleClickRelease())
	{
		CommitSelection();
	}
	else
	{
		return false;
	}

	return true;
}

bool FlowstateEditor::ProcessInput_Tiles_Select()
{
	Spear::InputManager& input = Spear::ServiceLocator::GetInputManager();
	if (input.ClickStart())
	{
		StartSelection();
	}
	else if (input.ClickHold())
	{
		if (!input.InputHold(INPUT_SHIFT))
		{
			m_ongoingClickSelection.clear();
			const Vector2i diff{ m_hoveredTile - m_clickOrigin };

			for (int stepX = 0; stepX <= std::abs(diff.x); stepX++)
			{
				for (int stepY = 0; stepY <= std::abs(diff.y); stepY++)
				{
					const HashableVector2i tile{ m_clickOrigin.x + (stepX * Sign(diff.x)), m_clickOrigin.y + (stepY * Sign(diff.y)) };
					if (ValidTile(tile))
					{
						m_ongoingClickSelection.insert(tile);
					}
				}
			}
		}
		else if (m_hoveredTileValid)
		{
			m_ongoingClickSelection.insert(m_hoveredTile);
		}
	}
	else if (input.ClickRelease())
	{
		CommitSelection();
	}
	else
	{
		return false;
	}

	return true;
}

bool FlowstateEditor::ProcessInput_Objects_Select()
{
	Spear::InputManager& input = Spear::ServiceLocator::GetInputManager();
	const bool bHoveringSelectedObject = m_hoveredObject && m_selectedObjects.contains(m_hoveredObject);

	if (input.ClickStart())
	{
		m_clickOrigin = input.GetMousePos();

		if (input.InputHold(INPUT_CTRL))
		{
			if (bHoveringSelectedObject)
			{
				m_selectedObjects.erase(m_hoveredObject);
				return true;
			}
		}
		else if(!bHoveringSelectedObject)
		{
			m_selectedObjects.clear();
		}

		if (m_hoveredObject)
		{
			if (!bHoveringSelectedObject)
			{
				m_selectedObjects.insert(m_hoveredObject);
			}
			if (!input.InputHold(INPUT_CTRL))
			{
				m_draggingObject = m_hoveredObject;
				m_draggedObjectDeltaToMouse = m_draggingObject->GetPosition().XY() - MousePosToWorldPos().ToFloat();
				m_draggedObjectStartPosition = m_draggingObject->GetPosition().XY();
			}
		}
		return true;
	}
	else if (input.ClickHold())
	{
		if (m_draggingObject)
		{
			const Vector2f delta = (MousePosToWorldPos() - m_draggingObject->GetPosition().XY()) + m_draggedObjectDeltaToMouse;
			for (GameObject* obj : m_selectedObjects)
			{
				obj->SetPosition(obj->GetPosition() + delta);
			}
		}
		else
		{
			m_dragSelectingObjects = true;
		}

		return true;
	}
	else if (input.ClickRelease())
	{
		if (m_draggingObject)
		{
			const Vector2f totalDelta = m_draggingObject->GetPosition().XY() - m_draggedObjectStartPosition;
			if (totalDelta != Vector2f::ZeroVector)
			{
				CommitAction(new EditorAction_DragReposition(m_selectedObjects, m_draggingObject->GetPosition().XY() - m_draggedObjectStartPosition));
			}
		}
		else
		{
			// Drag Select - Commit
			const Vector2f firstMousePos = ScreenPosToWorldPos(m_clickOrigin.ToFloat()) + GameObjectWorldPosOffset();
			const Vector2f curMousePos = MousePosToWorldPos() + GameObjectWorldPosOffset();

			const std::vector<GameObject*>& gameObjects = GameObject::GetAllObjects();
			for (GameObject* object : gameObjects)
			{
				if(!object->IsDestroyedInEditor())
				{
					if (object->GetPosition().XY().IsBetween(firstMousePos, curMousePos))
					{
						m_selectedObjects.insert(object);
					}
				}
			}
		}

		m_draggingObject = nullptr;
		m_dragSelectingObjects = false;
		return true;
	}
	
	return false;
}

bool FlowstateEditor::ProcessInput_Objects_Create()
{
	Spear::InputManager& input = Spear::ServiceLocator::GetInputManager();
	if (input.MiddleClickRelease())
	{
		m_selectedObjects.clear();

		EditorAction_CreateObject* CreateAction = new EditorAction_CreateObject(m_objectConstructorId, MousePosToWorldPos() + GameObjectWorldPosOffset());
		m_selectedObjects.insert(CreateAction->GetCreatedObject());

		CommitAction(CreateAction);
	}

	return false;
}

bool FlowstateEditor::ProcessInput_Objects_Delete()
{
	Spear::InputManager& input = Spear::ServiceLocator::GetInputManager();
	if (input.InputRelease(INPUT_DELETE))
	{
		if(m_selectedObjects.size())
		{
			CommitAction(new EditorAction_DeleteObjects(m_selectedObjects));

			if (m_selectedObjects.contains(m_hoveredObject))
			{
				m_hoveredObject = nullptr;
			}
			m_selectedObjects.clear();
		}
	}

	return false;
}

void FlowstateEditor::StartSelection()
{
	m_clickOrigin = m_hoveredTile;

	if (Spear::ServiceLocator::GetInputManager().InputHold(INPUT_CTRL))
	{
		if (!m_selectedTiles.contains(m_hoveredTile))
		{
			m_ongoingSelectionMode = SELECTION_ADDITIVE;
		}
		else
		{
			m_ongoingSelectionMode = SELECTION_SUBTRACTIVE;
		}
	}
	else
	{
		m_selectedTiles.clear();
		m_ongoingSelectionMode = SELECTION_ADDITIVE;
	}

	m_ongoingClickSelection.clear();
}

void FlowstateEditor::FloodSelect(const Vector2i& origin)
{
	if (!ValidTile(origin))
	{
		return;
	}

	std::unordered_set<HashableVector2i> searched;

	const GridNode& searchNode = m_map->GetNode(origin);
	std::unordered_set<HashableVector2i> search{origin};
	while (search.size())
	{
		const Vector2i& searchPos = *search.begin();
		m_ongoingClickSelection.insert(searchPos);

		for (int i = 0; i < 4; i++)
		{
			HashableVector2i neighbourPos{ searchPos };
			if (i % 2)
			{
				neighbourPos.x += (i == 1) ? 1 : -1;
			}
			else
			{
				neighbourPos.y += (i == 0) ? 1 : -1;
			}
			if (ValidTile(neighbourPos))
			{
				if (m_map->GetNode(neighbourPos).CompareNodeByTexture(searchNode))
				{
					if (!searched.contains(neighbourPos))
					{
						search.insert(neighbourPos);
					}
				}
			}
		}

		searched.insert(searchPos);
		search.erase(searchPos);
	}
}

void FlowstateEditor::CommitSelection()
{
	if (m_ongoingSelectionMode == SELECTION_ADDITIVE)
	{
		m_selectedTiles.insert(m_ongoingClickSelection.begin(), m_ongoingClickSelection.end());
	}
	else if (m_ongoingSelectionMode == SELECTION_SUBTRACTIVE)
	{
		for (const HashableVector2i& selectedTile : m_ongoingClickSelection)
		{
			m_selectedTiles.erase(selectedTile);
		}
	}

	m_ongoingClickSelection.clear();
}

void FlowstateEditor::ModifyZoom(float factor)
{
	const float cachedZoom{m_camZoom};
	m_camZoom *= factor;
	m_camZoom = std::min(m_camZoom, m_zoomMax);
	m_camZoom = std::max(m_camZoom, m_zoomMin);

	if (cachedZoom != m_camZoom)
	{
		const Vector2f anchor = Spear::InputManager::Get().GetMousePos().ToFloat();
		m_camOffset = anchor - (anchor - m_camOffset) * (m_camZoom / cachedZoom);
	}
}

void FlowstateEditor::MakePanel_Editor_Tiles()
{
	ImGui::SeparatorText("Tile Editor");
	ImGui::Text("Map Size:");
	int mapWidth{ m_map->gridWidth };
	int mapHeight{m_map->gridHeight};
	bool mapSizeChanged = ImGui::InputInt("Width", &mapWidth);
	mapSizeChanged |= ImGui::InputInt("Height", &mapHeight);
	{
		m_map->SetSize(mapWidth, mapHeight);
	}

	ImGui::SeparatorText("Plane Heights");
	ImGui::InputFloat("Inner", &m_map->planeHeights[PLANE_HEIGHT_INNER]);
	ImGui::InputFloat("Outer", &m_map->planeHeights[PLANE_HEIGHT_OUTER]);
}

void FlowstateEditor::MakePanel_Editor_Objects()
{
	ImGui::SeparatorText("Object Picker");
	std::vector<std::pair<const char*, GameObject::ConstructorFuncPtr>> ObjectConstructors = *GameObject::ObjectConstructors();

	if (ImGui::BeginListBox("##ObjectList"))
	{
		for (int i = 0; i < ObjectConstructors.size(); i++)
		{
			const bool IsSelected = (m_objectConstructorId == i);
			if (ImGui::Selectable(ObjectConstructors[i].first, IsSelected))
			{
				m_objectConstructorId = i;
			}

			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if (IsSelected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndListBox();
	}

	ImGui::Text("(Middle-Click to Place Objects!)");
}

void FlowstateEditor::MakePanel_Visibility()
{
	ImGui::Begin("Show-In-Editor Flags");
	for (int flagbit = 0; (1 << flagbit) < DRAW_ALL; flagbit++)
	{
		DrawFlags flag = static_cast<DrawFlags>(1 << flagbit);
		ImGui::CheckboxFlags(GetModeText(flag), &m_visibilityFlags, flag);
	}
	ImGui::End();
}

void FlowstateEditor::MakePanel_Details_Tile()
{
	if (!m_selectedTiles.size())
	{
		ImGui::Text("No tile selected");
		return;
	}

	// Taking a COPY of the first node and amalgamating settings from selected tiles
	GridNode commonNode = m_map->GetNode(*m_selectedTiles.begin());
	for (const HashableVector2i selectedTile : m_selectedTiles)
	{
		const GridNode& node = m_map->GetNode(selectedTile);
		for (int i = 0; i < 2; i++)
		{
			if (commonNode.texIdFloor[i] != node.texIdFloor[i])
			{
				commonNode.texIdFloor[i] = TEX_MULTI;
			}
			if (commonNode.texIdRoof[i] != node.texIdRoof[i])
			{
				commonNode.texIdRoof[i] = TEX_MULTI;
			}
		}
		if (commonNode.texIdWall != node.texIdWall)
		{
			commonNode.texIdWall = TEX_MULTI;
		}
	}
	
	ImGui::SeparatorText("Texture Settings");

	const char* strRoofA = "Roof (Far)";
	ImGui::ImageButton(strRoofA, m_mapTextures.GetTextureViewForLayer(commonNode.texIdRoof[1]), ImVec2(m_mapTextures.GetWidth(), m_mapTextures.GetHeight()));
	bool modifiedRoofA = MakePopup_TextureSelect(commonNode.texIdRoof[1], strRoofA);
	ImGui::SameLine();
	ImGui::Text(strRoofA);

	const char* strRoofB = "Roof (Near)";
	ImGui::ImageButton(strRoofB, m_mapTextures.GetTextureViewForLayer(commonNode.texIdRoof[0]), ImVec2(m_mapTextures.GetWidth(), m_mapTextures.GetHeight()));
	bool modifiedRoofB = MakePopup_TextureSelect(commonNode.texIdRoof[0], strRoofB);
	ImGui::SameLine();
	ImGui::Text(strRoofB);

	const char* strWall = "Wall";
	ImGui::ImageButton(strWall, m_mapTextures.GetTextureViewForLayer(commonNode.texIdWall), ImVec2(m_mapTextures.GetWidth(), m_mapTextures.GetHeight()));
	bool modifiedWall = MakePopup_TextureSelect(commonNode.texIdWall, strWall);
	ImGui::SameLine();
	ImGui::Text(strWall);

	const char* strFloorA = "Floor (Near)";
	ImGui::ImageButton(strFloorA, m_mapTextures.GetTextureViewForLayer(commonNode.texIdFloor[0]), ImVec2(m_mapTextures.GetWidth(), m_mapTextures.GetHeight()));
	bool modifiedFloorA = MakePopup_TextureSelect(commonNode.texIdFloor[0], strFloorA);
	ImGui::SameLine();
	ImGui::Text(strFloorA);

	const char* strFloorB = "Floor (Far)";
	ImGui::ImageButton(strFloorB, m_mapTextures.GetTextureViewForLayer(commonNode.texIdFloor[1]), ImVec2(m_mapTextures.GetWidth(), m_mapTextures.GetHeight()));
	bool modifiedFloorB = MakePopup_TextureSelect(commonNode.texIdFloor[1], strFloorB);
	ImGui::SameLine();
	ImGui::Text(strFloorB);

	ImGui::SeparatorText("Height Settings");
	bool modifiedRise = ImGui::InputInt("Rise", &commonNode.extendUp);
	commonNode.extendUp = std::max(commonNode.extendUp, 0);
	bool modifiedFall = ImGui::InputInt("Fall", &commonNode.extendDown);
	commonNode.extendDown = std::max(commonNode.extendDown, 0);

	ImGui::SeparatorText("Collision Flags");
	bool modifiedCollWall = ImGui::CheckboxFlags("Wall##coll", &commonNode.collisionMask, COLL_WALL);
	bool modifiedCollSolid = ImGui::CheckboxFlags("Solid", &commonNode.collisionMask, COLL_SOLID);

	ImGui::SeparatorText("Visibility Flags");
	bool modifiedDrawN = ImGui::CheckboxFlags("North Face", &commonNode.drawFlags, DRAW_N);
	bool modifiedDrawE = ImGui::CheckboxFlags("East Face", &commonNode.drawFlags, DRAW_E);
	bool modifiedDrawS = ImGui::CheckboxFlags("South Face", &commonNode.drawFlags, DRAW_S);
	bool modifiedDrawW = ImGui::CheckboxFlags("West Face", &commonNode.drawFlags, DRAW_W);

	for(const HashableVector2i selectedTile : m_selectedTiles)
	{
		GridNode& node = m_map->GetNode(selectedTile);

		if (modifiedRoofB)
			node.texIdRoof[0] = commonNode.texIdRoof[0];
		if (modifiedRoofA)
			node.texIdRoof[1] = commonNode.texIdRoof[1];
		if (modifiedWall)
			node.texIdWall = commonNode.texIdWall;
		if (modifiedFloorA)
			node.texIdFloor[0] = commonNode.texIdFloor[0];
		if (modifiedFloorB)
			node.texIdFloor[1] = commonNode.texIdFloor[1];

		if (modifiedRise)
			node.extendUp = commonNode.extendUp;
		if (modifiedFall)
			node.extendDown = commonNode.extendDown;

		if (modifiedCollWall)
			node.collisionMask = (node.collisionMask & ~COLL_WALL) | (commonNode.collisionMask & COLL_WALL);
		if (modifiedCollSolid)
			node.collisionMask = (node.collisionMask & ~COLL_SOLID) | (commonNode.collisionMask & COLL_SOLID);

		if (modifiedDrawN)
			node.drawFlags = (node.drawFlags & ~DRAW_N) | (commonNode.drawFlags & DRAW_N);
		if (modifiedDrawE)
			node.drawFlags = (node.drawFlags & ~DRAW_E) | (commonNode.drawFlags & DRAW_E);
		if (modifiedDrawS)
			node.drawFlags = (node.drawFlags & ~DRAW_S) | (commonNode.drawFlags & DRAW_S);
		if (modifiedDrawW)
			node.drawFlags = (node.drawFlags & ~DRAW_W) | (commonNode.drawFlags & DRAW_W);

	}
}

void FlowstateEditor::MakePanel_Details_Object()
{
	if (m_selectedObjects.empty())
	{
		ImGui::Text("No object selected");
		return;
	}

	bool bUniformType = true;
	GameObject* selectedObject = *m_selectedObjects.begin();
	const char* objectClass = selectedObject->GetClassName();
	for (const GameObject* obj : m_selectedObjects)
	{
		if (obj->GetClassName() != objectClass)
		{
			bUniformType = false;
			break;
		}
	}

	if (bUniformType)
	{
		ImGui::SeparatorText(selectedObject->GetClassName());

		if (!m_modifyObjectsAction)
		{
			m_modifyObjectsAction = new EditorAction_ModifyProperties(m_selectedObjects);
		}

		// Expose selected objects for editing in ImGui, encapsulated within an EditorAction
		m_modifyObjectsAction->Expose();

		if (m_modifyObjectsAction->IsModificationComplete())
		{
			// If objects were modified, handover ownership of Action to the Undo pipeline (this deletes Action for us when appropriate)
			CommitAction(m_modifyObjectsAction);
			m_modifyObjectsAction = nullptr;
		}
		else if(!m_modifyObjectsAction->IsModificationInProgress())
		{
			// If no modifications were made, delete the Action ourselves (this is a fairly lazy approach to avoid worrying about situations where m_selectedObjects may be different next frame)
			delete m_modifyObjectsAction;
			m_modifyObjectsAction = nullptr;
		}
	}
	else
	{
		ImGui::Text("Multiple types selected");
	}
}

void FlowstateEditor::MakePanel_UndoRedo()
{
	ImGui::Begin("History");
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(.5f, .5f, .5f, 1.0f));
	for (EditorActionBase* redoAction : m_redoActions)
	{
		ImGui::Text(redoAction->ActionName().c_str());
	}
	ImGui::PopStyleColor();

	for (int i = m_undoActions.size() - 1; i >= 0; i--)
	{
		EditorActionBase* UndoAction = m_undoActions[i];
		ImGui::Text(UndoAction->ActionName().c_str());
	}

	ImGui::End();
}

bool FlowstateEditor::MakePopup_TextureSelect(int& outValue, const char* popupId)
{
	if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
	{
		ImGui::OpenPopup(popupId);
	}

	bool valueChanged = false;
	if (ImGui::BeginPopup(popupId, ImGuiWindowFlags_NoMove))
	{
		for (int i = 0; i < m_mapTextures.GetDepth(); i++)
		{
			ImGui::Image(m_mapTextures.GetTextureViewForLayer(i), ImVec2(m_mapTextures.GetWidth(), m_mapTextures.GetHeight()));
			if (ImGui::IsItemClicked())
			{
				valueChanged = outValue != i;
				outValue = i;
				ImGui::CloseCurrentPopup();
			}

			const int popupItemsPerRow = 4;
			if ((i + 1) % popupItemsPerRow != 0)
			{
				ImGui::SameLine();
			}
		}

		ImGui::EndPopup();
	}

	if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
	{
		if (outValue != TEX_NONE)
		{
			outValue = TEX_NONE;
			valueChanged = true;
		}
	}

	return valueChanged;
}

const char* FlowstateEditor::GetModeText(DrawFlags flag)
{
	switch (flag)
	{
		case DRAW_OBJECTS:
			return "Objects";
		case DRAW_FLOOR:
			return "Floor";
		case DRAW_FLOOR2:
			return "Floor2";
		case DRAW_WALL:
			return "Wall";
		case DRAW_ROOF:
			return "Roof";
		case DRAW_ROOF2:
			return "Roof2";
		case DRAW_RISE:
			return "Rise";
		case DRAW_FALL:
			return "Fall";
		case DRAW_COLLISION:
			return "Collision";
		case DRAW_DIRECTION:
			return "Directional";
		default:
			return "Unknown";
	}
}

void FlowstateEditor::StateRender()
{
	Spear::Renderer& rend = Spear::ServiceLocator::GetScreenRenderer();

	// Draw save popup if cooldown active
	// TODO: Make this an ImGui overlay ( see ImGuiDemo > Examples > Simple Overlay)
	if(m_saveCooldown > 0.f)
	{
		Spear::Renderer::TextData textSavePopup;
		textSavePopup.text = "Level saved...";
		textSavePopup.pos = Vector2f(Spear::Core::GetWindowSize().x / 2, Spear::Core::GetWindowSize().y - 20.f);
		textSavePopup.alignment = Spear::Renderer::TEXT_ALIGN_MIDDLE;
		Spear::ServiceLocator::GetScreenRenderer().AddText(textSavePopup);
	}

	// Draw PlayerStart
	Spear::Renderer::TextData textStart;
	textStart.text = "@";
	textStart.pos = m_camOffset + (m_map->playerStart.ToFloat() * MapSpacing());
	textStart.alignment = Spear::Renderer::TEXT_ALIGN_MIDDLE;
	Spear::ServiceLocator::GetScreenRenderer().AddText(textStart);

	// Draw Map Data
	Vector2i mouseNode{MousePosToGridIndex()};
	constexpr float floorDepth = 0.9f;
	constexpr float wallDepth = 0.8f;
	constexpr float roofDepth = 0.7f;
	constexpr float outlineDepth = 0.5f;
	constexpr float objectDepth = 0.4f;

	constexpr float inactiveOpacity = 0.2f;

	for (int x = 0; x < m_map->gridWidth; x++)
	{
		for (int y = 0; y < m_map->gridHeight; y++)
		{
			GridNode& node{ m_map->GetNode(x, y) };
			const bool bActiveSelection = m_selectedTiles.size() || m_ongoingClickSelection.size();
			const bool bIsHovered = m_hoveredTile.x == x && m_hoveredTile.y == y;
			const bool bIsSelected = m_selectedTiles.contains({ x, y }) || m_ongoingClickSelection.contains({x, y});

			// Tile Outlines
			const Vector2f tilePos = m_camOffset + (Vector2f(x, y) * MapSpacing());
			if (m_editorMode == MODE_TILES)
			{

				Spear::Renderer::LineBoxData box;
				box.colour = (node.collisionMask > 0 ? m_colourCollision : m_colourDefault);
				box.start = tilePos - Vector2f(TileRadius());
				box.end = tilePos + Vector2f(TileRadius());
				box.depth = outlineDepth;

				if (!bIsHovered)
				{
					box.depth += 0.025f;
				}
				if (!bIsSelected)
				{
					box.depth += 0.025f;
				}

				if (m_ongoingClickSelection.contains({ x, y }))
				{
					switch (m_ongoingSelectionMode)
					{
					case SELECTION_ADDITIVE: box.colour = m_colourSelectingAdd; break;
					case SELECTION_SUBTRACTIVE: box.colour = m_colourSelectingSubtract; break;
					default: ASSERT(false); break;
					}
				}
				else if (bIsHovered)
				{
					box.colour = m_colourHovered;
				}
				else if (bIsSelected)
				{
					box.colour = m_colourSelected;
				}
				rend.AddLineBox(box);
			}

			// Meta info
			if (m_visibilityFlags & DRAW_RISE && node.extendUp)
			{
				Spear::Renderer::TextData textRise;
				textRise.text = std::to_string(node.extendUp);
				textRise.pos = tilePos;
				textRise.alignment = Spear::Renderer::TEXT_ALIGN_MIDDLE;
				Spear::ServiceLocator::GetScreenRenderer().AddText(textRise);
			}
			if (m_visibilityFlags & DRAW_FALL && node.extendDown)
			{
				Spear::Renderer::TextData textRise;
				textRise.text = std::to_string(node.extendDown);
				textRise.pos = tilePos;
				textRise.alignment = Spear::Renderer::TEXT_ALIGN_MIDDLE;
				Spear::ServiceLocator::GetScreenRenderer().AddText(textRise);
			}
			if (m_visibilityFlags & DRAW_DIRECTION)
			{
				Spear::Renderer::TextData textDraw;
				std::string textFlags{ "" };
				if (node.drawFlags & DRAW_N) textFlags += "N";
				if (node.drawFlags & DRAW_E) textFlags += "E";
				if (node.drawFlags & DRAW_S) textFlags += "S";
				if (node.drawFlags & DRAW_W) textFlags += "W";
				textDraw.text = textFlags;
				textDraw.pos = tilePos;
				textDraw.alignment = Spear::Renderer::TEXT_ALIGN_MIDDLE;
				Spear::ServiceLocator::GetScreenRenderer().AddText(textDraw);
			}
			
			// Wall Textures
			if (m_visibilityFlags & DRAW_WALL && node.texIdWall != TEX_NONE)
			{
				Spear::Renderer::SpriteData sprite;
				sprite.pos = Vector2f(x, y) * MapSpacing();
				sprite.pos += m_camOffset;
				sprite.size = Vector2f(m_camZoom, m_camZoom);
				sprite.texLayer = node.texIdWall;
				sprite.depth = wallDepth;
				if (m_editorMode == MODE_OBJECTS || (bActiveSelection && !bIsSelected))
				{
					sprite.opacity = m_fadedTileOpacity;
				}
				Spear::ServiceLocator::GetScreenRenderer().AddSprite(sprite, BATCH_MAP);
			}

			// Roof Textures
			if (m_visibilityFlags & DRAW_ROOF && node.texIdRoof[0] != TEX_NONE)
			{
				Spear::Renderer::SpriteData sprite;
				sprite.pos = Vector2f(x, y) * MapSpacing();
				sprite.pos += m_camOffset;
				sprite.size = Vector2f(m_camZoom, m_camZoom);
				sprite.texLayer = node.texIdRoof[0];
				sprite.depth = roofDepth;
				if (m_editorMode == MODE_OBJECTS || (bActiveSelection && !bIsSelected))
				{
					sprite.opacity = m_fadedTileOpacity;
				}
				Spear::ServiceLocator::GetScreenRenderer().AddSprite(sprite, BATCH_MAP);
			}
			if (m_visibilityFlags & DRAW_ROOF2 && node.texIdRoof[1] != TEX_NONE)
			{
				Spear::Renderer::SpriteData sprite;
				sprite.pos = Vector2f(x, y) * MapSpacing();
				sprite.pos += m_camOffset;
				sprite.size = Vector2f(m_camZoom, m_camZoom);
				sprite.texLayer = node.texIdRoof[1];
				sprite.depth = roofDepth;
				if (m_editorMode == MODE_OBJECTS || (bActiveSelection && !bIsSelected))
				{
					sprite.opacity = m_fadedTileOpacity;
				}
				Spear::ServiceLocator::GetScreenRenderer().AddSprite(sprite, BATCH_MAP);
			}

			// Floor Textures
			if (m_visibilityFlags & DRAW_FLOOR && node.texIdFloor[0] != TEX_NONE)
			{
				Spear::Renderer::SpriteData sprite;
				sprite.pos = Vector2f(x, y) * MapSpacing();
				sprite.pos += m_camOffset;
				sprite.size = Vector2f(m_camZoom, m_camZoom);
				sprite.texLayer = node.texIdFloor[0];
				sprite.depth = floorDepth;
				if (m_editorMode == MODE_OBJECTS || (bActiveSelection && !bIsSelected))
				{
					sprite.opacity = m_fadedTileOpacity;
				}
				Spear::ServiceLocator::GetScreenRenderer().AddSprite(sprite, BATCH_MAP);
			}
			if (m_visibilityFlags & DRAW_FLOOR2 && node.texIdFloor[1] != TEX_NONE)
			{
				Spear::Renderer::SpriteData sprite;
				sprite.pos = Vector2f(x, y) * MapSpacing();
				sprite.pos += m_camOffset;
				sprite.size = Vector2f(m_camZoom, m_camZoom);
				sprite.texLayer = node.texIdFloor[1];
				sprite.depth = floorDepth;
				if (m_editorMode == MODE_OBJECTS || (bActiveSelection && !bIsSelected))
				{
					sprite.opacity = m_fadedTileOpacity;
				}
				Spear::ServiceLocator::GetScreenRenderer().AddSprite(sprite, BATCH_MAP);
			}
		}
	}

	// Draw GameObjects
	if(m_visibilityFlags & DRAW_OBJECTS)
	{
		const std::vector<GameObject*>& gameObjects = GameObject::GetAllObjects();
		const Vector2f selectionBoxStart = ScreenPosToWorldPos(m_clickOrigin.ToFloat());
		const Vector2f selectionBoxEnd = MousePosToWorldPos();
		for (GameObject* object : gameObjects)
		{
			const Vector3f editorPosition = (m_camOffset + (object->GetPosition().XY() - GameObjectWorldPosOffset()) * MapSpacing());

			bool bSelected = m_editorMode == MODE_OBJECTS && m_selectedObjects.contains(object);
			bool bHovered = m_editorMode == MODE_OBJECTS && m_hoveredObject == object && !m_draggingObject && !m_dragSelectingObjects;
			if (!bHovered)
			{
				if (m_editorMode == MODE_OBJECTS && m_dragSelectingObjects)
				{
					bHovered = (object->GetPosition().XY().IsBetween(selectionBoxStart + GameObjectWorldPosOffset(), selectionBoxEnd + GameObjectWorldPosOffset()));
				}
			}

			object->DrawInEditor(editorPosition, m_camZoom, bSelected, bHovered);
		}

		if(m_editorMode == MODE_OBJECTS)
		{
			if (m_dragSelectingObjects)
			{
				Spear::Renderer::LineBoxData box;
				box.start = m_clickOrigin.ToFloat();
				box.end = Spear::InputManager::Get().GetMousePos().ToFloat();
				rend.AddLineBox(box);
			}
		}
	}
	Spear::Renderer::Get().Render();
}

void FlowstateEditor::StateExit()
{
	ResetEditor();

	Spear::ServiceLocator::GetScreenRenderer().ReleaseAll();
	Spear::ServiceLocator::GetInputManager().AllowImguiKeyboardConsumption(true);

	Spear::ImguiManager& imguiManager = Spear::ImguiManager::Get();
	imguiManager.SetPanelsEnabled(false);
	imguiManager.SetMenuBarStatsEnabled(true);
	imguiManager.SetMenuBarLabel(nullptr);

	Spear::WindowManager::Get().SetWindowTitleOverride(nullptr);
}

void FlowstateEditor::NewLevel(const char* levelName)
{
	ResetEditor();
	InitialiseMap(levelName);

	Spear::WindowManager::Get().SetWindowTitleOverride((std::string("*") + levelName + " - Spear").c_str());
}

void FlowstateEditor::SaveLevel()
{
	if(m_saveCooldown <= 0.f)
	{
		LevelFileManager::EditorSaveLevel(*m_map);
		m_saveCooldown = 3.0f;
	}
}

void FlowstateEditor::SaveLevel(const char* levelName)
{
	m_map->mapName = levelName;
	SaveLevel();
	Spear::WindowManager::Get().SetWindowTitleOverride((std::string(levelName) + " - Spear").c_str());
}

void FlowstateEditor::OpenLevel(const char* levelName)
{
	ResetEditor();
	InitialiseMap(levelName);

	LevelFileManager::EditorLoadLevel(levelName, *m_map);
	Spear::WindowManager::Get().SetWindowTitleOverride((std::string(m_map->mapName) + " - Spear").c_str());
}

void FlowstateEditor::CommitAction(EditorActionBase* action)
{
	ClearRedoActions();
	m_undoActions.emplace_back(action);
}

void FlowstateEditor::UndoAction(int steps)
{
	while (steps-- > 0 && !m_undoActions.empty())
	{
		m_undoActions.back()->Undo(m_selectedObjects);
		m_redoActions.emplace_back(m_undoActions.back());
		m_undoActions.pop_back();
	}
}

void FlowstateEditor::RedoAction(int steps)
{
	while (steps-- > 0 && !m_redoActions.empty())
	{
		m_redoActions.back()->Redo(m_selectedObjects);
		m_undoActions.emplace_back(m_redoActions.back());
		m_redoActions.pop_back();
	}
}

void FlowstateEditor::ClearUndoActions()
{
	for (EditorActionBase* undoAction : m_undoActions)
	{
		delete undoAction;
	}
	m_undoActions.clear();
}

void FlowstateEditor::ClearRedoActions()
{
	for (EditorActionBase* redoAction : m_redoActions)
	{
		delete redoAction;
	}
	m_redoActions.clear();
}
