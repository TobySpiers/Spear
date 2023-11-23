#include "SpearEngine/Core.h"
#include "SpearEngine/ServiceLocator.h"
#include "SpearEngine/InputManager.h"
#include "SpearEngine/ScreenRenderer.h"
#include "SpearEngine/UiButton.h"
#include "SpearEngine/ScreenRenderer.h"

#include "eFlowstate.h"
#include "FlowstateEditor.h"
#include "LevelManager.h"


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
	config[INPUT_CYCLE_MODE] = SDL_SCANCODE_SPACE;
	
	config[INPUT_SAVE] = SDL_SCANCODE_S;
	config[INPUT_LOAD] = SDL_SCANCODE_L;

	Spear::ServiceLocator::GetInputManager().ConfigureInputs(config, INPUT_COUNT);

	// Set background colour
	glClearColor(0.5f, 0.5f, 0.5f, 1.f);

	// Load editor gui textures
	m_menuTextures.Allocate(64, 64, 2); // 64x64 textures (2 slots)
	m_menuTextures.SetDataFromFile(0, "../Assets/SPRITES/mode_Editor.png");
	m_menuTextures.SetDataFromFile(1, "../Assets/SPRITES/mode_Play.png");
	Spear::ServiceLocator::GetScreenRenderer().CreateSpriteBatch(m_menuTextures, 20);

	// Load map textures
	m_mapTextures.Allocate(64, 64, eLevelTextures::TEX_TOTAL); // 64x64 textures (tex-total slots)
	m_mapTextures.SetDataFromFile(eLevelTextures::TEX_STONE, "../Assets/SPRITES/wall64_wolf.png");
	m_mapTextures.SetDataFromFile(eLevelTextures::TEX_WOOD, "../Assets/SPRITES/wall64_rough.png");
	Spear::ServiceLocator::GetScreenRenderer().CreateSpriteBatch(m_mapTextures, 800);

	// Load editor font
	m_editorFont.LoadFont("../Assets/FONTS/PublicPixelRegular24/PublicPixel");
	Spear::ServiceLocator::GetScreenRenderer().CreateSpriteBatch(m_editorFont, 100);

	// Assign map textures to Editor Buttons
	for (int i = 0; i < eLevelTextures::TEX_TOTAL; i++)
	{
		m_textureButtons[i].Initialise(m_menuTextures);
		m_textureButtons[i].m_sprite.texLayer = i;
		m_textureButtons[i].m_sprite.pos = Vector2f(50.f, 100.f + (i * 75.f));
	}
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
	for (int i = 0; i < eLevelTextures::TEX_TOTAL; i++)
	{
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
	if (input.InputRelease(INPUT_CYCLE_MODE))
	{
		m_curMode++;
		if (m_curMode >= EditorMode::MODE_TOTAL)
		{
			m_curMode = 0;
		}
	}

	// Editor Mouse
	if(!m_cursorInMenu)
	{
		Vector2i gridIndex{MousePosToGridIndex()};
		if (ValidGridIndex(gridIndex))
		{
			GridNode& node = m_map.GetNode(gridIndex.x, gridIndex.y);

			// During RISE/FALL mode, cache first value so we can mass-edit a set of tiles
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
					case MODE_WALL:
						node.texIdWall = m_curTex;
						node.collisionMask = eCollisionType::COLL_SOLID;
						break;
					case MODE_FLOOR:
						node.texIdFloor = m_curTex;
						break;
					case MODE_ROOF:
						node.texIdRoof = m_curTex;
						break;
					case MODE_RISE:
						node.extendUp = m_clickCache;
						break;
					case MODE_FALL:
						node.extendDown = m_clickCache;
						break;
					case MODE_COLLISION:
						node.collisionMask = eCollisionType::COLL_SOLID;
						break;
				}
			}
			else if (input.InputHold(INPUT_CLEAR))
			{
				switch (m_curMode)
				{
				case MODE_WALL:
					node.texIdWall = eLevelTextures::TEX_NONE;
					node.collisionMask = eCollisionType::COLL_NONE;
					break;
				case MODE_FLOOR:
					node.texIdFloor = eLevelTextures::TEX_NONE;
					break;
				case MODE_ROOF:
					node.texIdRoof = eLevelTextures::TEX_NONE;
					break;
				case MODE_RISE:
					node.extendUp = 0;
				case MODE_FALL:
					node.extendDown = 0;
				case MODE_COLLISION:
					node.collisionMask = eCollisionType::COLL_NONE;
					break;
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
		case MODE_FLOOR:
			return "Floor";
		case MODE_WALL:
			return "Wall";
		case MODE_ROOF:
			return "Roof";
		case MODE_RISE:
			return "Rise";
		case MODE_FALL:
			return "Fall";
		case MODE_COLLISION:
			return "Collision";
		default:
			return "Unknown";
	}
}

void FlowstateEditor::StateRender()
{
	Spear::ScreenRenderer& rend = Spear::ServiceLocator::GetScreenRenderer();

	// Draw Editor HUD
	Spear::ScreenRenderer::TextData textMode;
	textMode.text = std::string("MODE: ") + GetModeText();
	textMode.pos = Vector2f(20.f, 20.f);
	textMode.alignment = Spear::ScreenRenderer::TEXT_ALIGN_LEFT;
	Spear::ServiceLocator::GetScreenRenderer().AddText(textMode, BATCH_TEXT);

	Spear::ScreenRenderer::TextData textLevel;
	textLevel.text = "Level 0";
	textLevel.pos = Vector2f(Spear::Core::GetWindowSize().x / 2, 20.f);
	textLevel.alignment = Spear::ScreenRenderer::TEXT_ALIGN_MIDDLE;
	Spear::ServiceLocator::GetScreenRenderer().AddText(textLevel, BATCH_TEXT);

	Spear::ScreenRenderer::TextData textResolution;
	textResolution.text = std::to_string(m_map.gridWidth) + " x " + std::to_string(m_map.gridHeight);
	textResolution.pos = Vector2f(Spear::Core::GetWindowSize().x - 20.f, 20.f);
	textResolution.alignment = Spear::ScreenRenderer::TEXT_ALIGN_RIGHT;
	Spear::ServiceLocator::GetScreenRenderer().AddText(textResolution, BATCH_TEXT);

	// Draw save popup if cooldown active
	if(m_saveCooldown > 0.f)
	{
		Spear::ScreenRenderer::TextData textSavePopup;
		textSavePopup.text = "Level saved...";
		textSavePopup.pos = Vector2f(Spear::Core::GetWindowSize().x / 2, Spear::Core::GetWindowSize().y - 20.f);
		textSavePopup.alignment = Spear::ScreenRenderer::TEXT_ALIGN_MIDDLE;
		Spear::ServiceLocator::GetScreenRenderer().AddText(textSavePopup, BATCH_TEXT);
	}

	// Draw Map Data
	Vector2i mouseNode{MousePosToGridIndex()};
	for (int x = 0; x < m_map.gridWidth; x++)
	{
		for (int y = 0; y < m_map.gridHeight; y++)
		{
			GridNode& node{ m_map.GetNode(x, y) };

			// Tile Outlines
			Spear::ScreenRenderer::LinePolyData square;
			square.segments = 4;
			square.colour = (node.collisionMask > 0 ? Colour4f::Red() : Colour4f::White());
			square.pos = Vector2f(x, y) * MapSpacing();
			square.pos += m_camOffset;
			square.radius = TileRadius() + 2;
			square.rotation = TO_RADIANS(45.f);

			if(mouseNode.x == x && mouseNode.y == y && !m_cursorInMenu)
			{
				square.colour = Colour4f::Blue();
			}

			rend.AddLinePoly(square);

			// Height info
			if (m_curMode == MODE_RISE && node.extendUp)
			{
				Spear::ScreenRenderer::TextData textRise;
				textRise.text = std::to_string(node.extendUp);
				textRise.pos = square.pos;
				textRise.alignment = Spear::ScreenRenderer::TEXT_ALIGN_MIDDLE;
				Spear::ServiceLocator::GetScreenRenderer().AddText(textRise, BATCH_TEXT);
			}
			else if (m_curMode == MODE_FALL && node.extendDown)
			{
				Spear::ScreenRenderer::TextData textRise;
				textRise.text = std::to_string(node.extendDown);
				textRise.pos = square.pos;
				textRise.alignment = Spear::ScreenRenderer::TEXT_ALIGN_MIDDLE;
				Spear::ServiceLocator::GetScreenRenderer().AddText(textRise, BATCH_TEXT);
			}

			// Floor Textures
			if (node.texIdFloor != TEX_NONE)
			{
				Spear::ScreenRenderer::SpriteData sprite;
				sprite.pos = Vector2f(x, y) * MapSpacing();
				sprite.pos += m_camOffset;
				sprite.opacity = 0.25f;
				sprite.size = Vector2f(m_camZoom, m_camZoom);
				sprite.texLayer = node.texIdFloor;
				Spear::ServiceLocator::GetScreenRenderer().AddSprite(sprite, BATCH_MAP);
			}

			// Wall Textures
			if (node.texIdWall != TEX_NONE)
			{
				Spear::ScreenRenderer::SpriteData sprite;
				sprite.pos = Vector2f(x, y) * MapSpacing();
				sprite.pos += m_camOffset;
				sprite.size = Vector2f(m_camZoom, m_camZoom);
				sprite.texLayer = node.texIdWall;

				if(m_curMode == MODE_ROOF)
				{
					sprite.opacity = 0.5f;
				}

				Spear::ServiceLocator::GetScreenRenderer().AddSprite(sprite, BATCH_MAP);
			}

			// Roof Textures
			if (m_curMode == MODE_ROOF && node.texIdRoof != TEX_NONE)
			{
				Spear::ScreenRenderer::SpriteData sprite;
				sprite.pos = Vector2f(x, y) * MapSpacing();
				sprite.pos += m_camOffset;
				sprite.size = Vector2f(m_camZoom, m_camZoom);
				sprite.texLayer = node.texIdRoof;
				Spear::ServiceLocator::GetScreenRenderer().AddSprite(sprite, BATCH_MAP);
			}
		}
	}

	// Draw Editor Buttons
	for (int i = 0; i < eLevelTextures::TEX_TOTAL; i++)
	{
		m_textureButtons[i].Draw(BATCH_MAP);
	}
	// Active Button Highlight 
	Spear::ScreenRenderer::LinePolyData activeTexButton;
	activeTexButton.segments = 4;
	activeTexButton.colour = Colour4f::Blue();
	activeTexButton.pos = Vector2f(50.f, 100.f + (m_curTex * 75.f));
	activeTexButton.radius = 48;
	activeTexButton.rotation = TO_RADIANS(45.f);
	rend.AddLinePoly(activeTexButton);

	Spear::ServiceLocator::GetScreenRenderer().Render();
}

void FlowstateEditor::StateExit()
{
	Spear::ServiceLocator::GetScreenRenderer().ReleaseAll();
}

void FlowstateEditor::SaveLevel()
{
	if(m_saveCooldown <= 0.f)
	{
		LevelManager::EditorSaveLevel("level", m_map);
		m_saveCooldown = 3.0f;
	}
}

void FlowstateEditor::LoadLevel()
{
	LevelManager::EditorLoadLevel("level", m_map);
}