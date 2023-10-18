#include "SpearEngine/Core.h"
#include "SpearEngine/ServiceLocator.h"
#include "SpearEngine/InputManager.h"
#include "SpearEngine/ScreenRenderer.h"
#include "SpearEngine/UiButton.h"
#include "SpearEngine/ScreenRenderer.h"

#include "eFlowstate.h"
#include "FlowstateEditor.h"

enum eTextures
{
	TEX_NONE,
	TEX_STONE,
	TEX_WOOD,
};

void GridNode::Reset()
{
	texIdRoof = 0;
	texIdWall = 0;
	texIdFloor = 0;
	extendUp = 0;
	extendDown = 0;
	collisionMask = 0;
}

void EditorMapData::ClearData()
{
	for (int i = 0; i < MAP_WIDTH_MAX_SUPPORTED * MAP_HEIGHT_MAX_SUPPORTED; i++)
	{
		gridNodes[i].Reset();
	}
}

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

	Spear::ServiceLocator::GetInputManager().ConfigureInputs(config, INPUT_COUNT);

	// Set background colour
	glClearColor(0.5f, 0.5f, 0.5f, 1.f);

	// Load editor textures
	m_menuTextures.Allocate(64, 64, 2); // 64x64 textures (2 slots)
	m_menuTextures.SetDataFromFile(0, "../Assets/SPRITES/mode_Editor.png");
	m_menuTextures.SetDataFromFile(1, "../Assets/SPRITES/mode_Play.png");
	Spear::ServiceLocator::GetScreenRenderer().CreateSpriteBatch(m_menuTextures, 20);

	// Load map textures
	m_mapTextures.Allocate(64, 64, 2); // 64x64 textures (2 slots)
	m_mapTextures.SetDataFromFile(0, "../Assets/SPRITES/wall64_wolf.png");
	m_mapTextures.SetDataFromFile(1, "../Assets/SPRITES/wall64_rough.png");
	Spear::ServiceLocator::GetScreenRenderer().CreateSpriteBatch(m_mapTextures, 800);

	// Load editor font
	m_editorFont.LoadFont("../Assets/FONTS/PublicPixelRegular24/PublicPixel");
	Spear::ServiceLocator::GetScreenRenderer().CreateSpriteBatch(m_editorFont, 100);
}

int FlowstateEditor::StateUpdate(float deltaTime)
{
	Spear::InputManager& input = Spear::ServiceLocator::GetInputManager();
	
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

	// Editing Controls
	Vector2i gridIndex{MousePosToGridIndex()};
	if (ValidGridIndex(gridIndex))
	{
		GridNode& node = m_map.GetNode(gridIndex.x, gridIndex.y);

		if (input.InputHold(INPUT_APPLY))
		{
			node.texIdWall = TEX_STONE;
		}
		else if (input.InputHold(INPUT_CLEAR))
		{
			node.texIdWall = TEX_NONE;
		}
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

	Spear::ScreenRenderer::SpriteData sprite;
	sprite.pos = Vector2f(200, 200);
	Spear::ServiceLocator::GetScreenRenderer().AddSprite(sprite, BATCH_GUI);

	Spear::ServiceLocator::GetScreenRenderer().Render();
}

void FlowstateEditor::StateExit()
{
	Spear::ServiceLocator::GetScreenRenderer().ReleaseAll();
}