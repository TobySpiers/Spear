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


void FlowstateEditor::StateEnter()
{
	NewLevel("Untitled");
	m_selectedTiles.clear();
	m_camOffset = { 400.f, 200.f };

	// Configure Inputs
	int config[INPUT_COUNT];

	config[INPUT_CTRL] = SDL_SCANCODE_LCTRL;
	config[INPUT_SHIFT] = SDL_SCANCODE_LSHIFT;
	config[INPUT_ALT] = SDL_SCANCODE_LALT;

	config[INPUT_APPLY] = SDL_BUTTON_LEFT;
	config[INPUT_CLEAR] = SDL_BUTTON_RIGHT;
	config[INPUT_QUIT] = SDL_SCANCODE_ESCAPE;
	config[INPUT_MODIFIER] = SDL_SCANCODE_LCTRL;


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
	
	config[INPUT_SAVE] = SDL_SCANCODE_S;
	config[INPUT_LOAD] = SDL_SCANCODE_L;

	Spear::ServiceLocator::GetInputManager().ConfigureInputBindings(config, INPUT_COUNT);

	// Set background colour
	glClearColor(0.5f, 0.5f, 0.5f, 1.f);

	// Load map textures
	m_mapTextures.InitialiseFromDirectory("../Assets/TILESETS/64");
	Spear::ServiceLocator::GetScreenRenderer().CreateSpriteBatch(m_mapTextures, 64 * 64);

	// Disable ImGui panels - Editor sets up ImGui directly
	Spear::ImguiManager& imguiManager = Spear::ImguiManager::Get();
	imguiManager.SetPanelsEnabled(false);

	Spear::WindowManager::Get().SetWindowTitleOverride("*Untitled - Spear");
}

int FlowstateEditor::StateUpdate(float deltaTime)
{
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

		if (ImGui::BeginMenu("Mode"))
		{
			bool test = false;
			ImGui::MenuItem("Tile", nullptr, &test);
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
	if (selectedModal)
	{
		ImGui::OpenPopup(selectedModal);
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
	if (ImGui::SmallButton("(M) Map"))
	{
		m_editorMode = MODE_TILES;
	}
	ImGui::SameLine();
	if (ImGui::SmallButton("(O) Object"))
	{
		m_editorMode = MODE_OBJECTS;
	}
	ImGui::NewLine();

	switch (m_editorMode)
	{
	case MODE_TILES:
		MakePanel_Editor_Map();
		break;
	case MODE_OBJECTS:
		MakePanel_Editor_Objects();
		break;
	}
	ImGui::End();

	MakePanel_Visibility();

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

	m_hoveredTile = MousePosToGridIndex();
	m_hoveredTileValid = ValidTile(m_hoveredTile);
	ProcessInput();

	// Deprecating...? Keyboard Camera Controls
	Spear::InputManager& input = Spear::ServiceLocator::GetInputManager();
	const int mouseWheel = input.Wheel();
	const float zoomSpeed{10.f};
	float scrollSpeed{1000.f};
	if (input.InputHold(INPUT_SHIFT))
	{
		scrollSpeed = 2000.f;
	}
	if(!input.InputHold(INPUT_MODIFIER))
	{ 
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
		if (input.InputHold(INPUT_ZOOM_IN) || mouseWheel > 0)
		{
			m_camZoom *= 1.1f;
		}
		if (input.InputHold(INPUT_ZOOM_OUT) || mouseWheel < 0)
		{
			m_camZoom *= 0.9f;
		}
		m_camZoom = std::min(m_camZoom, 3.f);
		m_camZoom = std::max(m_camZoom, 0.75f);
	}

	// Save / Load
	if (input.InputHold(INPUT_MODIFIER) && input.InputStart(INPUT_SAVE))
	{
		SaveLevel();
	}
	else if (input.InputHold(INPUT_MODIFIER) && input.InputHold(INPUT_LOAD))
	{
		OpenLevel("Test.level");
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
	Vector2f mousePos{ Spear::ServiceLocator::GetInputManager().GetMousePos().ToFloat() };
	mousePos -= m_camOffset;
	
	return { mousePos.x / MapSpacing(), mousePos.y / MapSpacing() };
}

void FlowstateEditor::ProcessInput()
{
	if (ProcessInput_DragView())
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
	
		/* Minimum Viable Controls:
		* Middle Click: Create new & select
		* Click: Clear selection & select object
		* Ctrl Click: Add to selection
		* Click + Drag: Reposition (w/ group)
		* Del: Delete currently selected
		*/

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
		if (input.InputHold(INPUT_SHIFT))
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
	return false;
}

bool FlowstateEditor::ProcessInput_Objects_Create()
{
	Spear::InputManager& input = Spear::ServiceLocator::GetInputManager();
	if (input.MiddleClickRelease())
	{
		m_selectedObjects.clear();
		GameObject::ConstructorFuncPtr ObjectConstructor = GameObject::ObjectConstructors()->at(m_objectConstructorId).second;
		GameObject* newObj = ObjectConstructor();
		newObj->SetPosition(MousePosToWorldPos());
		m_selectedObjects.emplace_back(newObj);
	}

	return false;
}

bool FlowstateEditor::ProcessInput_Objects_Delete()
{
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

void FlowstateEditor::MakePanel_Editor_Map()
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

	// TOOD: This should be part of MapData so different maps can use unique offsets (small offsets for road & pavement vs. large offsets for canyons etc.)
	// Maybe units represent 'tiles' so 1 = aligned to first tile, 2 equals aligned to 2nd tile, 1.5 is in middle, etc.
	int temp{0};
	ImGui::SeparatorText("TODO (Unimplemented)");
	ImGui::Text("Roof Distances: ");
	ImGui::InputInt("Upper Roof", &temp);
	ImGui::InputInt("Lower Roof", &temp);

	ImGui::Text("Floor Distances: ");
	ImGui::InputInt("Upper Floor", &temp);
	ImGui::InputInt("Lower Floor", &temp);
}

void FlowstateEditor::MakePanel_Editor_Objects()
{
	ImGui::SeparatorText("Object Picker");
	std::vector<std::pair<const char*, GameObject::ConstructorFuncPtr>> ObjectConstructors = *GameObject::ObjectConstructors();

	if (ImGui::BeginListBox("Object"))
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

	m_selectedObjects[0]->PopulateEditorPanel();
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

			// Tile Outlines
			const Vector2f tilePos = m_camOffset + (Vector2f(x, y) * MapSpacing());
			if (m_editorMode == MODE_TILES)
			{
				Spear::Renderer::LinePolyData square;
				square.segments = 4;
				square.colour = (node.collisionMask > 0 ? m_colourCollision : m_colourDefault);
				square.pos = tilePos;
				square.radius = TileRadius() + 2;
				square.rotation = TO_RADIANS(45.f);
				square.depth = outlineDepth;
				if (m_ongoingClickSelection.contains({ x, y }))
				{
					switch (m_ongoingSelectionMode)
					{
					case SELECTION_ADDITIVE: square.colour = m_colourSelectingAdd; break;
					case SELECTION_SUBTRACTIVE: square.colour = m_colourSelectingSubtract; break;
					default: ASSERT(false); break;
					}
				}
				else if (m_hoveredTileValid && m_hoveredTile.x == x && m_hoveredTile.y == y)
				{
					square.colour = m_colourHovered;
				}
				else if (m_selectedTiles.contains({ x, y }))
				{
					square.colour = m_colourSelected;
				}
				rend.AddLinePoly(square);
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
				Spear::ServiceLocator::GetScreenRenderer().AddSprite(sprite, BATCH_MAP);
			}
		}
	}

	const float objectIconSize = 10.f;

	// Draw GameObjects
	const std::vector<GameObject*>& gameObjects = GameObject::GetAllObjects();
	for (const GameObject* object : gameObjects)
	{
		Spear::Renderer::LinePolyData icon;
		icon.segments = 8;
		icon.colour = m_colourObject;
		icon.pos = object->GetPosition().XY() * MapSpacing();
		icon.pos += m_camOffset;
		icon.radius = m_camZoom * objectIconSize;
		icon.depth = objectDepth;
		Spear::ServiceLocator::GetScreenRenderer().AddLinePoly(icon);
	}

	Spear::ServiceLocator::GetScreenRenderer().Render();
}

void FlowstateEditor::StateExit()
{
	GameObject::GlobalDestroy();

	delete m_map;
	m_map = nullptr;

	Spear::ServiceLocator::GetScreenRenderer().ReleaseAll();

	Spear::ImguiManager& imguiManager = Spear::ImguiManager::Get();
	imguiManager.SetPanelsEnabled(false);
	imguiManager.SetMenuBarStatsEnabled(true);
	imguiManager.SetMenuBarLabel(nullptr);

	Spear::WindowManager::Get().SetWindowTitleOverride(nullptr);
}

void FlowstateEditor::NewLevel(const char* levelName)
{
	delete m_map;
	m_map = new EditorMapData(levelName);
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
	LevelFileManager::EditorLoadLevel(levelName, *m_map);
	Spear::WindowManager::Get().SetWindowTitleOverride((std::string(m_map->mapName) + " - Spear").c_str());
}