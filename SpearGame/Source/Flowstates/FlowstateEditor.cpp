#include "Core/Core.h"
#include "Core/ServiceLocator.h"
#include "Core/InputManager.h"
#include "Core/ImguiManager.h"
#include "Graphics/ScreenRenderer.h"
#include "UiButton.h"

#include "eFlowstate.h"
#include "FlowstateEditor.h"
#include "LevelFileManager.h"


void FlowstateEditor::StateEnter()
{
	m_map.ClearData();
	
	// Configure Inputs
	int config[INPUT_COUNT];
	config[INPUT_APPLY] = SDL_BUTTON_LEFT;
	config[INPUT_CLEAR] = SDL_BUTTON_RIGHT;
	config[INPUT_QUIT] = SDL_SCANCODE_ESCAPE;
	config[INPUT_MODIFIER] = SDL_SCANCODE_LCTRL;

	config[INPUT_INCREASE_MAPSIZE] = SDL_SCANCODE_EQUALS;
	config[INPUT_DECREASE_MAPSIZE] = SDL_SCANCODE_MINUS;

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

	// Load editor gui textures
	m_menuTextures.Allocate(64, 64, 2); // 64x64 textures (2 slots)
	m_menuTextures.SetDataFromFile(0, "../Assets/SPRITES/mode_Editor.png");
	m_menuTextures.SetDataFromFile(1, "../Assets/SPRITES/mode_Play.png");
	Spear::ServiceLocator::GetScreenRenderer().CreateSpriteBatch(m_menuTextures, 20);

	// Load map textures
	m_mapTextures.InitialiseFromDirectory("../Assets/TILESETS/64");
	Spear::ServiceLocator::GetScreenRenderer().CreateSpriteBatch(m_mapTextures, 800);

	// Assign map textures to Editor Buttons
	for (int i = 0; i < m_mapTextures.GetDepth(); i++)
	{
		m_textureButtons.push_back(Spear::UiButton());
		m_textureButtons.back().Initialise(m_menuTextures);
		m_textureButtons.back().m_sprite.texLayer = i;
		m_textureButtons.back().m_sprite.pos = Vector2f(50.f, 100.f + (i * 75.f));
		m_textureButtons.back().m_sprite.depth = 0.f;
	}

	// Enable ImGui for editor functionality
	Spear::ImguiManager::Get().SetImguiEnabled(true);
}

int FlowstateEditor::StateUpdate(float deltaTime)
{
	if(m_saveCooldown > 0.f)
	{
		m_saveCooldown -= deltaTime;
	}

	Spear::InputManager& input = Spear::ServiceLocator::GetInputManager();

	// Editor HUD Controls
	m_cursorInMenu = false;
	const int mouseWheel = input.Wheel();
	for (int i = 0; i < m_mapTextures.GetDepth(); i++)
	{
		m_textureButtons[i].m_sprite.pos.y += m_menuTexturesScrollSpeed * mouseWheel;
		m_textureButtons[i].Update();

		if (m_textureButtons[i].MouseOver())
		{
			m_cursorInMenu = true;
		}

		if (m_textureButtons[i].Clicked())
		{
			m_curTex = i;
		}
	}

	// Editor Keyboard
	if (input.InputHold(INPUT_INCREASE_MAPSIZE))
	{
		if(m_map.gridWidth < MAP_WIDTH_MAX_SUPPORTED && m_map.gridHeight < MAP_HEIGHT_MAX_SUPPORTED)
		{
			m_map.SetSize(m_map.gridWidth + 1, m_map.gridHeight + 1);
		}
	}
	else if (input.InputHold(INPUT_DECREASE_MAPSIZE))
	{
		if (m_map.gridWidth > 4 && m_map.gridHeight > 4)
		{
			m_map.SetSize(m_map.gridWidth - 1, m_map.gridHeight - 1);
		}
	}
	if (input.InputRelease(INPUT_MODE_NEXT))
	{
		m_curMode++;
		if (m_curMode >= EditorMode::MODE_TOTAL)
		{
			m_curMode = 0;
		}
	}
	else if (input.InputRelease(INPUT_MODE_PREV))
	{
		m_curMode--;
		if (m_curMode < 0)
		{
			m_curMode = EditorMode::MODE_TOTAL - 1;
		}
	}

	// Editor Mouse
	if(!m_cursorInMenu)
	{
		Vector2i gridIndex{MousePosToGridIndex()};
		if (ValidGridIndex(gridIndex))
		{
			GridNode& node = m_map.GetNode(gridIndex.x, gridIndex.y);

			// During modes such as Rise/Fall, cache first value so we can mass-edit a set of tiles
			if (input.InputStart(INPUT_APPLY))
			{
				if (m_curMode == MODE_RISE)
				{
					m_clickCache = node.extendUp + 1;
				}
				else if (m_curMode == MODE_FALL)
				{
					m_clickCache = node.extendDown + 1;
				}
			}

			if (input.InputHold(INPUT_APPLY))
			{
				switch (m_curMode)
				{
					case MODE_PLAYERSTART:
						m_map.playerStart = gridIndex;
						break;
					case MODE_WALL:
						node.texIdWall = m_curTex;
						node.collisionMask = eCollisionMask::COLL_WALL;
						break;
					case MODE_FLOOR:
						node.texIdFloor[0] = m_curTex;
						break;
					case MODE_FLOOR2:
						node.texIdFloor[1] = m_curTex;
						break;
					case MODE_ROOF:
						node.texIdRoof[0] = m_curTex;
						break;
					case MODE_ROOF2:
						node.texIdRoof[1] = m_curTex;
						break;
					case MODE_RISE:
						node.extendUp = m_clickCache;
						break;
					case MODE_FALL:
						node.extendDown = m_clickCache;
						break;
					case MODE_COLLISION:
						node.collisionMask = eCollisionMask::COLL_WALL;
						break;
				}
			}
			else if (input.InputHold(INPUT_CLEAR))
			{
				switch (m_curMode)
				{
				case MODE_WALL:
					node.texIdWall = eLevelTextures::TEX_NONE;
					node.collisionMask = eCollisionMask::COLL_NONE;
					break;
				case MODE_FLOOR:
					node.texIdFloor[0] = eLevelTextures::TEX_NONE;
					break;
				case MODE_FLOOR2:
					node.texIdFloor[1] = eLevelTextures::TEX_NONE;
					break;
				case MODE_ROOF:
					node.texIdRoof[0] = eLevelTextures::TEX_NONE;
					break;
				case MODE_ROOF2:
					node.texIdRoof[1] = eLevelTextures::TEX_NONE;
					break;
				case MODE_RISE:
					node.extendUp = 0;
					break;
				case MODE_FALL:
					node.extendDown = 0;
					break;
				case MODE_COLLISION:
					node.collisionMask = eCollisionMask::COLL_NONE;
					break;
				case MODE_DRAW_DIRECTION:
					node.drawFlags = eDrawFlags::DRAW_DEFAULT;
					break;
				}
			}
			else if (m_curMode == MODE_DRAW_DIRECTION)
			{
				if (input.InputHold(INPUT_FACE_NORTH))
				{
					node.drawFlags |= DRAW_N;
				}
				if (input.InputHold(INPUT_FACE_EAST))
				{
					node.drawFlags |= DRAW_E;
				}
				if (input.InputHold(INPUT_FACE_SOUTH))
				{
					node.drawFlags |= DRAW_S;
				}
				if (input.InputHold(INPUT_FACE_WEST))
				{
					node.drawFlags |= DRAW_W;
				}
			}
		}
	}

	// Camera Controls
	const float scrollSpeed{1000.f};
	const float zoomSpeed{10.f};
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
		if (input.InputHold(INPUT_ZOOM_IN))
		{
			m_camZoom *= 1.05f;
		}
		if (input.InputHold(INPUT_ZOOM_OUT))
		{
			m_camZoom *= 0.95f;
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
		LoadLevel();
	}

	// Quit
	if (input.InputRelease(INPUT_QUIT))
	{
		return static_cast<int>(eFlowstate::STATE_MENU);
	}
	return static_cast<int>(eFlowstate::STATE_THIS);
}

Vector2i FlowstateEditor::MousePosToGridIndex()
{
	Vector2f mousePos{ Spear::ServiceLocator::GetInputManager().GetMousePos().ToFloat()};
	mousePos -= m_camOffset;

	return Vector2i(std::round(mousePos.x / MapSpacing()), std::round(mousePos.y / MapSpacing()));
}

bool FlowstateEditor::ValidGridIndex(const Vector2i& index)
{
	return (index.x >= 0 && index.x < m_map.gridWidth
		 && index.y >= 0 && index.y < m_map.gridHeight);
}

const char* FlowstateEditor::GetModeText()
{
	switch (m_curMode)
	{
		case MODE_PLAYERSTART:
			return "PlayerStart";
		case MODE_FLOOR:
			return "Floor";
		case MODE_FLOOR2:
			return "Floor2";
		case MODE_WALL:
			return "Wall";
		case MODE_ROOF:
			return "Roof";
		case MODE_ROOF2:
			return "Roof2";
		case MODE_RISE:
			return "Rise";
		case MODE_FALL:
			return "Fall";
		case MODE_COLLISION:
			return "Collision";
		case MODE_DRAW_DIRECTION:
			return "DrawDir";
		default:
			return "Unknown";
	}
}

void FlowstateEditor::StateRender()
{
	Spear::Renderer& rend = Spear::ServiceLocator::GetScreenRenderer();

	// Draw Editor HUD
	Spear::Renderer::TextData textMode;
	textMode.text = std::string("MODE: ") + GetModeText();
	textMode.pos = Vector2f(20.f, 20.f);
	textMode.alignment = Spear::Renderer::TEXT_ALIGN_LEFT;
	Spear::ServiceLocator::GetScreenRenderer().AddText(textMode);

	Spear::Renderer::TextData textLevel;
	textLevel.text = "Level 0";
	textLevel.pos = Vector2f(Spear::Core::GetWindowSize().x / 2, 20.f);
	textLevel.alignment = Spear::Renderer::TEXT_ALIGN_MIDDLE;
	Spear::ServiceLocator::GetScreenRenderer().AddText(textLevel);

	Spear::Renderer::TextData textResolution;
	textResolution.text = std::to_string(m_map.gridWidth) + " x " + std::to_string(m_map.gridHeight);
	textResolution.pos = Vector2f(Spear::Core::GetWindowSize().x - 20.f, 20.f);
	textResolution.alignment = Spear::Renderer::TEXT_ALIGN_RIGHT;
	Spear::ServiceLocator::GetScreenRenderer().AddText(textResolution);

	// Draw save popup if cooldown active
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
	textStart.pos = m_camOffset + (m_map.playerStart.ToFloat() * MapSpacing());
	textStart.alignment = Spear::Renderer::TEXT_ALIGN_MIDDLE;
	Spear::ServiceLocator::GetScreenRenderer().AddText(textStart);

	// Draw Map Data
	Vector2i mouseNode{MousePosToGridIndex()};
	constexpr float floorDepth = 0.9f;
	constexpr float wallDepth = 0.8f;
	constexpr float roofDepth = 0.7f;
	constexpr float outlineDepth = 0.1f;
	constexpr float inactiveOpacity = 0.2f;
	for (int x = 0; x < m_map.gridWidth; x++)
	{
		for (int y = 0; y < m_map.gridHeight; y++)
		{
			GridNode& node{ m_map.GetNode(x, y) };

			// Tile Outlines
			Spear::Renderer::LinePolyData square;
			square.segments = 4;
			square.colour = (node.collisionMask > 0 ? Colour4f::Red() : Colour4f::White());
			square.pos = Vector2f(x, y) * MapSpacing();
			square.pos += m_camOffset;
			square.radius = TileRadius() + 2;
			square.rotation = TO_RADIANS(45.f);
			square.depth = outlineDepth;

			if(mouseNode.x == x && mouseNode.y == y && !m_cursorInMenu)
			{
				square.colour = Colour4f::Blue();
			}

			rend.AddLinePoly(square);

			// Height info
			if (m_curMode == MODE_RISE && node.extendUp)
			{
				Spear::Renderer::TextData textRise;
				textRise.text = std::to_string(node.extendUp);
				textRise.pos = square.pos;
				textRise.alignment = Spear::Renderer::TEXT_ALIGN_MIDDLE;
				Spear::ServiceLocator::GetScreenRenderer().AddText(textRise);
			}
			else if (m_curMode == MODE_FALL && node.extendDown)
			{
				Spear::Renderer::TextData textRise;
				textRise.text = std::to_string(node.extendDown);
				textRise.pos = square.pos;
				textRise.alignment = Spear::Renderer::TEXT_ALIGN_MIDDLE;
				Spear::ServiceLocator::GetScreenRenderer().AddText(textRise);
			}
			else if (m_curMode == MODE_DRAW_DIRECTION)
			{
				Spear::Renderer::TextData textDraw;
				std::string textFlags{ "" };
				if (node.drawFlags & DRAW_N) textFlags += "N";
				if (node.drawFlags & DRAW_E) textFlags += "E";
				if (node.drawFlags & DRAW_S) textFlags += "S";
				if (node.drawFlags & DRAW_W) textFlags += "W";
				textDraw.text = textFlags;
				textDraw.pos = square.pos;
				textDraw.alignment = Spear::Renderer::TEXT_ALIGN_MIDDLE;
				Spear::ServiceLocator::GetScreenRenderer().AddText(textDraw);
			}

			// Roof Textures
			if ((m_curMode == MODE_ROOF && node.texIdRoof[0] != TEX_NONE)
			|| (m_curMode == MODE_ROOF2 && node.texIdRoof[1] != TEX_NONE))
			{
				Spear::Renderer::SpriteData sprite;
				sprite.pos = Vector2f(x, y) * MapSpacing();
				sprite.pos += m_camOffset;
				sprite.size = Vector2f(m_camZoom, m_camZoom);
				sprite.texLayer = node.texIdRoof[m_curMode == MODE_ROOF ? 0 : 1];
				sprite.depth = roofDepth;
				Spear::ServiceLocator::GetScreenRenderer().AddSprite(sprite, BATCH_MAP);
			}

			// Wall Textures
			if (node.texIdWall != TEX_NONE)
			{
				Spear::Renderer::SpriteData sprite;
				sprite.pos = Vector2f(x, y) * MapSpacing();
				sprite.pos += m_camOffset;
				sprite.size = Vector2f(m_camZoom, m_camZoom);
				sprite.texLayer = node.texIdWall;
				sprite.depth = wallDepth;

				if(m_curMode != MODE_WALL && m_curMode != MODE_COLLISION)
				{
					sprite.opacity = inactiveOpacity;
				}

				Spear::ServiceLocator::GetScreenRenderer().AddSprite(sprite, BATCH_MAP);
			}

			// Floor Textures
			if ((m_curMode != MODE_FLOOR2 && node.texIdFloor[0] != TEX_NONE)
			|| (m_curMode == MODE_FLOOR2 && node.texIdFloor[1] != TEX_NONE))
			{
				Spear::Renderer::SpriteData sprite;
				sprite.pos = Vector2f(x, y) * MapSpacing();
				sprite.pos += m_camOffset;
				sprite.size = Vector2f(m_camZoom, m_camZoom);
				sprite.texLayer = node.texIdFloor[m_curMode == MODE_FLOOR ? 0 : 1];
				sprite.depth = m_curMode == MODE_FLOOR ? roofDepth : floorDepth;

				if (m_curMode != MODE_FLOOR)
				{
					sprite.opacity = inactiveOpacity;
				}

				Spear::ServiceLocator::GetScreenRenderer().AddSprite(sprite, BATCH_MAP);
			}
		}
	}

	// Draw Editor Buttons
	for (int i = 0; i < m_mapTextures.GetDepth(); i++)
	{
		m_textureButtons[i].Draw(BATCH_MAP);
	}
	// Active Button Highlight 
	Spear::Renderer::LinePolyData activeTexButton;
	activeTexButton.segments = 4;
	activeTexButton.colour = Colour4f::Blue();
	activeTexButton.pos = m_textureButtons[m_curTex].m_sprite.pos;
	activeTexButton.radius = 48;
	activeTexButton.rotation = TO_RADIANS(45.f);
	rend.AddLinePoly(activeTexButton);

	Spear::ServiceLocator::GetScreenRenderer().Render();
}

void FlowstateEditor::StateExit()
{
	Spear::ServiceLocator::GetScreenRenderer().ReleaseAll();

	Spear::ImguiManager::Get().SetImguiEnabled(false);
}

void FlowstateEditor::SaveLevel()
{
	if(m_saveCooldown <= 0.f)
	{
		LevelFileManager::EditorSaveLevel("level", m_map);
		m_saveCooldown = 3.0f;
	}
}

void FlowstateEditor::LoadLevel()
{
	LevelFileManager::EditorLoadLevel("level", m_map);
}