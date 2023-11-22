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
	config[INPUT_SCROLL_LEFT] = SDL_SCANCODE_A;
	config[INPUT_SCROLL_RIGHT] = SDL_SCANCODE_D;
	config[INPUT_SCROLL_UP] = SDL_SCANCODE_W;
	config[INPUT_SCROLL_DOWN] = SDL_SCANCODE_S;
	config[INPUT_ZOOM_IN] = SDL_SCANCODE_E;
	config[INPUT_ZOOM_OUT] = SDL_SCANCODE_Q;
	config[INPUT_SAVE] = SDL_SCANCODE_S;
	config[INPUT_LOAD] = SDL_SCANCODE_L;

	Spear::ServiceLocator::GetInputManager().ConfigureInputs(config, INPUT_COUNT);

	// Set background colour
	glClearColor(0.5f, 0.5f, 0.5f, 1.f);

	// Load editor textures
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
	Spear::InputManager& input = Spear::ServiceLocator::GetInputManager();
	
	// Editor HUD Controls
	for (int i = 0; i < eLevelTextures::TEX_TOTAL; i++)
	{
		m_textureButtons[i].Update();
		if (m_textureButtons[i].Clicked())
		{
			m_curTex = i;
		}
	}

	// Editing Controls
	Vector2i gridIndex{MousePosToGridIndex()};
	if (ValidGridIndex(gridIndex))
	{
		GridNode& node = m_map.GetNode(gridIndex.x, gridIndex.y);

		if (input.InputHold(INPUT_APPLY))
		{
			node.texIdWall = m_curTex;
		}
		else if (input.InputHold(INPUT_CLEAR))
		{
			node.texIdWall = eLevelTextures::TEX_NONE;
		}
	}

	// Camera Controls
	const float scrollSpeed{1000.f};
	const float zoomSpeed{10.f};
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

	// Save / Load
	if (input.InputHold(INPUT_SAVE))
	{
		SaveLevel();
	}
	else if (input.InputHold(INPUT_LOAD))
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

void FlowstateEditor::StateRender()
{
	Spear::ScreenRenderer& rend = Spear::ServiceLocator::GetScreenRenderer();
	Vector2i mouseNode{MousePosToGridIndex()};

	// Draw Map Data
	for (int x = 0; x < m_map.gridWidth; x++)
	{
		for (int y = 0; y < m_map.gridHeight; y++)
		{
			GridNode& node{ m_map.GetNode(x, y) };

			// Tile Outlines
			Spear::ScreenRenderer::LinePolyData square;
			square.segments = 4;
			square.colour = Colour4f::White();
			square.pos = Vector2f(x, y) * MapSpacing();
			square.pos += m_camOffset;
			square.radius = TileRadius() + 2;
			square.rotation = TO_RADIANS(45.f);

			if(mouseNode.x == x && mouseNode.y == y)
				square.colour = Colour4f::Blue();

			rend.AddLinePoly(square);

			// Texture Data
			if (node.texIdWall != TEX_NONE)
			{
				Spear::ScreenRenderer::SpriteData sprite;
				sprite.pos = Vector2f(x, y) * MapSpacing();
				sprite.pos += m_camOffset;
				sprite.size = Vector2f(m_camZoom, m_camZoom);
				sprite.texLayer = node.texIdWall;
				Spear::ServiceLocator::GetScreenRenderer().AddSprite(sprite, BATCH_MAP);
			}
		}
	}

	// Draw Editor HUD
	Spear::ScreenRenderer::TextData textMode;
	textMode.text = "MODE: Wall";
	textMode.pos = Vector2f(20.f, 20.f);
	textMode.alignment = Spear::ScreenRenderer::TEXT_ALIGN_LEFT;
	Spear::ServiceLocator::GetScreenRenderer().AddText(textMode, BATCH_TEXT);

	Spear::ScreenRenderer::TextData textLevel;
	textLevel.text = "Test Level 0";
	textLevel.pos = Vector2f(Spear::Core::GetWindowSize().x / 2, 20.f);
	textLevel.alignment = Spear::ScreenRenderer::TEXT_ALIGN_MIDDLE;
	Spear::ServiceLocator::GetScreenRenderer().AddText(textLevel, BATCH_TEXT);

	Spear::ScreenRenderer::TextData textTemp;
	textTemp.text = "Temp Val";
	textTemp.pos = Vector2f(Spear::Core::GetWindowSize().x - 20.f, 20.f);
	textTemp.alignment = Spear::ScreenRenderer::TEXT_ALIGN_RIGHT;
	Spear::ServiceLocator::GetScreenRenderer().AddText(textTemp, BATCH_TEXT);

	// Draw Editor Buttons
	for (int i = 0; i < eLevelTextures::TEX_TOTAL; i++)
	{
		m_textureButtons[i].Draw(BATCH_MAP);
	}

	Spear::ServiceLocator::GetScreenRenderer().Render();
}

void FlowstateEditor::StateExit()
{
	Spear::ServiceLocator::GetScreenRenderer().ReleaseAll();
}

void FlowstateEditor::SaveLevel()
{
	LevelManager::EditorSaveLevel("test", m_map);
}

void FlowstateEditor::LoadLevel()
{
	LevelManager::EditorLoadLevel("test", m_map);
}